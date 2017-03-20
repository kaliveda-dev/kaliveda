#include "Riostream.h"
#include "KVReconstructedNucleus.h"
#include "KVTelescope.h"
#include "KVIDTelescope.h"
#include "KVGroup.h"
#include "KVMultiDetArray.h"
#include "KVACQParam.h"

using namespace std;

ClassImp(KVReconstructedNucleus);

//////////////////////////////////////////////////////////////////////////////////////////
//KVReconstructedNucleus
//
//Particles reconstructed from data measured in multidetector arrays using identification
//telescopes (see KVIDTelescope class).
//
//If the particle has been identified (IsIdentified()=kTRUE) and calibrated (IsCalibrated()=kTRUE)
//then information on its nature and kinematics can be obtained using the methods of parent
//classes KVNucleus and KVParticle (GetZ(), GetVelocity(), etc.).
//
//Information specific to the detection and identification of the particle:
//
//   GetDetectorList()         -  list of detectors passed through, in reverse order
//   GetNumDet()               -  number of detectors in list = number of detectors passed through
//   GetDetector(Int_t i)      -  ith detector in list (i=0 is stopping detector)
//   GetStoppingDetector()     -  detector in which particle stopped (first in list)
//
//   GetGroup()                -  group in which particle detected/stopped (see KVGroup)
//   GetTelescope()            -  telescope to which stopping detector belongs (see KVTelescope)
//
//   GetIdentifyingTelescope() -  ID telescope which identified this particle (see KVIDTelescope)
//      IsIdentified()               - =kTRUE if particle identification has been completed
//      IsCalibrated()             - =kTRUE if particle's energy has been set
//      GetIDTelescopes()     - list of ID telescopes through which particle passed.
//                                   this list is used to find an ID telescope which can identify the particle.
//    GetRealZ(), GetRealA()  -  return the Z and A determined with floating-point precision by the
//                               identification of the particle.
//

void KVReconstructedNucleus::init()
{
   //default initialisation
   fRealZ = fRealA = 0.;
   fDetNames = "/";
   fIDTelName = "";
   fIDTelescope = 0;
   fNSegDet = 0;
   fAnalStatus = 99;
   fTargetEnergyLoss = 0;
   ResetBit(kIsIdentified);
   ResetBit(kIsCalibrated);
   ResetBit(kCoherency);
   ResetBit(kZMeasured);
   ResetBit(kAMeasured);
}


KVReconstructedNucleus::KVReconstructedNucleus() : fIDResults("KVIdentificationResult", 5)
{
   //default ctor.
   init();
}

KVReconstructedNucleus::KVReconstructedNucleus(const KVReconstructedNucleus& obj)
   : KVNucleus(), fIDResults("KVIdentificationResult", 5)
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVReconstructedNucleus&) obj).Copy(*this);
#endif
}

KVReconstructedNucleus::~KVReconstructedNucleus()
{
   //dtor
   // calls KVGroup::Reset() of the group in which this particle was detected
   if (GetGroup()) {
      GetGroup()->Reset();
   }
   init();
}

//______________________________________________________________________________

void KVReconstructedNucleus::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVReconstructedNucleus.
   //Customized streamer.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 17) {
         // Before v17, fIDResults was a static array: KVIdentificationResult fIDresults[5]
         // We convert this to the new TClonesArray format
         KVNucleus::Streamer(R__b);
         fDetNames.Streamer(R__b);
         fIDTelName.Streamer(R__b);
         if (R__v >= 16) {
            R__b >> fNSegDet;
            R__b >> fAnalStatus;
         }
         R__b >> fRealZ;
         R__b >> fRealA;
         R__b >> fTargetEnergyLoss;
         int R__i;
         KVIdentificationResult id_array[5];
         for (R__i = 0; R__i < 5; R__i++) {
            id_array[R__i].Streamer(R__b);
            id_array[R__i].Copy(*GetIdentificationResult(R__i + 1));
         }
         R__b.CheckByteCount(R__s, R__c, KVReconstructedNucleus::IsA());
      } else
         R__b.ReadClassBuffer(KVReconstructedNucleus::Class(), this, R__v, R__s, R__c);
      // if the multidetector object exists, update some informations
      // concerning the detectors etc. hit by this particle
      if (gMultiDetArray) {
         MakeDetectorList();
         if (GetGroup()) GetGroup()->AddHit(this);
         fIDTelescope = 0;
         if (fIDTelName != "") fIDTelescope = gMultiDetArray->GetIDTelescope(fIDTelName.Data());
         TIter next_det(&fDetList);
         KVDetector* det;
         while ((det = (KVDetector*)next_det())) {
            if (R__v < 16) fNSegDet += det->GetSegment();  // fNSegDet/fAnalStatus non-persistent before v.16
            det->AddHit(this);
            det->SetAnalysed();
            //modify detector's counters depending on particle's identification state
            if (IsIdentified())
               det->IncrementIdentifiedParticles();
            else
               det->IncrementUnidentifiedParticles();
         }
      }
   } else {
      R__b.WriteClassBuffer(KVReconstructedNucleus::Class(), this);
   }
}

//___________________________________________________________________________

void KVReconstructedNucleus::Print(Option_t*) const
{

   int ndets = GetNumDet();
   if (ndets) {

      for (int i = ndets - 1; i >= 0; i--) {
         KVDetector* det = GetDetector(i);
         if (det) det->Print("data");
      }
      for (int i = 1; i <= GetNumberOfIdentificationResults(); i++) {
         KVIdentificationResult* idr = const_cast<KVReconstructedNucleus*>(this)->GetIdentificationResult(i);
         if (idr->IDattempted) idr->Print();
      }
   }
   if (GetStoppingDetector()) cout << "STOPPED IN : " <<
                                      GetStoppingDetector()->GetName() << endl;
   if (IsIdentified()) {
      if (GetIdentifyingTelescope()) cout << "IDENTIFIED IN : " <<
                                             GetIdentifyingTelescope()->GetName() << endl;
      cout << " =======> ";
      cout << " Z=" << GetZ() << " A=" << GetA();
      if (IsAMeasured()) cout << " Areal=" << GetRealA();
      else cout << " Zreal=" << GetRealZ();
   } else {
      cout << "(unidentified)" << endl;
   }
   if (IsCalibrated()) {
      cout << " Total Energy = " << GetEnergy() << " MeV,  Theta=" << GetTheta() << " Phi=" << GetPhi() << endl;
      cout << "    Target energy loss correction :  " << GetTargetEnergyLoss() << " MeV" << endl;
   } else {
      cout << "(uncalibrated)" << endl;
   }
   cout << "RECONSTRUCTION STATUS : " << endl;
   switch (GetStatus()) {
      case kStatusOK:
         cout <<
              "Particle alone in group, or identification independently of other"
              << endl;
         cout << "particles in group is directly possible." << endl;
         break;

      case kStatusOKafterSub:
         cout <<
              "Particle reconstructed after identification of others in group"
              << endl;
         cout <<
              "and subtraction of their calculated energy losses in common detectors."
              << endl;
         break;

      case kStatusOKafterShare:
         cout <<
              "Particle identification estimated after arbitrary sharing of"
              << endl;
         cout <<
              "energy lost in common detectors between several reconstructed particles."
              << endl;
         break;

      case kStatusStopFirstStage:
         cout <<
              "Particle stopped in first stage of telescope. Estimation of minimum Z."
              << endl;
         break;

      case kStatusPileupDE:
         cout <<
              "Undetectable pile-up in first member of identifying telesscope (apparent status=OK)."
              << endl;
         cout << "Would lead to incorrect identification by DE-E method (Z and/or A overestimated)."
              << endl;
         break;

      case kStatusPileupGhost:
         cout <<
              "Undetectable ghost particle in filtered simulation."
              << endl;
         cout << "Another particle passed through all of the same detectors (pile-up)."
              << endl;
         break;


      default:
         cout << GetStatus() << endl;
         break;
   }
   if (GetParameters()->GetNpar()) GetParameters()->Print();
}

//_______________________________________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVReconstructedNucleus::Copy(TObject& obj) const
#else
void KVReconstructedNucleus::Copy(TObject& obj)
#endif
{
   //
   //Copy this to obj
   //
   KVNucleus::Copy(obj);
   ((KVReconstructedNucleus&) obj).SetIdentifyingTelescope(GetIdentifyingTelescope());
   ((KVReconstructedNucleus&) obj).fDetNames = fDetNames;
   ((KVReconstructedNucleus&) obj).SetRealZ(GetRealZ());
   ((KVReconstructedNucleus&) obj).SetRealA(GetRealA());
   ((KVReconstructedNucleus&) obj).SetTargetEnergyLoss(GetTargetEnergyLoss());
}



void KVReconstructedNucleus::Clear(Option_t* opt)
{
   //Reset nucleus. Calls KVNucleus::Clear.
   //Calls KVGroup::Reset for the group where it was reconstructed.

   KVNucleus::Clear(opt);
   if (GetGroup())
      GetGroup()->Reset();
   fDetList.Clear();
   fIDResults.Clear("C");
   init();
}

void KVReconstructedNucleus::AddDetector(KVDetector* det)
{
   //Add a detector to the list of those through which the particle passed.
   //Put reference to detector into fDetectors array, increase number of detectors by one.
   //As this is only used in initial particle reconstruction, we add 1 unidentified particle to the detector.
   // Creates KVHashList fDetList in case it does not exist.

   //add name of detector to fDetNames
   fDetNames += det->GetName();
   fDetNames += "/";
   // store pointer to detector
   fDetList.Add(det);
   if (det->IsDetecting()) {
      //add segmentation index of detector to total segmentation index of particle
      fNSegDet += det->GetSegment();
      //add 1 unidentified particle to the detector
      det->IncrementUnidentifiedParticles();
   }

}

//______________________________________________________________________________________________//

void KVReconstructedNucleus::Reconstruct(KVDetector* kvd)
{
   //Reconstruction of a detected nucleus from the successive energy losses
   //measured in a series of detectors/telescopes.
   //
   //Starting from detector *kvd, collect information from all detectors placed directly
   //in front of *kvd (kvd->GetAlignedDetectors()),
   //these are the detectors the particle has passed through.
   //
   //Each one is added to the particle's list (KVReconstructedNucleus::AddDetector), and,
   //if it is not an unsegmented detector, it is marked as having been "analysed"
   //(KVDetector::SetAnalysed) in order to stop it being considered as a starting point for another
   //particle reconstruction.

   fNSegDet = 0;
   SetParameter("ARRAY", kvd->GetNameOfArray());
   //get list of detectors through which particle passed
   if (kvd->GetGroup()) {
      TList* aligned = kvd->GetAlignedDetectors();
      if (aligned) {

         TIter next_aligned(aligned);
         KVDetector* d;
         while ((d = (KVDetector*) next_aligned())) {
            AddDetector(d);
            d->AddHit(this);  // add particle to list of particles hitting detector
            d->SetAnalysed(kTRUE);   //cannot be used to seed another particle
         }
      }
      kvd->GetGroup()->AddHit(this);
   }
}

//_____________________________________________________________________________________

void KVReconstructedNucleus::Identify()
{
   // Try to identify this nucleus by calling the Identify() function of each
   // ID telescope crossed by it, starting with the telescope where the particle stopped, in order
   //      -  only attempt identification in ID telescopes containing the stopping detector.
   //      -  only telescopes which have been correctly initialised for the current run are used,
   //         i.e. those for which KVIDTelescope::IsReadyForID() returns kTRUE.
   // This continues until a successful identification is achieved or there are no more ID telescopes to try.
   // The identification code corresponding to the identifying telescope is set as the identification code of the particle.


   KVList* idt_list = GetStoppingDetector()->GetAlignedIDTelescopes();
   if (idt_list && idt_list->GetSize() > 0) {

      KVIDTelescope* idt;
      TIter next(idt_list);
      Int_t idnumber = 1;
      Int_t n_success_id = 0;//number of successful identifications
      while ((idt = (KVIDTelescope*) next())) {
         KVIdentificationResult* IDR = GetIdentificationResult(idnumber++);


         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?

            IDR->IDattempted = kTRUE;
            idt->Identify(IDR);

            if (IDR->IDOK) n_success_id++;
         } else
            IDR->IDattempted = kFALSE;

         if (n_success_id < 1 &&
               ((!IDR->IDattempted) || (IDR->IDattempted && !IDR->IDOK))) {
            // the particle is less identifiable than initially thought
            // we may have to wait for secondary identification
            Int_t nseg = GetNSegDet();
            SetNSegDet(TMath::Max(nseg - 1, 0));
            //if there are other unidentified particles in the group and NSegDet is < 2
            //then exact status depends on segmentation of the other particles : reanalyse
            if (GetNSegDet() < 2 && GetNUnidentifiedInGroup(GetGroup()) > 1) {
               AnalyseParticlesInGroup(GetGroup());
               return;
            }
            //if NSegDet = 0 it's hopeless
            if (!GetNSegDet()) {
               AnalyseParticlesInGroup(GetGroup());
               return;
            }
         }


      }

   }

}

//______________________________________________________________________________________________//

void KVReconstructedNucleus::AnalyseParticlesInGroup(KVGroup* grp)
{
   //The short description of this method is:
   //
   //      Here we analyse all the particles stopping in this group in order to
   //      determine 'KVReconstructedNucleus::GetStatus()' for each one.
   //      This is a sort of 'first-order' coherency analysis necessary prior to
   //      attempting particle identification.
   //
   //The long description of this method is:
   //
   //Analyse the different particles reconstructed in this group in order to
   //establish the strategy necessary to fully reconstruct each one
   //In a group with >=3-member telescopes, the maximum number of particles which may be
   //reconstructed correctly is equal to the number of telescopes constituting the
   //last layer of the group if all particles stop in the last detector of the last
   //layer.
   //i.e. in rings 4-9 (ChIo-Si-CsI telescopes) any group can detect up to 4 particles
   //all going through to the CsI i.e. either light particles identified from CsI R-L
   //matrices or fragments identified from Si-CsI matrices.
   //
   //Any particle detected in a group on its own is perfectly fine (fAnalStatus=0).
   //
   //Any particle detected in the final layer of a "3-detector layer" group
   //(i.e. ChIo-Si-CsI) is perfectly fine (fAnalStatus=0).
   //
   //A particle stopping in the 2nd layer of a "3-detector layer" group (i.e. in Si)
   //can be reconstructed if all the other particles in its group stop in the 3rd
   //layer (i.e. CsI), once the other particles have been reconstructed and their
   //calculated energy loss in the ChIo has been subtracted from the measured value
   //(fAnalStatus=1).
   //
   //It's a question of "segmented" or "independent" detectors. The ChIo's are not
   //"independent" because a particle passing the ChIo can hit several detectors behind
   //it, whereas any particle passing a given Si can only hit the CsI directly behind
   //that Si, because the Si (and CsI) are more "segmented" than the ChIo's.
   //Soooo...any particle crossing at least 2 "independent" detectors can be identified
   //sans probleme. In fact CsI count for 2 independent detectors themselves, because
   //light particles can be identified just from CsI information i.e. no need for
   //"delta-E" signal. Unless of course the R-L attempt fails, at which moment the
   //particle's AnalStatus may be changed and it may be up for reassessment in
   //subsequent rounds of analysis.
   //Particles hitting 0 independent detectors (I.e. stopped in ChIo)
   //can not be reconstructed. Any particle hitting only 1 independent detector can
   //be reconstructed if alone in the group i.e. no other particles or all others have
   //hit >=2 independent detectors. If more than one particle in the same group only
   //hit 1 independent detector, then one can only make a rough estimation of their
   //nature.
   if (GetNUnidentifiedInGroup(grp) > 1) { //if there is more than one unidentified particle in the group

      UShort_t n_nseg_1 = 0;
      if (!grp->GetParticles()) {
         ::Error("KVReconstructedNucleus::AnalyseParticlesInGroup", "No particles in group ?");
         return;
      }
      TIter next(grp->GetParticles());
      KVReconstructedNucleus* nuc;
      //loop over particles counting up different cases
      while ((nuc = (KVReconstructedNucleus*) next())) {
         //ignore identified particles
         if (nuc->IsIdentified())
            continue;

         if (nuc->GetNSegDet() >= 2) {
            //all part.s crossing 2 or more independent detectors are fine
            nuc->SetStatus(KVReconstructedNucleus::kStatusOK);
         } else if (nuc->GetNSegDet() == 1) {
            //only 1 independent detector hit => depends on what's in the rest
            //of the group
            n_nseg_1++;
         } else {
            //part.s crossing 0 independent detectors (i.E. arret ChIo)
            //can not be reconstructed
            nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
         }
      }
      next.Reset();
      //loop again, setting status
      while ((nuc = (KVReconstructedNucleus*) next())) {
         if (nuc->IsIdentified())
            continue;           //ignore identified particles

         if (nuc->GetNSegDet() == 1) {
            if (n_nseg_1 == 1) {
               //just the one ? then we can get it no problem
               //after identifying the others and subtracting their calculated
               //energy losses from the "dependent"/"non-segmented" detector
               //(i.E. the ChIo)
               nuc->SetStatus(KVReconstructedNucleus::kStatusOKafterSub);
            } else {
               //more than one ? then we can make some wild guess by sharing the
               //"non-segmented" (i.e. ChIo) contribution between them, but
               //I wouldn't trust it as far as I can spit
               nuc->SetStatus(KVReconstructedNucleus::kStatusOKafterShare);
            }
            //one possibility remains: the particle may actually have stopped e.g.
            //in the DE detector of a DE-E telescope, in which case AnalStatus = 3
            if (nuc->GetIDTelescopes()->GetSize() == 0) {
               //no ID telescopes with which to identify particle
               nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
            }
         }
      }
   } else if (GetNUnidentifiedInGroup(grp) == 1) {
      //only one unidentified particle in group: if NSegDet>=1 then it's OK

      //loop over particles looking for the unidentified one
      TIter next(grp->GetParticles());
      KVReconstructedNucleus* nuc;
      while ((nuc = (KVReconstructedNucleus*) next()))
         if (!nuc->IsIdentified())
            break;

      if (nuc->GetNSegDet() > 0) {
         //OK no problem
         nuc->SetStatus(KVReconstructedNucleus::kStatusOK);
      } else {
         //dead in the water
         nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
      }
      //one possibility remains: the particle may actually have stopped e.g. in the 1st member
      //of a telescope, in which case AnalStatus = 3
      if (nuc->GetIDTelescopes()->GetSize() == 0) {
         //no ID telescopes with which to identify particle
         nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
      }
   }
#ifdef KV_DEBUG
   Info("AnalyseGroups", "OK after analysis of particles in groups");
#endif
}

//___________________________________________________________________________

void KVReconstructedNucleus::GetAnglesFromStoppingDetector(Option_t* opt)
{
   // Calculate angles theta and phi for reconstructed nucleus based
   // on detector in which it stopped*. The nucleus'
   // momentum is set using these angles, its mass and its kinetic energy.
   // The (optional) option string can be "random" or "mean".
   // If "random" (default) the angles are drawn at random between the
   // over the surface of the detector.
   // If "mean" the (theta,phi) position of the centre of the detector
   // is used to fix the nucleus' direction.
   //
   // *unless the detector directly in front of the stopping detector has
   //  a smaller solid angle, in which case we use that one (because the
   //  particle had to pass through the smaller angular range defined by
   //  the DE-detector)

   //don't try if particle has no correctly defined energy
   if (GetEnergy() <= 0.0)
      return;
   if (!GetStoppingDetector())
      return;

   KVDetector* angle_det = GetStoppingDetector();
   if (angle_det->GetNode()->GetNDetsInFront()) {
      KVDetector* d = (KVDetector*)angle_det->GetNode()->GetDetectorsInFront()->First();
      if (d->GetSolidAngle() < angle_det->GetSolidAngle())
         angle_det = d;
   }
   if (!strcmp(opt, "random")) {
      //random angles
      TVector3 dir = angle_det->GetRandomDirection("random");
      SetMomentum(GetEnergy(), dir);
   } else {
      //middle of telescope
      TVector3 dir = angle_det->GetDirection();
      SetMomentum(GetEnergy(), dir);
   }
}

//_________________________________________________________________________________

void KVReconstructedNucleus::Calibrate()
{
   //Calculate and set the energy of a (previously identified) reconstructed particle,
   //including an estimate of the energy loss in the target.
   //
   //Starting from the detector in which the particle stopped, we add up the
   //'corrected' energy losses in all of the detectors through which it passed.
   //Whenever possible, for detectors which are not calibrated or not working,
   //we calculate the energy loss. Measured & calculated energy losses are also
   //compared for each detector, and may lead to new particles being seeded for
   //subsequent identification. This is done by KVIDTelescope::CalculateParticleEnergy().
   //
   //For particles whose energy before hitting the first detector in their path has been
   //calculated after this step we then add the calculated energy loss in the target,
   //using gMultiDetArray->GetTargetEnergyLossCorrection().

   KVIDTelescope* idt = GetIdentifyingTelescope();
   idt->CalculateParticleEnergy(this);
   if (idt->GetCalibStatus() != KVIDTelescope::kCalibStatus_NoCalibrations) {
      SetIsCalibrated();
      //add correction for target energy loss - moving charged particles only!
      Double_t E_targ = 0.;
      if (GetZ() && GetEnergy() > 0) {
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         SetTargetEnergyLoss(E_targ);
      }
      Double_t E_tot = GetEnergy() + E_targ;
      SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      GetAnglesFromStoppingDetector();
   }
}

void KVReconstructedNucleus::MakeDetectorList()
{
   // Protected method, called when required to fill fDetList with pointers to
   // the detectors whose names are stored in fDetNames.
   // If gMultiDetArray=0x0, fDetList list will be empty.

   fDetList.Clear();
   if (gMultiDetArray) gMultiDetArray->FillDetectorList(this, &fDetList, fDetNames);
}

void KVReconstructedNucleus::SetIdentification(KVIdentificationResult* idr)
{
   // Set identification of nucleus from informations in identification result object
   // The mass (A) information in KVIdentificationResult is only used if the mass
   // was measured as part of the identification. Otherwise the nucleus' mass formula
   // will be used to calculate A from the measured Z.

   SetIDCode(idr->IDcode);
   SetZMeasured(idr->Zident);
   SetAMeasured(idr->Aident);
   SetZ(idr->Z);
   if (idr->Aident) {
      SetA(idr->A);
      SetRealA(idr->PID);
   } else {
      SetRealZ(idr->PID);
   }
   //debug
//   std::cout << "KVReconstructedNucleus::SetIdentification() IDCode=" << idr->IDcode << std::endl;
//   std::cout << "KVReconstructedNucleus::SetIdentification() Zident=" << idr->Zident << std::endl;
//   std::cout << "KVReconstructedNucleus::SetIdentification() Z=" << idr->Z << std::endl;
//   std::cout << "KVReconstructedNucleus::SetIdentification() PID=" << idr->PID << std::endl;
//   std::cout << "KVReconstructedNucleus::SetIdentification() RealZ=" << GetRealZ() << std::endl;
//   std::cout << "KVReconstructedNucleus::SetIdentification() Aident=" << idr->Aident << std::endl;
}

void KVReconstructedNucleus::SubtractEnergyFromAllDetectors()
{
   // Subtract the calculated energy loss of this particle from the measured energy
   // loss of all detectors it passed through.

   TIter nxt(GetDetectorList(), kIterBackward);
   KVDetector* det;
   Double_t Einc = GetEnergy() - GetTargetEnergyLoss(); // energy before first detector
   while ((det = (KVDetector*)nxt())) {
      Double_t Edet = det->GetEnergy();
      Double_t dE = det->GetDeltaE(GetZ(), GetA(), Einc); // calculate apparent energy loss in active layer
      Double_t Eres = det->GetERes(GetZ(), GetA(), Einc); // calculate energy after detector
      Edet -= dE;
      if (Edet < 0.1) Edet = 0.;
      det->SetEnergyLoss(Edet);
      Einc = Eres;
      if (Einc < 0.1) break;
   }
}

