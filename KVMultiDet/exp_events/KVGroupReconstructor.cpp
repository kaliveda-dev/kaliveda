//Created by KVClassFactory on Mon Oct 19 14:11:26 2015
//Author: John Frankland,,,

#include "KVGroupReconstructor.h"
#include "KVMultiDetArray.h"
#include "KVTarget.h"

#include <TPluginManager.h>
using std::cout;
using std::endl;

ClassImp(KVGroupReconstructor)

bool KVGroupReconstructor::fDoIdentification = kTRUE;
bool KVGroupReconstructor::fDoCalibration = kTRUE;

KVGroupReconstructor::KVGroupReconstructor()
   : KVBase("KVGroupReconstructor", "Reconstruction of particles in detector groups"),
     fGroup(nullptr), fGrpEvent(nullptr)
{
   // Default constructor

}

KVGroupReconstructor::~KVGroupReconstructor()
{
   // Destructor

   SafeDelete(fGrpEvent);
}

//________________________________________________________________

void KVGroupReconstructor::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVGroupReconstructor::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVGroupReconstructor& CastedObj = (KVGroupReconstructor&)obj;
}

void KVGroupReconstructor::SetGroup(KVGroup* g)
{
   // set the group to be reconstructed
   fGroup = g;
   fPartSeedCond = dynamic_cast<KVMultiDetArray*>(fGroup->GetArray())->GetPartSeedCond();
}

void KVGroupReconstructor::SetReconEventClass(TClass* c)
{
   // Instantiate event fragment object
   // Set condition for seeding reconstructed particles

   if (!fGrpEvent) fGrpEvent = (KVReconstructedEvent*)c->New();
}

KVGroupReconstructor* KVGroupReconstructor::Factory(const TString& plugin)
{
   // Create a new object of a class derived from KVGroupReconstructor defined by a plugin.
   // If plugin="" this is just a new KVGroupReconstructor instance

   if (plugin == "") return new KVGroupReconstructor;
   TPluginHandler* ph = LoadPlugin("KVGroupReconstructor", plugin);
   if (ph) {
      return (KVGroupReconstructor*)ph->ExecPlugin(0);
   }
   return nullptr;
}

void KVGroupReconstructor::Process()
{
   // Perform full reconstruction for group: reconstruct, identify, calibrate
   //   - identification can be inhibited (for all groups) by calling KVGroupReconstructor::SetDoIdentification(false);
   //   - calibration can be inhibited (for all groups) by calling KVGroupReconstructor::SetDoCalibration(false);

   nfireddets = 0;
   Reconstruct();
   if (GetEventFragment()->GetMult() == 0) {
      return;
   }
   if (fDoIdentification) Identify();
   if (fDoCalibration) Calibrate();
}

void KVGroupReconstructor::Reconstruct()
{
   // Reconstruct the particles in the group from hit trajectories
   // The condition for seeding particles is determined by the
   // multidetector to which the group belongs and the current dataset

   TIter nxt_traj(GetGroup()->GetTrajectories());
   KVGeoDNTrajectory* traj;

   // loop over trajectories
   while ((traj = (KVGeoDNTrajectory*)nxt_traj())) {

      // Work our way along the trajectory, starting from furthest detector from target,
      // start reconstruction of new detected particle from first fired detector.
      traj->IterateFrom();
      KVGeoDetectorNode* node;
      while ((node = traj->GetNextNode())) {

         KVReconstructedNucleus* kvdp;
         if ((kvdp = ReconstructTrajectory(traj, node))) {
            ReconstructParticle(kvdp, traj, node);
            break; //start next trajectory
         }

      }

   }

   if (GetEventFragment()->GetMult()) {
      AnalyseParticles();
      PostReconstructionProcessing();
   }
}

void KVGroupReconstructor::PostReconstructionProcessing()
{
   // This method will be called after reconstruction and first-order coherency analysis
   // of all particles in the group (if there are any reconstructed particles).
   // By default it does nothing.
}

KVReconstructedNucleus* KVGroupReconstructor::ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   KVDetector* d = node->GetDetector();
   nfireddets += d->Fired();
   // if d has fired and is either independent (only one trajectory passes through it)
   // or, if several trajectories pass through it,
   // only if the detector directly in front of it on this trajectory fired also
   if (!d->IsAnalysed() && d->Fired(fPartSeedCond)
         && (node->GetNTraj() == 1 ||
             (traj->GetNodeInFront(node) &&
              traj->GetNodeInFront(node)->GetDetector()->Fired()))) {

      return GetEventFragment()->AddParticle();
   }

   return nullptr;
}

void KVGroupReconstructor::ReconstructParticle(KVReconstructedNucleus* part, const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   // Reconstruction of a detected nucleus from the successive energy losses
   // measured in a series of detectors/telescopes along a given trajectory

   const KVReconNucTrajectory* Rtraj = (const KVReconNucTrajectory*)GetGroup()->GetTrajectoryForReconstruction(traj, node);
   part->SetReconstructionTrajectory(Rtraj);
   part->SetParameter("ARRAY", GetGroup()->GetArray()->GetName());

   Rtraj->IterateFrom();// iterate over trajectory
   KVGeoDetectorNode* n;
   while ((n = Rtraj->GetNextNode())) {

      KVDetector* d = n->GetDetector();
      d->AddHit(part);  // add particle to list of particles hitting detector

   }

}

void KVGroupReconstructor::AnalyseParticles()
{
   if (GetNUnidentifiedInGroup() > 1) { //if there is more than one unidentified particle in the group

      Int_t n_nseg_1 = 0;
      //loop over particles counting up different cases
      for (KVReconstructedEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
         KVReconstructedNucleus* nuc = it.get_pointer();
         //ignore identified particles
         if (nuc->IsIdentified())
            continue;
         // The condition for a particle to be identifiable straight away is that the first
         // identification method that will be used must be independent
         //if (nuc->GetNSegDet() >= 1) {
         if (nuc->GetReconstructionTrajectory()->GetIDTelescopes()
               && ((KVIDTelescope*)nuc->GetReconstructionTrajectory()->GetIDTelescopes()->First())->IsIndependent()) {
            //all particles whose first identification telescope is independent are fine
            nuc->SetStatus(KVReconstructedNucleus::kStatusOK);
         }
         else if (nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
            //no independent identification telescope => depends on what's in the rest of the group
            ++n_nseg_1;
         }
         else {
            //no identification available
            nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
         }
      }
      //loop again, setting status
      for (KVReconstructedEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
         KVReconstructedNucleus* nuc = it.get_pointer();
         if (nuc->IsIdentified())
            continue;           //ignore identified particles

         if (!(nuc->GetReconstructionTrajectory()->GetIDTelescopes()
               && ((KVIDTelescope*)nuc->GetReconstructionTrajectory()->GetIDTelescopes()->First())->IsIndependent())
               && nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
            //particles with no independent identification possibility
            if (n_nseg_1 == 1) {
               //just the one ? then we can get it no problem
               //after identifying the others and subtracting their calculated
               //energy losses from the other detectors
               nuc->SetStatus(KVReconstructedNucleus::kStatusOKafterSub);
            }
            else {
               //more than one ? then we can make some wild guess by sharing the
               //contribution between them, but I wouldn't trust it as far as I can spit
               nuc->SetStatus(KVReconstructedNucleus::kStatusOKafterShare);
            }
            //one possibility remains: the particle may actually have stopped e.g.
            //in the DE detector of a DE-E telescope
            if (!nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
               //no ID telescopes with which to identify particle
               nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
            }
         }
      }
   }
   else if (GetNUnidentifiedInGroup() == 1) {
      //only one unidentified particle in group: just need an idtelescope which works

      //loop over particles looking for the unidentified one
      KVReconstructedNucleus* nuc(nullptr);
      for (KVReconstructedEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
         nuc = it.get_pointer();
         if (!nuc->IsIdentified()) break;
      }
      //cout << "nuc->GetNSegDet()=" << nuc->GetNSegDet() << endl;
      if (nuc->GetReconstructionTrajectory()->GetNumberOfIdentifications()) {
         //OK no problem
         nuc->SetStatus(KVReconstructedNucleus::kStatusOK);
      }
      else {
         //dead in the water
         nuc->SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
      }
   }
}

void KVGroupReconstructor::IdentifyParticle(KVReconstructedNucleus& PART)
{
   // Try to identify this nucleus by calling the Identify() function of each
   // ID telescope crossed by it, starting with the telescope where the particle stopped, in order
   //   -  only telescopes which have been correctly initialised for the current run are used,
   //      i.e. those for which KVIDTelescope::IsReadyForID() returns kTRUE.
   //
   // This continues until a successful identification is achieved or there are no more ID telescopes to try.
   //
   // The identification code corresponding to the identifying telescope is set as the identification code of the particle.

   const KVSeqCollection* idt_list = PART.GetReconstructionTrajectory()->GetIDTelescopes();
   identifying_telescope = nullptr;
   id_by_type.clear();

   if (idt_list->GetEntries() > 0) {

      KVIDTelescope* idt;
      TIter next(idt_list);
      Int_t idnumber = 1;
      Int_t n_success_id = 0;//number of successful identifications
      while ((idt = (KVIDTelescope*) next())) {
         KVIdentificationResult* IDR = PART.GetIdentificationResult(idnumber++);
         IDR->SetIDType(idt->GetType());// without this, the identification type is never set!
         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?
            id_by_type[IDR->GetIDType()] = IDR;// map contains only attempted identifications
            IDR->IDattempted = kTRUE;
            idt->Identify(IDR);

            if (IDR->IDOK) n_success_id++;
         }
         else
            IDR->IDattempted = kFALSE;

         if (n_success_id < 1 &&
               ((!IDR->IDattempted) || (IDR->IDattempted && !IDR->IDOK))) {
            // the particle is less identifiable than initially thought
            // we may have to wait for secondary identification
            Int_t nseg = PART.GetNSegDet();
            PART.SetNSegDet(TMath::Max(nseg - 1, 0));
            //if there are other unidentified particles in the group and NSegDet is < 1
            //then exact status depends on segmentation of the other particles : reanalyse
            if (PART.GetNSegDet() < 1 && GetNUnidentifiedInGroup() > 1) {
               AnalyseParticles();
               return;
            }
            //if NSegDet = 0 it's hopeless
            if (!PART.GetNSegDet()) {
               AnalyseParticles();
               return;
            }
         }

      }
      // set first successful identification as particle identification
      // as long as the id telescope concerned contains the stopping detector!
      Int_t id_no = 1;
      Bool_t ok = kFALSE;
      KVIdentificationResult* pid = PART.GetIdentificationResult(id_no);
      next.Reset();
      idt = (KVIDTelescope*)next();
      while (idt->HasDetector(PART.GetStoppingDetector())) {
         if (pid->IDattempted && pid->IDOK) {
            ok = kTRUE;
            partID = *pid;
            identifying_telescope = idt;
            break;
         }
         ++id_no;
         pid = PART.GetIdentificationResult(id_no);
         idt = (KVIDTelescope*)next();
      }
      if (ok) {
         PART.SetIsIdentified();
         PART.SetIdentifyingTelescope(identifying_telescope);
         PART.SetIdentification(&partID, identifying_telescope);
      }

   }

}

//_________________________________________________________________________________

void KVGroupReconstructor::CalibrateParticle(KVReconstructedNucleus* PART)
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

   KVIDTelescope* idt = PART->GetIdentifyingTelescope();
   if (!idt) return;
   idt->CalculateParticleEnergy(PART);
   if (idt->GetCalibStatus() != KVIDTelescope::kCalibStatus_NoCalibrations) {
      PART->SetIsCalibrated();
      //add correction for target energy loss - moving charged particles only!
      Double_t E_targ = 0.;
      if (PART->GetZ() && PART->GetEnergy() > 0) {
         E_targ = GetTargetEnergyLossCorrection(PART);
         PART->SetTargetEnergyLoss(E_targ);
      }
      Double_t E_tot = PART->GetEnergy() + E_targ;
      PART->SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      PART->GetAnglesFromReconstructionTrajectory();
   }
}

//_________________________________________________________________________________

void KVGroupReconstructor::TreatStatusStopFirstStage(KVReconstructedNucleus& d)
{
   // particles stopped in first member of a telescope
   // estimation of Z (minimum) from energy loss (if detector is calibrated)
   Int_t zmin = d.GetStoppingDetector()->FindZmin(-1., d.GetMassFormula());
   if (zmin) {
      d.SetZ(zmin);
      d.SetIsIdentified();
      KVGeoDNTrajectory* t = (KVGeoDNTrajectory*)d.GetStoppingDetector()->GetNode()->GetTrajectories()->First();
      d.SetIdentifyingTelescope((KVIDTelescope*) t->GetIDTelescopes()->Last());
   }
}

void KVGroupReconstructor::Identify()
{
   // All particles which have not been previously identified (IsIdentified=kFALSE), and which
   // may be identified independently of all other particles in their group according to the 1st
   // order coherency analysis (KVReconstructedNucleus::GetStatus=0), will be identified.
   // Particles stopping in first member of a telescope (KVReconstructedNucleus::GetStatus=3) will
   // have their Z estimated from the energy loss in the detector (if calibrated).

   for (KVReconstructedEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
      KVReconstructedNucleus& d = it.get_reference();
      if (!d.IsIdentified()) {
         if (d.GetStatus() == KVReconstructedNucleus::kStatusOK) {
            // identifiable particles
            IdentifyParticle(d);
         }
         else if (d.GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
            // particles stopped in first member of a telescope
            // estimation of Z (minimum) from energy loss (if detector is calibrated)
            TreatStatusStopFirstStage(d);
         }
      }
   }
}

//_____________________________________________________________________________

void KVGroupReconstructor::Calibrate()
{
   // Calculate and set energies of all identified but uncalibrated particles in event.

   KVReconstructedNucleus* d;

   while ((d = GetEventFragment()->GetNextParticle())) {

      if (d->IsIdentified() && !d->IsCalibrated()) {
         CalibrateParticle(d);
      }

   }

}

Double_t KVGroupReconstructor::GetTargetEnergyLossCorrection(KVReconstructedNucleus* ion)
{
   // Calculate the energy loss in the current target of the multidetector
   // for the reconstructed charged particle 'ion', assuming that the current
   // energy and momentum of this particle correspond to its state on
   // leaving the target.
   //
   // WARNING: for this correction to work, the target must be in the right 'state':
   //
   //      gMultiDetArray->GetTarget()->SetIncoming(kFALSE);
   //      gMultiDetArray->GetTarget()->SetOutgoing(kTRUE);
   //
   // (see KVTarget::GetParticleEIncFromERes).
   //
   // The returned value is the energy lost in the target in MeV.
   // The energy/momentum of 'ion' are not affected.

   KVMultiDetArray* array = (KVMultiDetArray*)GetGroup()->GetArray();
   if (!array->GetTarget() || !ion) return 0.0;
   return (array->GetTarget()->GetParticleEIncFromERes(ion) - ion->GetEnergy());
}

