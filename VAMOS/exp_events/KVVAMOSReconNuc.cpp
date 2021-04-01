//Created by KVClassFactory on Thu Sep 13 10:51:51 2012
//Author: Guilain ADEMARD

#include "KVVAMOSReconNuc.h"
#include "KVVAMOSDetector.h"
#include "KVVAMOSTransferMatrix.h"
#include "KVNamedParameter.h"
#include "KVTarget.h"
#include "TGraphErrors.h"
#include "TROOT.h"
#include "KVIDQA.h"
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"
#include "KVVAMOSDataCorrection.h"

ClassImp(KVVAMOSReconNuc)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVVAMOSReconNuc</h2>
   <h4>Nucleus identified by VAMOS spectrometer</h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVVAMOSReconNuc::KVVAMOSReconNuc() : fCodes()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVVAMOSReconNuc::KVVAMOSReconNuc(const KVVAMOSReconNuc& obj)  : KVReconstructedNucleus(), fCodes()
{
   // Copy constructor
   init();
   obj.Copy(*this);
}

KVVAMOSReconNuc::~KVVAMOSReconNuc()
{
   // Destructor
}
//________________________________________________________________

void KVVAMOSReconNuc::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVVAMOSReconNuc. The array (fDetE) of the contributions
   // of each detectors to the nucleus' energy is written/read if the nucleus
   // is calibrated.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      R__b.ReadClassBuffer(KVVAMOSReconNuc::Class(), this, R__v, R__s, R__c);
      if (R__v <= 3 && IsCalibrated()) {
         Int_t N = GetDetectorList()->GetEntries();
         if (R__v < 3) {
            // Before Version 3 of KVVAMOSReconNuc, fDetE was Float_t array
            Float_t* tmp(new Float_t[N]);
            R__b.ReadFastArray(tmp, N);
            fDetE.clear();
            fDetE.resize(N);
            for (Int_t i = 0; i < N; i++) fDetE[i] = (Double_t)tmp[i];
            delete[] tmp;
         }
         else if (R__v == 3) {
            // In version 3, fDetE is Double_t array
            Double_t* tmp(new Double_t[N]);
            R__b.ReadFastArray(tmp, N);
            fDetE.clear();
            fDetE.resize(N);
            for (Int_t i = 0; i < N; i++) fDetE[i] = tmp[i];
            delete[] tmp;
         }
      }
   }
   else {
      R__b.WriteClassBuffer(KVVAMOSReconNuc::Class(), this);
   }
}

//________________________________________________________________

void KVVAMOSReconNuc::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVVAMOSReconNuc::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVReconstructedNucleus::Copy(obj);
   KVVAMOSReconNuc& CastedObj = (KVVAMOSReconNuc&)obj;
   CastedObj.fCodes    = fCodes;
   CastedObj.fRT       = fRT;
   CastedObj.fDetE     = fDetE;
   CastedObj.fStripFoilEloss  = fStripFoilEloss;
   CastedObj.fBasicToF     = fBasicToF;
   CastedObj.fBasicPath    = fBasicPath;
   CastedObj.fBasicRealAE  = fBasicRealAE;
   CastedObj.fBasicRealAoQ = fBasicRealAoQ;
   CastedObj.fBasicRealQ   = fBasicRealQ;
   CastedObj.fBasicQ       = fBasicQ;
   CastedObj.fBasicRealA   = fBasicRealA;
   CastedObj.fBasicA       = fBasicA;
   CastedObj.fBasicNHF     = fBasicNHF;
   CastedObj.fkIsBasicQandAID = fkIsBasicQandAID;

   CastedObj.fToF_corr     = fToF_corr;
   CastedObj.fPath_corr    = fPath_corr;
   CastedObj.fRealAE_corr  = fRealAE_corr;
   CastedObj.fRealAoQ_corr = fRealAoQ_corr;
   CastedObj.fRealQ_corr   = fRealQ_corr;
   CastedObj.fRealA_corr   = fRealA_corr;
   CastedObj.fKE_corr      = fKE_corr;
   CastedObj.fEBeforeVAMOS_corr = fEBeforeVAMOS_corr;
   CastedObj.fA_CsI        = fA_CsI;
   CastedObj.fCorrNHF      = fCorrNHF;
   CastedObj.fkIsCorrected = fkIsCorrected;
   CastedObj.fkIsCorrQandAID = fkIsCorrQandAID;
}
//________________________________________________________________

void KVVAMOSReconNuc::init()
{
   //default initialisations
   if (gDataSet)
      SetMassFormula(UChar_t(gDataSet->GetDataSetEnv("KVVAMOSReconNuc.MassFormula", Double_t(kEALMass))));

   fStripFoilEloss = 0;
   fDetE.clear();

   //basic Q and A ID results
   fBasicToF     = -666.;
   fBasicPath    = -666.;
   fBasicRealAE  = -666.;
   fBasicRealAoQ = -666.;
   fBasicRealQ   = -666.;
   fBasicQ       = -666;
   fBasicRealA   = -666.;
   fBasicA       = -666;
   fBasicNHF     = -666.;

   //corrected Q and A ID results
   fToF_corr     = -666.;
   fPath_corr    = -666.;
   fRealAE_corr  = -666.;
   fRealAoQ_corr = -666.;
   fRealQ_corr   = -666.;
   fRealA_corr   = -666.;
   fKE_corr      = -666.;
   fEBeforeVAMOS_corr = -666.;
   fCorrNHF      = -666.;

   fA_CsI        = -666;

   fkIsCorrected    = kFALSE;
   fkIsCorrQandAID  = kFALSE;
   fkIsBasicQandAID = kFALSE;

   fdebug = kFALSE;
}
//________________________________________________________________

void KVVAMOSReconNuc::Calibrate()
{
   //Calculate and set the energy of a (previously identified) reconstructed nucleus,
   //including an estimate of the energy loss in the stripping foil and in the target.
   //
   //Starting from the detector in which the nucleus stopped, we add up the
   //'corrected' energy losses in all of the detectors through which it passed.
   //Whenever possible, for detectors which are not calibrated or not working,
   //we calculate the energy loss.
   //
   //For nuclei whose energy before hitting the first detector in their path has been
   //calculated after this step we then add the calculated energy loss in stripping foil and in the target,
   //using:
   //   gVamos->GetStripFoilEnergyLossCorrection();
   //   gMultiDetArray->GetTargetEnergyLossCorrection().

   if (gVamos->Calibrate(this)) {
      // if the nucleus is calibrated by this way then the energy losses
      // in each detector are not calculated then they are set to -1
      if (fdebug) Info("Calibrate", "... gVamos->Calibrate() was success, CalibrateFromDetList() won't be called ...");
      if (fDetE.empty()) {
         Int_t N = GetDetectorList()->GetEntries();
         fDetE.resize(N, -1.);
      }
   }
   else {
      if (fdebug) Info("Calibrate", "... gVamos->Calibrate() wasn't success, CalibrateFromDetList() will be called ...");
      CalibrateFromDetList();
   }


   if (IsCalibrated() && GetEnergy() > 0) {

      // set angles of momentum from trajectory reconstruction
      if (fRT.FPtoLabWasAttempted()) {
         SetTheta(GetThetaI());
         SetPhi(GetPhiI());
      }

      if (GetZ()) {

         Double_t E_tot   = GetEnergy();
         Double_t E_sfoil = 0.;
         Double_t E_targ  = 0.;

         //add correction for stripping foil energy loss - moving charged particles only
         E_sfoil = gVamos->GetStripFoilEnergyLossCorrection(this);
         SetStripFoilEnergyLoss(E_sfoil);
         SetEnergy(E_tot += E_sfoil);

         //add correction for target energy loss - moving charged particles only
         if (gMultiDetArray->GetTarget()) {
            E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
            SetTargetEnergyLoss(E_targ);
            SetEnergy(E_tot += E_targ);
         }
      }
   }
}

//________________________________________________________________
void KVVAMOSReconNuc::CalibrateFromDetList()
{
   // The energy of each particle is calculated as follows:
   //
   //      E = dE_1 + dE_2 + ... + dE_N
   //
   // dE_1, dE_2, ... = energy losses measured in each detector through which
   //                          the particle has passed (or stopped, in the case of dE_N).
   //                         These energy losses are corrected for (Z,A)-dependent effects
   //                          such as pulse-heigth defect in silicon detectors, losses in
   //                          windows of gas detectors, etc.
   //
   // If none of the detectors of the detector list is calibrated, the particle's energy cannot be calculated &
   // the code will be kECode0.
   // Whenever possible, the energy loss for fired detectors which are uncalibrated
   // or not functioning is calculated.
   // If the detectors used to measure energy are not calibrated or not fired, or multihit, the
   // energy code will be kECode2.
   // Otherwise, the code will be kECode1.
   // The flag returned by IsCalibrated will be true is the energy code is different from kECode0.

   //status code
   SetECode(kECode0);

   //uncharged particles
   if (GetZ() == 0) return;

   SetECode(kECode1);
   SetIsCalibrated();

   Double_t Etot = 0;
   Bool_t stopdetOK = kTRUE;
   KVVAMOSDetector* stopdet = (KVVAMOSDetector*)GetStoppingDetector();
   KVVAMOSDetector* det     = NULL;
   TIter next_det(GetDetectorList());
   if (fDetE.empty()) fDetE.resize(GetDetectorList()->GetEntries(), 0.);
   Int_t idx = -1;
   while ((det = (KVVAMOSDetector*)next_det())) {
      fDetE[++idx] = 0.;
      if (!stopdetOK) continue;
      // transmission=kFALSE if particle stop in det
      Bool_t transmission = (det != stopdet);

      // The stopping detector has to be fired, calibrated, hit by only one
      // particle
      if (!transmission && !((det->GetNHits() == 1) && det->Fired() && det->IsECalibrated())) {
         SetECode(kECode0);
         SetIsUncalibrated();
         stopdetOK = kFALSE;
         continue;
      }

      // modify thicknesses of all layers according to orientation,
      // and store original thicknesses in array. Effective thickness
      // is previously calculated in the method 'Propagate' and stored
      // in the list 'fParameters'
      Double_t* thickness;
      thickness = new Double_t[det->GetListOfAbsorbers()->GetEntries()];
      KVMaterial* abs;
      Int_t i = 0;
      TIter next_abs(det->GetListOfAbsorbers());
      TIter next_par(GetParameters()->GetList());
      TString parname, absname;
      while ((abs = (KVMaterial*) next_abs())) {
         thickness[i++] = abs->GetThickness();
         absname = abs->GetAbsGeoVolume()->GetName();
         if (absname.BeginsWith("DET_")) parname.Form("STEP:%s", det->GetName());
         else parname.Form("STEP:%s/%s", det->GetName(), absname.Data());
         KVNamedParameter* par = NULL;
         while ((par = (KVNamedParameter*)next_par())) {
            if (!strncmp(parname.Data(), par->GetName(), parname.Sizeof() - 1)) {
               abs->SetThickness(par->GetDouble());
//             Info("CalibrateFromDetList","thickness of absorber %s has been changed: %f --> %f", parname.Data(), thickness[i-1], par->GetDouble());
               break;
            }
         }
      }

      det->SetEResAfterDetector(Etot);
      Double_t Edet = det->GetCorrectedEnergy(this, -1, transmission);

      // Detector is calibrated and hit by only one particle
      if (Edet > 0 &&  det->GetNHits() == 1) {
         det->SetEResAfterDetector(Etot);

         // if Edet is greater than the maximal possible theoretical value
         // this may be because Z or A are underestimated and one of both
         // has to be increased (see KVDetector::GetCorrectedEnergy)
         if (GetParameters()->GetIntValue("GetCorrectedEnergy.Warning") == 1) {
            Int_t z0 = GetZ();
            Int_t a0 = GetA();
            Int_t z = z0, a = a0;
            Bool_t isOK = kFALSE;
            Edet = det->GetEnergy();

            //Warning("CalibrateFromDetList", "MeasuredDE>MaxDE in %s, Z= %d, A=%d, MeasuredDE= %f MeV, MaxDE %f MeV", det->GetName(), z0, a0, Edet, det->GetMaxDeltaE(z0, a0));

            while (!isOK && (z - GetZ() <= 10)) {
               //increase Z
               ++z;
               //modify A only if it is not measured
               if (!IsAMeasured()) a = GetAFromZ(z, (Char_t)GetMassFormula());
               isOK  = (Edet <= det->GetMaxDeltaE(z, a));
               //Info("CalibrateFromDetList", "changing Z= %d, A=%d, MeasuredDE= %f MeV, MaxDE %f MeV", z, a, Edet, det->GetMaxDeltaE(z, a));
            }
            //Info("CalibrateFromDetList", "MeasuredDE<MaxDE in %s by changing Z: %d->%d, A: %d->%d \n", det->GetName(), z0 , z, a0 , a);
            SetZandA(z, a);
            det->SetEResAfterDetector(Etot);
            Edet  = det->GetCorrectedEnergy(this, -1, transmission);
            // we back to initial values of Z and A
            SetZandA(z0, a0);
         }

         Etot += Edet;
         fDetE[idx] = Edet;
//       Info("CalibrateFromDetList","Corrected DeltaE= %f in %s, idx= %d", Edet, det->GetName(), idx);
      }
      // Detector is uncalibrated/unfired/multihit. Calculate energy loss.
      // calculate energy of particle before detector from energy after
      // detector
      else {

         Edet = det->GetDeltaEFromERes(GetZ(), GetA(), Etot);
         if (Edet < 0.0) Edet = 0.0;

         if (det->GetNHits() > 1) {
            if (!(det->Fired() && det->IsECalibrated()))
               det->SetEnergyLoss(Edet + det->GetEnergy());// sum up calculated energy losses in uncalibrated detector
            //Info("CalibrateFromDetList", "MultiHit in %s", det->GetName());
         }
         else if (!det->Fired() || !det->IsECalibrated())
            det->SetEnergyLoss(Edet);

         det->SetEResAfterDetector(Etot);
         Edet  = det->GetCorrectedEnergy(this, Edet, transmission);
         Etot += Edet;
         fDetE[idx] = Edet;
         if (det->IsUsedToMeasure("E")) SetECode(kECode2);
         //Info("CalibrateFromDetList", "Calculated DeltaE= %f in %s, idx= %d", Edet, det->GetName(), idx);
      }

      // reset thicknesses of absorbers
      i = 0;
      next_abs.Reset();
      while ((abs = (KVMaterial*) next_abs())) {
         abs->SetThickness(thickness[i++]);
      }
      delete [] thickness;
   }
   SetEnergy(Etot);
}
//________________________________________________________________

void KVVAMOSReconNuc::Clear(Option_t* t)
{
   //Reset nucleus' properties

   KVReconstructedNucleus::Clear(t);
   init();
   fCodes.Clear();
   fRT.Reset();
}
//________________________________________________________________
Bool_t KVVAMOSReconNuc::StoppedInChIo()
{
   //Returns kTRUE if particle stopped in ChIo

   if (!strcmp(GetStoppingDetector()->GetType(), "CHI")) {
      return kTRUE;
   }
   else {
      return kFALSE;
   }
}

Bool_t KVVAMOSReconNuc::StoppedInSi()
{
   //Returns kTRUE if particle stopped in Si wall

   if (!strcmp(GetStoppingDetector()->GetType(), "SI")) {
      return kTRUE;
   }
   else {
      return kFALSE;
   }
}

Bool_t KVVAMOSReconNuc::StoppedInCsI()
{
   //Returns kTRUE if particle stopped in CsI wall

   if (!strcmp(GetStoppingDetector()->GetType(), "CSI")) {
      return kTRUE;
   }
   else {
      return kFALSE;
   }
}
//________________________________________________________________

void KVVAMOSReconNuc::GetAnglesFromReconstructionTrajectory(Option_t*)
{
   // Overwrites the same method of the mother class. This method is obsolete
   // for a nucleus reconstructed in VAMOS because angles are set by the
   // reconstruction.
   // Overwriting is important because this method is called in the Streamer
   // method of KVReconstructedEvent to calculate again the angles.
   return;
}
//________________________________________________________________

Float_t KVVAMOSReconNuc::CalculateRealAE(const Char_t* tof_name) const
{
   // Returns the real value of the mass number calculated for the
   // measured energy and the measured time of flight, without any
   // corrections
   // Begin_Latex
   // A = #frac{E}{(#gamma-1)u}
   // End_Latex
   // where
   //   E     : corrected kinetic energy in (MeV)
   //   u     : atomic mass unit in MeV/c^2
   //   gamma : Lorentz factor calculated from the velocity
   //           deduced from the time of flight measurment
   //           with name tof_name
   //

   //return CalculateRealAE(GetZ(), GetEnergyBeforeVAMOS(), GetBeta(tof_name));
   Double_t dist, tof;
   dist = tof = 0.;
   Int_t nHF;
   nHF = 0;
   return CalculateRealAE(GetEnergyBeforeVAMOS(), CalculateBeta(dist, tof, nHF, tof_name));
}
//________________________________________________________________

Float_t KVVAMOSReconNuc::CalculateRealAoverQ(const Char_t* tof_name) const
{
   // returns the ratio between the mass number A and the charge state Q
   // calculated from the measurment of the Time of Flight of the nucleus,
   // BEFORE any correction of the data.
   // The returned value is real. Returns zero if the time of flight is
   // not correct.
   // Begin_Latex
   // #frac{A}{Q} = #frac{10 C}{u} #frac{B_{#rho}}{ #gamma #beta}
   // End_Latex
   // where
   //   u             : atomic mass unit in MeV/c^2
   //   C             : speed of light in vacuum in cm/ns
   //   beta and gamma: relativistic quantities calculated from the velocity
   //                   deduced from the time of flight measurment
   //                   with name tof_name
   Double_t dist, tof;
   dist = tof = 0.;
   Int_t nHF;
   nHF = 0;

   return CalculateRealAoverQ(GetBrho(), CalculateBeta(dist, tof, nHF, tof_name)) / u();
}
//________________________________________________________________

Float_t KVVAMOSReconNuc::CalculateRealQ(const Char_t* tof_name) const
{
   // Returns the real value of the charge state calculated for the
   // measured energy and the measured time of flight (tof_name).
   Double_t dist, tof;
   dist = tof = 0.;
   Int_t nHF;
   nHF = 0;

   Double_t beta = CalculateBeta(dist, tof, nHF, tof_name);
   Double_t A    = CalculateRealAE(GetEnergyBeforeVAMOS(), beta);
   Double_t AoQ  = CalculateRealAoverQ(GetBrho(), beta) / u();
   return (AoQ > 0. ? A / AoQ : 0.);
}

//________________________________________________________________

void KVVAMOSReconNuc::IdentifyZ()
{
   // VAMOS-specific Z identification.
   // Here we attribute the general identification codes and the subcodes from the different
   // identification algorithms:
   //
   //IDENTIFIED NUCLEI
   //Identified nuclei with ID code = 2 with subcodes 4 & 5
   //(masse hors limite superieure/inferieure) are relabelled
   //with kIDCode10 (identification entre les lignes CsI)
   //
   //UNIDENTIFIED NUCLEI
   //Unidentified nuclei receive the general ID code for non-identified particles (kIDCode14)

   Bool_t ok = kFALSE;
   const KVSeqCollection* idt_list = GetIDTelescopes();
   if (idt_list && idt_list->GetSize() > 0) {

      KVIDTelescope* idt;
      TIter next(idt_list);
      Int_t idnumber = 1;
      while ((idt = (KVIDTelescope*) next())) {
         // if it is not a ID-telescope for Z-identification
         // then go to the next one
         if (idt->InheritsFrom(KVIDQA::Class())) continue;

         KVIdentificationResult* IDR = GetIdentificationResult(idnumber++);

         if (IDR) {
            if (idt->IsReadyForID()) { // is telescope able to identify for this run ?
               IDR->IDattempted = kTRUE;
               idt->Identify(IDR);

               if (!ok && IDR->IDOK) {
                  ok = kTRUE;
                  SetIsZidentified(); //Set bit to kIdentified and add 1 identified particle and subtract 1 unidentified particle from each detector in its list
                  KVIDTelescope* idt = (KVIDTelescope*)idt_list->FindObjectByType(IDR->GetType());
                  if (!idt) {
                     Warning("IdentifyZ", "cannot find ID telescope with type %s", IDR->GetType());
                     idt_list->ls();
                     IDR->Print();
                  }

                  //Setting the identification
                  SetZIdentification(IDR, idt);

                  if (fdebug) {
                     Info("IdentifyZ", "... Z identification set, printing particle infos ... \nIDCode=%d \nZMeasured=%d, AMeasured=%d \nRealZ=%lf, RealA=%lf, \nZ=%d, A=%d, Aminimizer=%d",
                          GetIDCode(), (int) IsZMeasured(), (int) IsAMeasured(), GetRealZ(), GetRealA(), GetZ(), GetA(), GetAMinimizer());
                  }
               }
            }
            else
               IDR->IDattempted = kFALSE;
         };
      }
   }

   if (!ok) {
      /******* UNIDENTIFIED PARTICLES *******/

      /*** general ID code for non-identified particles ***/
      SetIDCode(kIDCode14);
   }
}
//________________________________________________________________

void KVVAMOSReconNuc::IdentifyQandA()
{
   // VAMOS-specific Q and A identification.
   //
   // +e494s: First we loop over each Q-A ID telescope placed at the front
   // of the stopping detector. For each ID telescope we loop
   // over each time of flight listed in the environment variable
   // KVVAMOSCodes.ACQParamListForToF to look for the corresponding
   // ID grid before to perform identification. This process is stopped
   // once an identification is OK. Then the corresponding TCode and
   // the identification result are set to this nucleus
   //
   // +e503:

   //Raw identification:
   //Apply VAMOS routine identification
   //without any application of data corrections
   //and save the associated informations.
   Bool_t ok = kFALSE;
   const KVSeqCollection* idt_list = GetIDTelescopes();
   if (idt_list && idt_list->GetSize() > 0) {

      KVIDTelescope* idt;
      TIter next(idt_list);
      while ((idt = (KVIDTelescope*) next())) {

         // is telescope able to identify for this run ?
         if (!idt->IsReadyForID()) continue;

         //------e494s experiment case------
         if (idt->InheritsFrom(KVIDQA::Class())) {// if ID-telescope for Q and A identification
            KVIDQA* qa_idt = (KVIDQA*)idt;

            static KVIdentificationResult IDR;

            // loop over the time acquisition parameters
            const Char_t* tof_name = NULL;
            for (Short_t i = 0; !ok && (tof_name = GetCodes().GetToFName(i)); i++) {
               IDR.Clear();
               IDR.IDattempted = kTRUE;

               Double_t dist, tof;
               dist = tof = 0.;
               Int_t nHF;
               nHF = 0;
               Double_t beta    = CalculateBeta(dist, tof, nHF, tof_name);
               Double_t realAE  = CalculateRealAE(GetEnergyBeforeVAMOS(), beta);
               Double_t realAoQ = CalculateRealAoverQ(GetBrho(), beta) / u();

               qa_idt->Identify(&IDR, tof_name, realAoQ, realAE);
               // for all nuclei we take the first identification which gives IDOK==kTRUE
               if (IDR.IDOK) {
                  SetBasicQandAIdentification(&IDR);
                  SetBasicRealAE(realAE);
                  SetBasicRealAoverQ(realAoQ);
                  SetBasicRealQ(qa_idt->GetRealQ());
                  SetBasicToF(tof);
                  SetBasicPath(dist);
                  SetBasicNHF(nHF);

                  if (fdebug) {
                     Info("IdentifyQandA", "ToF=%lf, nHF=%d, Path=%lf, Beta=%lf, RealAE=%lf, RealAoQ=%lf, RealQ=%lf, RealA=%lf, Q=%d, A=%d",
                          GetBasicToF(), GetBasicNHF(), GetBasicPath(), GetBasicBeta(), GetBasicRealAE(),
                          GetBasicRealAoverQ(), GetBasicRealQ(), GetBasicRealA(), GetBasicQ(), GetBasicA());
                  }

                  return;
               }
            }
         }

         //------e503 experiment case------
         else {
            if (idt->InheritsFrom(KVIDHarpeeICSi_e503::Class()) || idt->InheritsFrom(KVIDHarpeeSiCsI_e503::Class())) {
               // loop over the time acquisition parameters
               const Char_t* tof_name = NULL;

               for (Short_t i = 0; !ok && (tof_name = GetCodes().GetToFName(i)); i++) {
                  const KVString str(tof_name);
                  if (str != "TSI_HF") continue;

                  Double_t dist, tof;
                  dist = tof = 0.;
                  Int_t nHF;
                  nHF = 0;
                  Double_t beta    = CalculateBeta(dist, tof, nHF, tof_name);
                  Double_t realAE  = CalculateRealAE(GetEnergyBeforeVAMOS(), beta);
                  Double_t realAoQ = CalculateRealAoverQ(GetBrho(), beta) / u();

                  //Set the basic ID results
                  SetBasicQandAIdentification(tof_name, tof, dist, nHF, realAE, realAoQ);


                  if (fdebug) {
                     Info("IdentifyQandA", "ToF=%lf, nHF=%d, Path=%lf, Beta=%lf, RealAE=%lf, RealAoQ=%lf, RealQ=%lf, RealZ=%lf, RealA=%lf, Q=%d, Z=%d, A=%d",
                          GetBasicToF(), GetBasicNHF(), GetBasicPath(), GetBasicBeta(), GetBasicRealAE(),
                          GetBasicRealAoverQ(), GetBasicRealQ(), GetRealZ(), GetBasicRealA(), GetBasicQ(), GetZ(), GetBasicA());
                  }

                  return;
               }
            }

            //------ ??? stg is wrong, Not ID-Telescope for e494s either e503 ??? ------
            else continue;
         }
      }
   }

   /******* UNIDENTIFIED PARTICLES *******/
   /*** general ID code for non-identified particles ***/
   SetTCode(kTCode0);
}
//________________________________________________________________

void KVVAMOSReconNuc::MakeDetectorList()
{
   // Protected method, called when required to fill fDetList with pointers to
   // the detectors whose names are stored in fDetNames.
   // If gVamos=0x0, fDetList list will be empty.

   fDetList.Clear();
   if (gVamos) {
      fDetNames.Begin("/");
      while (!fDetNames.End()) {
         KVDetector* det = gVamos->GetDetector(fDetNames.Next(kTRUE));
         if (det) fDetList.Add(det);
      }
   }
}
//________________________________________________________________

void KVVAMOSReconNuc::ReconstructTrajectory()
{
   //Reconsturction of the trajectory at the focal plane and then at
   //the target point.

   if (ReconstructFPtrajByFitting()) {
      if (ReconstructLabTraj()) Propagate();
      else SetFPCode(kFPCode0);
   }
}
//________________________________________________________________

void KVVAMOSReconNuc::ReconstructFPtraj()
{
   // Reconstruct the Focal-Plane trajectory and set the appropriate
   // FPCode. If the reconstruction is well carried out (i.e. FPCode>kFPCode0)
   // then the tracking along this trajectory is runned.

   UChar_t res = 0;
   Double_t xyzf[3];          // [3] => [X, Y, Z]
   Double_t XYZf[4][3];       // [4] => [Complete Det1, Complete Det2, Incomplete Det1, Incomplete Det2]
   // [3] => [X, Y, Z]
   Short_t Idx[4] = { -1, -1, -1, -1}; // [4] => [Complete Det1, Complete Det2, Incomplete Det1, Incomplete Det2]
   UChar_t IncDetBitmask = 3;        // bits 11 => 3, first bit means missing X, second bit means missing Y
   Bool_t inc1IsX = kTRUE;           // true if the coordinate given by the first incomplete position is X ( false if Y)
   UShort_t Ncomp = 0, Ninc = 0;     // Number of Complete and Incomplete
   // position measurments

   SetFPCode(kFPCode0);   // Initialize FP codes to code 0 "no FP position recon."
   fRT.dirFP.SetXYZ(0, 0, 1);

   const Char_t* FPdetName = NULL;
   KVVAMOSDetector* d = NULL;
   // Loop over detector name to be used for the Focal plane Position
   // reconstruction
   for (Short_t i = 0; (FPdetName = GetCodes().GetFPdetName(i)); i++) {

      // Look at only the detectors in 'fDetList' with the name or
      // type 'FPdetName' measuring a position.
      if (
         ((d = (KVVAMOSDetector*)GetDetectorList()->FindObject(FPdetName)) ||
          (d = (KVVAMOSDetector*)GetDetectorList()->FindObjectByType(FPdetName)))
         && (res = d->GetPosition(xyzf)) > 4
      ) {


         if (res == 7) {
            // X, Y and Z measured bits 111 => 7
            // Complete position (XYZ)

            Idx[ Ncomp ] = i;
            for (Int_t j = 0; j < 3; j++) XYZf[ Ncomp ][ j ] = xyzf[ j ];
            Ncomp++;
         }
         else if (IncDetBitmask & res) {
            // look at only incomplete position measurment (XZ or YZ)
            // not found yet

            Idx[ 2 + Ninc ] = i;
            for (Int_t j = 0; j < 3; j++) XYZf[ 2 + Ninc ][ j ] = xyzf[ j ];
            if (Ninc == 0) inc1IsX = res & 1;
            Ninc++;
            IncDetBitmask = (IncDetBitmask | 4)^res;
         }


         if (Ncomp == 2) {
            //Case where the Focal plane Position is reconstructed from 2 complete position measurment.

            fRT.dirFP.SetX((XYZf[1][0] - XYZf[0][0]) / (XYZf[1][2] - XYZf[0][2]));
            fRT.dirFP.SetY((XYZf[1][1] - XYZf[0][1]) / (XYZf[1][2] - XYZf[0][2]));
            GetCodes().SetFPCode(Idx[0], Idx[1], Idx[2], Idx[3], inc1IsX);
            break;
         }
         else if ((Ncomp == 1) && !IncDetBitmask) {
            //Case where the Focal plane Position is reconstructed from 1 complete position measurment and 2 incomplete position measurment.

            fRT.dirFP.SetX((XYZf[3 - inc1IsX][0] - XYZf[0][0]) / (XYZf[3 - inc1IsX][2] - XYZf[0][2]));
            fRT.dirFP.SetY((XYZf[2 + inc1IsX][1] - XYZf[0][1]) / (XYZf[2 + inc1IsX][2] - XYZf[0][2]));
            GetCodes().SetFPCode(Idx[0], Idx[1], Idx[2], Idx[3], inc1IsX);
            break;
         }

      }
   }

   if (GetCodes().TestFPCode(kFPCode0)) return;

   // normalize the direction vector dirFP
   fRT.dirFP *= 1. / fRT.dirFP.Mag();

   // Xf = Xc1      - Zc1*tan( Thetaf )
   fRT.pointFP[0] = XYZf[0][0] - XYZf[0][2] * fRT.dirFP.X() / fRT.dirFP.Z();

   // Yf = Yc1      - Zc1*tan( Phif   )
   fRT.pointFP[1] = XYZf[0][1] - XYZf[0][2] * fRT.dirFP.Y() / fRT.dirFP.Z();

   fRT.SetFPparamsReady();

   //    Info("ReconstructFPtraj","\n    Xf= %f, Yf= %f, Thetaf= %f, Phif= %f\n    FPCode%d %s", GetXf(), GetYf(),GetThetaF(),GetPhiF(),GetCodes().GetFPCodeIndex(), GetCodes().GetFPStatus());
}
//________________________________________________________________

Bool_t KVVAMOSReconNuc::ReconstructFPtrajByFitting()
{
   // Reconstruct the Focal-Plane trajectory and set the appropriate
   // FPCode. If the reconstruction is well carried out (i.e. FPCode>kFPCode0)
   // then the tracking along this trajectory is runned.
   // In this method, the reconstruction of the FP trajectory is done by
   // fitting a first-order polynomial on the positions measured by the
   // detector listed in the the environment variable: KVVAMOSCodes.FocalPlanReconDetList

   UChar_t res = 0;
   Double_t xyzf [3];          // [3] => [X, Y, Z]
   Double_t dxyzf[3];          // [3] => [X, Y, Z]

   Short_t Idx[4] = { -1, -1, -1, -1}; // [4] => [Complete Det1, Complete Det2, Incomplete Det1, Incomplete Det2]
   UChar_t IncDetBitmask = 3;        // bits 11 => 3, first bit means missing X, second bit means missing Y
   Bool_t inc1IsX = kTRUE;           // true if the coordinate given by the first incomplete position is X ( false if Y)
   UShort_t Ncomp = 0, Ninc = 0;     // Number of Complete and Incomplete

   SetFPCode(kFPCode0);   // Initialize FP codes to code 0 "no FP position recon."

   static TGraphErrors graphX(4); // 4 is the max number of FPdetectors
   static TGraphErrors graphY(4); // 4 is the max number of FPdetectors
   Int_t NptX = 0, NptY = 0;     // Number of Points in X-Z and Y-Z plans
   const Char_t* FPdetName = NULL;
   KVVAMOSDetector* d = NULL;
   // Loop over the detector names to be used for the Focal plane Position
   // reconstruction
   for (Short_t i = 0; (FPdetName = GetCodes().GetFPdetName(i)) && i < 4; i++) {

      // Look at only the detectors in 'fDetList' with the name or
      // type 'FPdetName' measuring a position.
      if (
         ((d = (KVVAMOSDetector*)GetDetectorList()->FindObject(FPdetName)) ||
          (d = (KVVAMOSDetector*)GetDetectorList()->FindObjectByType(FPdetName)))
      ) {

         // initializing the result
         res = 0;

         // X position
         if (d->GetNMeasuredX() < 2) { // case where only one X position is measurable
            if ((d->GetPosition(xyzf, 'X') & 5) == 5) {
               d->GetDeltaXYZf(dxyzf, 'X');
               graphX.SetPoint(NptX, xyzf[2], xyzf [0]);
               graphX.SetPointError(NptX,      0, dxyzf[0] > 0.01 ? dxyzf[0] : 0.01);
               NptX++;
               res |= 5;
            }
         }
         else {   // case where several X positions are measurable
            for (Int_t j = 1; j <= d->GetNMeasuredX(); j++) {
               // look at only well measured X position with its Z
               if ((d->GetPosition(xyzf, 'X', j) & 5) == 5) {
                  d->GetDeltaXYZf(dxyzf, 'X', j);
                  graphX.SetPoint(NptX, xyzf[2], xyzf [0]);
                  graphX.SetPointError(NptX,      0, dxyzf[0] > 0.01 ? dxyzf[0] : 0.01);
                  NptX++;
                  res |= 5;
               }
            }
         }

         // Y position
         if (d->GetNMeasuredY() < 2) { // case where only one Y position is measurable
            if ((d->GetPosition(xyzf, 'Y') & 6) == 6) {
               d->GetDeltaXYZf(dxyzf, 'Y');
               graphY.SetPoint(NptY, xyzf[2], xyzf [1]);
               graphY.SetPointError(NptY,      0, dxyzf[1] > 0.01 ? dxyzf[1] : 0.01);
               NptY++;
               res |= 6;
            }
         }
         else {   // case where several Y positions are measurable
            for (Int_t j = 1; j <= d->GetNMeasuredY(); j++) {
               // look at only well measured Y position with its Z
               if ((d->GetPosition(xyzf, 'Y', j) & 6) == 6) {
                  d->GetDeltaXYZf(dxyzf, 'Y', j);
                  graphY.SetPoint(NptY, xyzf[2], xyzf [1]);
                  graphY.SetPointError(NptY,      0, dxyzf[1] > 0.01 ? dxyzf[1] : 0.01);
                  NptY++;
                  res |= 6;
               }
            }
         }


         if (res <= 4) continue;  // look at only case with Z well determined
         else if ((res & 7) == 7) {
            // if X, Y and Z are well measured, bits 111 => 7
            // Complete position (XYZ)

            Idx[ Ncomp ] = i;
            Ncomp++;
         }
         else if (IncDetBitmask & res) {
            // look at only incomplete position measurment (XZ or YZ)
            // if it is not found yet

            Idx[ 2 + Ninc ] = i;
            if (Ninc == 0) inc1IsX = res & 1;
            Ninc++;
            IncDetBitmask = (IncDetBitmask | 4)^res;
         }

         if ((Ncomp == 2) || ((Ncomp == 1) && !IncDetBitmask)) {
            // we break the look if:
            //    - the Focal plane Position is reconstructed from 2 complete position measurment
            // or if:
            //    - the Focal plane Position is reconstructed from 1 complete position measurment and 2 incomplete position measurment.

            graphX.Set(NptX);
            graphY.Set(NptY);

            fRT.dirFP.SetXYZ(0, 0, 1);
            TF1* ff = (TF1*)gROOT->GetFunction("pol1");

            graphX.Fit(ff, "QNC");
            fRT.dirFP.SetX(ff->GetParameter(1));
            fRT.pointFP[0] = ff->GetParameter(0);

            graphY.Fit(ff, "QNC");
            fRT.dirFP.SetY(ff->GetParameter(1));
            fRT.pointFP[1] = ff->GetParameter(0);

            // normalize the direction vector dirFP
            fRT.dirFP *= 1. / fRT.dirFP.Mag();

            GetCodes().SetFPCode(Idx[0], Idx[1], Idx[2], Idx[3], inc1IsX);

            fRT.SetFPparamsReady();

            //Info("ReconstructFPtrajByFitting","\n    Xf= %f, Yf= %f, Thetaf= %f, Phif= %f\n    FPCode%d %s", GetXf(), GetYf(),GetThetaF(),GetPhiF(),GetCodes().GetFPCodeIndex(), GetCodes().GetFPStatus());
            break;
         }
      }
   }
   return !GetCodes().TestFPCode(kFPCode0);
}
//________________________________________________________________

Bool_t KVVAMOSReconNuc::ReconstructLabTraj()
{
   // Reconstruction of the trajectory at the target point, in the reference
   // frame of the laboratory, from the trajectory at the focal plane.
   // The method ReconstructFPtraj() has to be call first.

   // No trajectory reconstruction in the laboratory if the reconstruction
   // in the focal plane is not OK.
   TString filename = gDataSet->GetDataSetEnv("KVVAMOSReconNuc.ReconstructLabTrajMethod");
   if (filename == "") {
      Warning("ReconstructLabTraj()", "No method defined. Should be given by %s.KVVAMOSReconNuc.ReconstructLabTrajMethod", gDataSet->GetName());
      return false;
   }
   else if (filename == "Zgoubi") {
      KVZGOUBIReconstruction* tm = gVamos->GetZGOUBIReconstruction();
      return tm->ReconstructFPtoLab(&fRT);
   }
   else if (filename == "Polynomial") {
      KVVAMOSTransferMatrix* tm = gVamos->GetTransferMatrix();
      return tm->ReconstructFPtoLab(&fRT);
   }
   else {
      Warning("ReconstructLabTraj()", "Method not valid, should be Polynomial or Zgoubi. Should be given by %s.KVVAMOSReconNuc.ReconstructLabTrajMethod. Zgoubi will be used.", gDataSet->GetName());
      KVZGOUBIReconstruction* tm = gVamos->GetZGOUBIReconstruction();
      return tm->ReconstructFPtoLab(&fRT);
   }
}
//________________________________________________________________

void KVVAMOSReconNuc::Propagate(ECalib cal)
{
   // Propagate the nucleus along the reconstructed trajectory tocalculate
   // some quantities from each volume (detectors) punched through at the
   // focal plane.

   // The propagation will be incoherent if the trajectory reconstruction
   // in the focal plane is not OK (i.e. the list of detectors in fDetList
   // could be different of the one in fParameters).
   //
   // The propagation is done by KVVAMOSReconGeoNavigator::PropagateNucleus();

   GetNavigator()->PropagateNucleus(this, cal);

   // if( !CheckTrackingCoherence() ){
   //    Info("Propagate","NO tracking coherence");
   //    GetDetectorList()->ls();
   //    GetParameters()->Print();
   //    cout<<endl;
   // }
}
//________________________________________________________________

void KVVAMOSReconNuc::Print(Option_t* option) const
{
   IGNORE_UNUSED(option);

   KVReconstructedNucleus::Print();
   //cout << " =======> ";
   //cout << " Q= " << GetQ() << ", Brho= " << GetBrho() << " T.m, ThetaV= " << GetThetaV() << " deg, PhiV= " << GetPhiV() << " deg" << endl;
}
//________________________________________________________________

void KVVAMOSReconNuc::SetZIdentification(KVIdentificationResult* idr, KVIDTelescope* idt)
{
   // VAMOS specific Z identification.
   // Call of usual KVReconstructedNucleus::SetIdentification() method
   // then save VAMOS specific observables
   //
   // The mass (A) information in KVIdentificationResult is only used if the mass
   // was measured as part of the identification, or if it was estimated by the
   // CsI minimizer in case of 'IDCode=3'.
   // Otherwise the nucleus' mass formula
   // will be used to calculate A from the measured Z.

   //debug
   if (fdebug) {
      Info("SetZIdentification", "... setting the following IDR for identification ... \nIDR::IDCode=%d \nIDR::IDOK=%d, IDR::IDquality=%d \nIDR::Zident=%d, IDR::Aident=%d \nIDR::PID=%lf, IDR::Z=%d, IDR::A=%d",
           idr->IDcode, (int) idr->IDOK, idr->IDquality, (int) idr->Zident, (int) idr->Aident, idr->PID, idr->Z, idr->A);
      idr->Print();
   }

   //usual ident setters
   SetIdentifyingTelescope(idt);
   SetIdentification(idr);

   //when identification is done by a 'KVIDHarpeeSiCsI_e503'
   //inherited ID telescope, the mass is changed "by hand" to the one
   //found by the minimiser, so IDR->Aident is also changed and the call
   //to SetIdentification(IDR) will not keep the information about the PID
   //of the Z grid.
   //Here we save it by hand...
   if ((idt->InheritsFrom(KVIDHarpeeSiCsI_e503::Class()))) {
      KVIDHarpeeSiCsI_e503* sicsi = static_cast<KVIDHarpeeSiCsI_e503*>(idt);
      assert(sicsi);

      SetRealZ(sicsi->GetBasePID()); //save the PID of Z grid by hand
      SetAMinimizer(idr->PID);       //save the mass found by minimizer by hand
   }

   //when identification is done by a 'KVIDHarpeeICSi_e503' inherited
   //ID telescope, a special IDquality code can be set for the
   //punch-through particules if the user wants to, by drawing a
   //KVIDLine labelled "Punch-through" in the grid.
   //if such a line was defined, then a special IDCode will be set
   //for particles below the line.
   if ((idt->InheritsFrom(KVIDHarpeeICSi_e503::Class())) && (idr->IDquality == KVIDHarpeeICSi_e503::kBelowPunchThrough)) {
      SetIDCode(kIDCode11);
      if (fdebug) Info("SetZIdentification", "... point is below punch-through, changing IDCode ...");
   }
}
//________________________________________________________________

void KVVAMOSReconNuc::SetBasicQandAIdentification(const Char_t* tof_name, Float_t tof, Float_t dist, Int_t nHF, Float_t realAE, Float_t realAoQ)
{
   //Set the basic Q and A identification routine
   //WITHOUT data corrections (so the results from
   //IdentifyQandA()).
   //
   //NOTE: if no corrections applied, these will be
   //the final identification results of the VAMOS
   //particle.

   SetTCode(KVVAMOSCodes::GetTCode(tof_name));
   SetBasicToF(tof);
   SetBasicPath(dist);
   SetBasicRealAE(realAE);
   SetBasicRealAoverQ(realAoQ);
   SetBasicNHF(nHF);

   //Since changing mass is done by leaving momentum unchanged, the kinetic
   //energy is changed too.
   //Keep its value and set it again at the end.
   Double_t E = GetEnergy();
   SetA(GetBasicA());
   SetRealA(GetBasicRealA());
   SetEnergy(E);

   SetIsBasicQandAidentified();
}
//________________________________________________________________
void KVVAMOSReconNuc::SetCorrectedQandAIdentification()
{
   //Set the corrected Q and A identification from
   //KVDataCorrection (see ApplyCorrection() method)
   //as final results.
   //
   //Also set the energy and angles (so momentum) of the nucleus
   //
   //Corrected observables must be set in the KVVAMOSDataCorrection inherited
   //class set to 'fDataCorr' pointer.
   //
   //To do so we can either: 1) use energy losses and identified mass
   //                        2) use velocity from ToF and identified mass (better resolution)


//   //1)-----Set energy from energy losses (deprecated)-----
//   //Since changing mass is done by leaving momentum unchanged, the kinetic
//   //energy is changed too.
//   //Keep its value and set it again at the end.
//    Double_t KE = GetCorrectedEnergy();
//    SetRealA(GetCorrectedRealA());
//    SetA(GetCorrectedA());
//    SetEnergy(KE);
//    SetTheta(GetThetaI());
//    SetPhi(GetPhiI());
//    SetRealQ(GetCorrectedRealQ());
//    SetQ(GetCorrectedQ());

   //2)-----Set energy from velocity (ToF measurement)-----
   //first set mass (so GetMass() returns the expected mass)
   SetAMeasured(kTRUE);
   SetRealA(GetCorrectedRealA());
   SetA(GetCorrectedA());
   SetQMeasured(kTRUE);
   SetRealQ(GetCorrectedRealQ());
   SetQ(GetCorrectedQ());
   //then set momentum from velocity in cartesian coordinates
   Double_t gamma_new = 1. / KVParticle::C() *  GetCorrectedGamma();
   Double_t vx = GetCorrectedVelocity() * TMath::Sin(GetThetaI() * TMath::DegToRad()) * TMath::Cos(GetPhiI() * TMath::DegToRad());
   Double_t vy = GetCorrectedVelocity() * TMath::Sin(GetThetaI() * TMath::DegToRad()) * TMath::Sin(GetPhiI() * TMath::DegToRad());
   Double_t vz = GetCorrectedVelocity() * TMath::Cos(GetThetaI() * TMath::DegToRad());
   TVector3 vvec(vx, vy, vz);
   TVector3 pvec = GetMass() * gamma_new * vvec;
   SetMomentum(pvec);

   //debug
   //Info("SetCorrectedQandAidentification", "Velocity=%lf, KE=%lf, new_Velocity=%lf, newKE=%lf", GetCorrectedVelocity(), GetCorrectedEnergy(), GetVelocity().Mag(), GetEnergy());

   SetIsCorrectedQandAidentified();
}
//________________________________________________________________

void KVVAMOSReconNuc::SetBasicQandAIdentification(KVIdentificationResult* idr)
{
   //Specific to KVIDQA grid results
   //
   // Set Q and A identification of nucleus from informations in identification result object
   // from the KVIDQA GRID.
   // The mass (A) information in KVIdentificationResult is only used if the mass
   // was measured as part of the identification. Otherwise the nucleus' mass formula
   // will be used to calculate A from the measured Z.
   SetTCode(idr->IDcode);
   SetQMeasured(idr->Zident);
   if (!IsAMeasured()) SetAMeasured(idr->Aident);
   SetQ(idr->Z);
   if (idr->Aident) {
      if (idr->Zident) SetIsBasicQandAidentified();
      // since changing mass is done by leaving momentum unchanged, the kinetic
      // energy is changed too.
      // Keep its value and set it again at the end.
      Double_t E = GetEnergy();
      SetA(idr->A);
      SetRealA(idr->PID);
      SetEnergy(E);
   }
   else SetRealQ(idr->PID);
}
//________________________________________________________________

Bool_t KVVAMOSReconNuc::CheckTrackingCoherence()
{
   // Verifies the coherence between the tracking result and the list of
   // fired detectors.
   // If at least one active volum of each fired detector is
   // inside the tracking result (saved in fParameters list). Return kTRUE if this
   // is OK.

   TString str;
   TIter nextdet(GetDetectorList());
   KVVAMOSDetector* det = NULL;
   while ((det = (KVVAMOSDetector*)nextdet())) {

      Bool_t ok = kFALSE;

      TIter next_tr(GetParameters()->GetList());
      TObject* tr = NULL;
      while ((tr = next_tr()) && !ok) {
         str = tr->GetName();
         if (str.BeginsWith(Form("DPATH:%s", det->GetName()))) ok = kTRUE;
      }

      if (!ok) return kFALSE;
   }
   return kTRUE;
}
//________________________________________________________________

Bool_t KVVAMOSReconNuc::CalculateCorrFlightDistanceAndTime(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const
{
   // Returns true if the corrected flight distance (dist) and the corrected time of flight (tof)
   // is correctly calculated.
   // The first calibrated and fired acq. parameter belonging both to
   // the start detector and to the stop detector (only to the start detector
   // for an HF-time) of the list fDetList will be chosen, and the total flight distance
   // will be equal to:
   //  - the path (from target point to focal plan) corrected on the distance
   //    covered between the focal plan and the start detector for HF-time.
   //  - the distance between the two detectors.

   dist = tof = 0.;
   nHF = 0;

   KVEBYEDAT_ACQParam* par = gVamos->GetVACQParam(tof_name);
   if (!par) {
      Error("GetCorrFlightDistanceAndTime", "No time %s is measured in VAMOS", tof_name);
      return kFALSE;
   }

   const Char_t* t_type = tof_name + 1;
   Bool_t isT_HF        = !strcmp("HF", par->GetLabel());
   Bool_t ok            = kFALSE;
   Double_t calibT      = 0;

   TIter next_det(GetDetectorList());
   KVVAMOSDetector* det   = NULL;
   KVVAMOSDetector* start = NULL;
   KVVAMOSDetector* stop  = NULL;
   // look for start and stop detectors
   while ((det = (KVVAMOSDetector*)next_det())) {

      // for HF time we only need the start detector
      if (isT_HF) {
         if (det->IsStartForT(t_type)) {
            if (det->IsStartForT(t_type) && (calibT = det->GetCalibT(t_type)) > 0) {
               start = det;
               ok = kTRUE;
               break;
            }
         }
      }
      // otherwise we need start and stop detectors
      else {
         if (!start && det->IsStartForT(t_type) && (calibT = det->GetCalibT(t_type))) start = det;
         else if (!stop && det->IsStopForT(t_type)) stop = det;
         if (start && stop) {
            ok = kTRUE;
            break;
         }
      }
   }

   if (!ok) {
      //debug
      //Error("GetCorrFlightDistanceAndTime", "detectors used to measure %s are not found in the detector list (fDetlist)\n isT_HF %d, t_type %s, calibT %f", tof_name, isT_HF, t_type, calibT);
      //GetDetectorList()->ls();
      //cout << endl;

      return kFALSE;
   }

   dist = CalculatePath(start, stop);
   if (dist <= 0.) return kFALSE;

   tof  = (isT_HF ? CalculateCorrectedT_HF(calibT, dist, nHF) : calibT);
   if (fdebug) Info("CaluclateCorrFlightDistanceAndTime", "... end of computation, corr_dist=%lf, corr_tof=%lf, nHF=%d ...", dist, tof, nHF);

   return kTRUE;
}
//________________________________________________________________

Double_t KVVAMOSReconNuc::CalculateCorrectedT_HF(Double_t tof, Double_t dist, Int_t& nHF) const
{
   // Returns the corrected time of flight obtained from beam pulse HF, by
   // removing or adding N times the beam pulse period. N is fitted by
   // minimizing the difference between the measured energy (Emeas) and the energy (E) deduced
   // from the velocity. The procedure is as follows:
   //
   //    - let Begin_Latex T_{f} = T_{i} + #DeltaT End_Latex be the corrected time of flight
   //      where Begin_Latex T_{i} End_Latex is the initial time of flight set as an argument;
   //    - Begin_Latex #Delta T End_Latex is deduced by minimizing the quantity
   //      Begin_Latex #chi = (E-E_{meas})^{2} End_Latex
   //      i.e. we resolve the equation
   //      Begin_Latex #frac{d#chi}{d#DeltaT} = 0 End_Latex.
   //      The solution is
   //      Begin_Latex #DeltaT = #frac{D}{c#sqrt{1-#gamma^{2}}} - T_{i} End_Latex where Begin_Latex #gamma = #frac{1}{#frac{E_{cal}}{mc^{2}}+1} End_Latex,
   //      D is the flight distance, m is the mass returned by GetMass() and c is the speed of light in the vacuum;
   //    - N if given by the rounded value of  Begin_Latex #DeltaT/period_{HF} End_Latex
   //
   //    This procedure can be applied only if the nucleus is identified and calibrated (i.e. IsCalibrated = true ).
   //    Otherwise this method returns the initial tof.
   //
   // Arguments :
   //         tof  - initial time of flight in ns
   //         dist - flight distance in cm

   if (!IsCalibrated() || (GetEnergy() <= 0)) return tof;
   if (gVamos->GetBeamPeriod() < 0) {
      Warning("CalculateCorrectedT_HF", "No correction because the beam pulse period is unknown");
      return tof;
   }

   nHF = CalculateNBeamPeriod(tof, dist);

   Double_t corr_tof = tof + nHF * gVamos->GetBeamPeriod();
   if (fdebug) Info("CalculateCorrectedT_HF", "... tof=%lf, corr_tof=%lf, nHF=%d...", tof, corr_tof, nHF);

   return corr_tof;
}
//________________________________________________________________
Int_t  KVVAMOSReconNuc::CalculateNBeamPeriod(Double_t tof, Double_t dist) const
{
   //Returns the number of time we add/remove the beam pulse period to
   //the time of flight obtained from beam pulse HF

   //Double_t alpha = 1. / (GetEnergy() / GetMass() + 1.);
   Double_t alpha = (GetEnergy() / GetMass() + 1.);
   Double_t delta_t = (dist / (C() * TMath::Sqrt(1. - alpha * alpha))) - tof;
   Int_t nn = TMath::Nint(delta_t / gVamos->GetBeamPeriod());

   if (fdebug) Info("CalculateNBeamPeriod", "... alpha=%lf, delta_t=%lf, n=%d ...", alpha, delta_t, nn);

   return nn;
}

//________________________________________________________________
Double_t KVVAMOSReconNuc::CalculatePath(KVVAMOSDetector* start, KVVAMOSDetector* stop) const
{
   // Returns the flight distance travelled by the nucleus from the start detector to the stop detector.
   // If stop=NULL, returns the distance from the target point to the start detector,
   // i.e. distance corresponding to a time of flight measured from the beam HF then the distance will be
   // equal to the reconstructed path (GetPath) plus (or minus) the distance between
   // the trajectory position at the focal plane (FP) and the trajectory position
   // at the start detector if this detector is localised behinds the FP (or
   // forwards the FP).
   //
   // Returns zero if the focal plan trajectory is not well reconstructed (i.e. kFPCode0 ),
   // if DeltaPath is not found for the detectors or if the reconstructed path is negative.

   if (const_cast<KVVAMOSReconNuc*>(this)->GetCodes().TestFPCode(kFPCode0)) return 0.;

   Double_t dp_start = FindDeltaPath(start);
   if (dp_start) {
      //------case where stop signal is given by detector i.e. 'stop' not null------
      if (stop) {
         Double_t dp_stop = FindDeltaPath(stop);

         if (dp_stop) {
            return TMath::Abs(dp_stop - dp_start);
         }

         else {
            if (fdebug) Warning("GetPath", "... <stop detector> delta_path=%lf, returning 0 ...", dp_stop);
            return 0.;
         }

      }

      //------case where stop signal is given by HF i.e. 'stop' is null------
      else if (GetPath() > 0) {
         //for debugging
         if (fdebug) {
            Info("GetPath", "HF signal used for ToF measurement \n Path BEFORE correction=%lf \n Path AFTER correction=%lf",
                 GetPath(), GetPath() + dp_start);
         }

         return GetPath() + dp_start;
      }
   }
   return 0.;
}
//________________________________________________________________

Double_t KVVAMOSReconNuc::CalculatePath(const Char_t* start_label, const Char_t* stop_label) const
{
   // Returns the flight distance travelled by the nucleus from the start detector to the stop detector.
   // If stop=NULL, returns the distance from the target point to the start detector,
   // i.e. distance corresponding to a time of flight  measured from the beam HF then the distance will be
   // equal to the reconstructed path (GetPath) plus (or minus) the distance between
   // the trajectory position at the focal plane (FP) and the trajectory position
   // at the start detector if this detector is localised behinds the FP (or
   // forwards the FP).
   //
   // Returns zero if DeltaPath is not found for the detectors.
   //
   // Inputs - labels of start and stop detectors

   KVVAMOSDetector* start = (KVVAMOSDetector*)GetDetectorList()->FindObjectByLabel(start_label);

   if (!strcmp(stop_label, "")) return CalculatePath(start);
   KVVAMOSDetector* stop = (KVVAMOSDetector*)GetDetectorList()->FindObjectByLabel(stop_label);
   return CalculatePath(start, stop);
}
//________________________________________________________________

/*
Double_t KVVAMOSReconNuc::FindDeltaPath(KVVAMOSDetector* det) const
{
   //returns the DeltaPath value associated to the detector 'det' used to correct
   //the flight distance.
   //Its value is given by a parameter stored in fParameters with the name
   //DPATH:<det_name>. If this parameter is not found then we take the parameter
   //with the name begining by DPATH:<det_time_base_name>. Otherwise zero is
   //returned with a warning message.
   //
   // This method has to be called once the tracking has been runned since
   // the DPATH parameter is calculated for each detector crossed by the nucleus
   // at this step (see Propagate).

   if (!det) return 0.;
   // Find the parameter with the name DPATH:<detector_name>

   KVNamedParameter* par = GetParameters()->FindParameter(Form("DPATH:%s", det->GetName()));
   if (par) return par->GetDouble();

   // Find the parameter with the name begining by DPATH:<detector_time_base_name>
   TString tmp;
   TIter next(GetParameters()->GetList());
   while ((par = (KVNamedParameter*)next())) {
      tmp = par->GetName();
      if (tmp.BeginsWith(Form("DPATH:%s", det->GetTBaseName()))) {
         //debug
         if (fdebug) Info("FindDeltaPath", "DeltaPath for the detector %s is given by %s", det->GetName(), par->GetName());
         return par->GetDouble();
      }
   }

   // No parameter found
   Warning("FindDeltaPath", "DeltaPath not found for the detector %s", det->GetName());

   return 0.;
}
*/

Double_t KVVAMOSReconNuc::FindDeltaPath(KVVAMOSDetector* det) const
{
   // Method used to find the deltapath by bypassing the ROOT geometry. Problem in ROOT geometry, see commented version of KVVAMOSReconNuc::FindDeltaPath(KVVAMOSDetector* det)
   Double_t deltapathtemp = 0;
   if (!det) return 0.;
   if (gVamos->GetToFlist_deltapath().HasParameter(det->GetName())) {
      deltapathtemp = gVamos->GetToFlist_deltapath().GetDoubleValue(det->GetName()) * TMath::Sqrt(1. + TMath::Power(TMath::Tan(GetThetaF() * TMath::DegToRad()), 2) + TMath::Power(TMath::Tan(GetPhiF() * TMath::DegToRad()), 2));
      return deltapathtemp;
   }
   return 0;
}
//________________________________________________________________

Double_t KVVAMOSReconNuc::GetEnergy(const Char_t* det_label) const
{
   // Returns the calculated contribution of each detector to the
   // nucleus' energy from their label ("CHI","SI","SED1","SED2",...).
   // Retruns -1 if no detector is found or if yet no contribution has
   // been determined ( done by methods Calibrate or InverseCalibration ).

   if (fDetE.empty()) return -1.;
   Int_t idx = GetDetectorIndex(det_label);
   return idx < 0 ? -1. : fDetE[idx];
}
//________________________________________________________________

Double_t KVVAMOSReconNuc::GetEnergyBefore(const Char_t* det_label) const
{
   // Returns the kinetic energy of the nucleus prior to entering in
   // detector with label 'det_label' ("CHI","SI","SED1","SED2",...).
   // Retruns -1 if no detector is found or if yet no contribution has
   // been determined ( done by methods Calibrate or InverseCalibration ).

   if (fDetE.empty()) return -1.;
   Int_t idx = GetDetectorIndex(det_label);
   if (idx < 0) return -1.;
   Double_t E = 0.;
   while (idx > -1) E += fDetE[idx--];
   return E;
}
//________________________________________________________________

Double_t KVVAMOSReconNuc::GetEnergyAfter(const Char_t* det_label) const
{
   // Returns the kinetic energy of the nucleus prior to entering in
   // detector with label 'det_label' ("CHI","SI","SED1","SED2",...).
   // Retruns -1 if no detector is found or if yet no contribution has
   // been calculated ( done by methods Calibrate or InverseCalibration ).

   if (fDetE.empty()) return -1.;
   Int_t idx = GetDetectorIndex(det_label);
   if (idx < 0) return -1.;
   idx--;
   Double_t E = 0.;
   while (idx > -1) E += fDetE[idx--];
   return E;
}
//________________________________________________________________

Int_t KVVAMOSReconNuc::GetDetectorIndex(const Char_t* det_label) const
{
   // Returns the index of the detector in the list of detectors (fDetList)
   // through which particle passed, from its label ("CHI","SI","SED1","SED2",...).
   // Returns -1 if no detector found.

   TIter next(GetDetectorList());
   KVBase* obj;
   Int_t idx = 0;
   while ((obj = (KVBase*)next())) {
      if (!strcmp(obj->GetLabel(), det_label)) break;
      idx++;
   }
   return  idx < GetDetectorList()->GetEntries() ? idx : -1;
}
