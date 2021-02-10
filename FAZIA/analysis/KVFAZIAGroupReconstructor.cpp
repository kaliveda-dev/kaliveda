#include "KVFAZIAGroupReconstructor.h"

#include <KVFAZIA.h>
#include <KVFAZIADetector.h>
#include <KVSignal.h>
#include <KVLightEnergyCsIFull.h>
#include <KVLightEnergyCsI.h>
#include <KVCalibrator.h>
#include <KVIDGCsI.h>
#include <KVCalibratedSignal.h>

ClassImp(KVFAZIAGroupReconstructor)

//////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAGroupReconstructor</h2>
<h4>Reconstruct particles in FAZIA telescopes</h4>
<!-- */
// --> END_HTML
//////////////////////////////////////////////////////////////////////////////////
void KVFAZIAGroupReconstructor::CalibrateParticle(KVReconstructedNucleus* PART)
{
   // Perform energy calibration of (previously identified) particle

   // check that the particle crossed at least one detector layer
   if (PART->GetReconstructionTrajectory()->GetN() < 1) return;

   Int_t ndet       = 0;
   Int_t ndet_calib = 0;
   Double_t ee      = 0.;
   Double_t etot    = 0.;

   Bool_t punch_through = kFALSE;
   Bool_t incoherency = kFALSE;
   Bool_t calculated = kFALSE;

   KVFAZIADetector* det = 0;
   PART->SetEnergy(0.);
   PART->SetECode(0);

   // loop over crossed detectors
   // compute energy if the detector is calibrated
   PART->GetReconstructionTrajectory()->IterateFrom();
   KVGeoDetectorNode* node;
   KVNameValueList part_id(Form("Z=%d,A=%d", PART->GetZ(), PART->GetA()));
   while ((node = PART->GetReconstructionTrajectory()->GetNextNode())) {
      det = (KVFAZIADetector*)node->GetDetector();
      if (det->IsCalibrated()) {
         ee = det->GetDetectorSignalValue("Energy", part_id);
         det->SetEnergyLoss(ee);
         etot += ee;
         ndet_calib++;
      }
      ndet++;
   }

   // loop again over crossed detectors
   // calculate the energy if not calibrated:
   // - first try using detectors behind to calculated DeltaE from Eres,
   // all detector behind have to be calibrated
   // - then try with the detector in front to calculate Eres from DeltaE (less accurate)
   Double_t ebehind = 0.;
   if (ndet != ndet_calib) {
      PART->GetReconstructionTrajectory()->IterateFrom();
      while ((node = PART->GetReconstructionTrajectory()->GetNextNode())) {
         det = (KVFAZIADetector*)node->GetDetector();
         // store energy loss behind for latter DeltaE from Eres calculations
         if (det->IsCalibrated()) ebehind += det->GetEnergyLoss();
         else {
            if (ebehind > 0.) {
               // calculate energy loss from residual energy
               ee = det->GetDeltaEFromERes(PART->GetZ(), PART->GetA(), ebehind);
               det->SetEnergyLoss(ee);
               etot += ee;
               ndet_calib++;
               calculated = kTRUE;
               ebehind += ee;
            }
            else if (det->GetNode()->GetNDetsInFront()) {
               KVFAZIADetector* det_infront = (KVFAZIADetector*)det->GetNode()->GetDetectorsInFront()->First();
               if (det_infront && det_infront->GetEnergyLoss()) {
                  // calculate energy loss from Delta E and check this energy is lower than
                  // punch through energy since it doesn't work if the particle crossed det
                  // (trying to calculate Si2 energy in case only Si1 calibrated)
                  ee = det_infront->GetEResFromDeltaE(PART->GetZ(), PART->GetA(), det_infront->GetEnergyLoss());
                  if (ee > det->GetPunchThroughEnergy(PART->GetZ(), PART->GetA())) continue;
                  det->SetEnergyLoss(ee);
                  etot += ee;
                  ndet_calib++;
                  calculated = kTRUE;
                  ebehind += ee;
               }
            }
         }
      }
   }

   // In case at least one detector has still no calibrated or calculated energy
   // the particle is considered as not calibrated
   if (ndet != ndet_calib) return;
   PART->SetEnergy(etot);

   //   // Now check that energy losses are coherent with particle identification and total energy
   //   // Create a dummy particle with same total energy
   KVNucleus avatar;
   avatar.SetZAandE(PART->GetZ(), PART->GetA(), PART->GetKE());

   Double_t chi2 = 0.;

   // iterating over detectors starting from the target
   // compute the theoretical energy loss of the avatar
   // compare to the calibrated/calculated energy
   // remove this energy from the avatar energy
   PART->GetReconstructionTrajectory()->IterateBackFrom();
   while ((node = PART->GetReconstructionTrajectory()->GetNextNode())) {
      det = (KVFAZIADetector*)node->GetDetector();
      Double_t temp = det->GetELostByParticle(&avatar);
      chi2 += ((det->GetEnergyLoss() - temp) / det->GetEnergyLoss()) * ((det->GetEnergyLoss() - temp) / det->GetEnergyLoss());
      avatar.SetKE(avatar.GetKE() - temp);
   }
   chi2 /= ndet;

   // in case the avatar still has energy (+- epsilon ?) we consider it as punch through particle
   // if chi2>10, the calibration is incoherent with calculated energy losses (why 10 ??? to be checked)
   // question : do we really need these two distinct cases ?
   if (avatar.GetKE() > 0) incoherency = kTRUE;
   else if (chi2 > 10.)    incoherency = kTRUE;// never set

   // checking for CsI punch through
   det = (KVFAZIADetector*)PART->GetStoppingDetector();
   if (det->GetIdentifier() == KVFAZIADetector::kCSI) {

      KVDetector* SI2 = PART->GetDetector(1);
      KVDetector* SI1 = PART->GetDetector(2);

      int Z = PART->GetZ();
      int A = PART->GetA();

      KVDetector SI1SI2("Si", SI1->GetThickness() + SI2->GetThickness());
      if (SI1->GetEnergyLoss() + SI2->GetEnergyLoss() < SI1SI2.GetDeltaEFromERes(Z, A, det->GetPunchThroughEnergy(Z, A))) punch_through = kTRUE;
   }


   // add energy loss in the target
   Double_t E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(PART);
   PART->SetTargetEnergyLoss(E_targ);
   PART->SetEnergy(PART->GetEnergy() + E_targ);

   // set particle momentum from telescope dimensions (random)
   PART->GetAnglesFromReconstructionTrajectory();

   // set calibration code (set to 0:not_calibrated at the beginning)
   if (punch_through)    PART->SetECode(3);
   else if (incoherency) PART->SetECode(4); // never set for now...
   else if (calculated)  PART->SetECode(2);
   else                  PART->SetECode(1);

   PART->SetIsCalibrated();
}

void KVFAZIAGroupReconstructor::PostReconstructionProcessing()
{
   // Copy FPGA energy values to reconstructed particle parameter lists
   // Set values in detectors for identification/calibration procedures

   for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
      KVReconstructedNucleus* rnuc = it.get_pointer<KVReconstructedNucleus>();

      rnuc->GetReconstructionTrajectory()->IterateFrom();

      KVGeoDetectorNode* node;
      while ((node = rnuc->GetReconstructionTrajectory()->GetNextNode())) {

         KVFAZIADetector* det = (KVFAZIADetector*)node->GetDetector();

         TIter next_s(det->GetListOfSignals());
         KVSignal* sig;
         while ((sig = (KVSignal*)next_s())) {

            if (!sig->PSAHasBeenComputed()) {
               sig->TreateSignal();
            }

            sig->GetPSAResult(det);
         }

         // now copy all detector signals to reconstructed particle paramet// never seter list...
         // they are stored with format "[detname].[signal_name]" except for
         // DetTag and GTTag which are the same for all detectors of the same telescope
         // and so are only stored once with name "DetTag" or "GTTag".
         TIter it(&det->GetListOfDetectorSignals());
         KVDetectorSignal* ds;
         while ((ds = (KVDetectorSignal*)it())) {
            if (ds->IsRaw() && !ds->IsExpression())
               // only store raw data, excluding any expressions based only on raw data
            {
               TString pname;
               // Only store non-zero parameters
               if (ds->GetValue() != 0) {
                  if (TString(ds->GetName()) != "DetTag" && TString(ds->GetName()) != "GTTag")
                     pname = Form("%s.%s", det->GetName(), ds->GetName());
                  else
                     pname = ds->GetName();
                  rnuc->GetParameters()->SetValue(pname, ds->GetValue());
               }
            }
         }
      }
   }
}

void KVFAZIAGroupReconstructor::IdentifyParticle(KVReconstructedNucleus& PART)
{


   // first idea :
   // try all possible identification, starting from the stopping ID tel
   // do not stop when a good one is found: the coherency between different
   // ID telescope could then be used latter.
   // The procedure is based on KVGroupReconstructor::IdentifyParticle(PART);
   // with some modifications.

   KVGroupReconstructor::IdentifyParticle(PART);


   // 8He dans CsI-PSA -> 8Be pour E789

   if (partID.IsType("CsI")) {
      if (partID.IDquality == KVIDGCsI::kICODE10) {
         // look at Si1-Si2 identification
         KVIdentificationResult* si1si2 = PART.GetIdentificationResult("Si-Si");
//         std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
         if (si1si2) {
            if (si1si2->IDattempted && si1si2->IDquality < KVIDZAGrid::kICODE4) {
               ChangeReconstructedTrajectory(PART, "SI2");
               partID = *(si1si2);
               identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
               PART.SetIdentifyingTelescope(identifying_telescope);
               PART.SetIdentification(&partID, identifying_telescope);
            }
         }
      }
      else {
         KVIdentificationResult* si2csi = PART.GetIdentificationResult("Si-CsI");
//         std::map<std::string, KVIdentificationResult*>::iterator si2csi = id_by_type.find("Si-CsI");
         if (si2csi) {
            if (si2csi->IDattempted && si2csi->IDquality < KVIDZAGrid::kICODE4) {
               partID = *(si2csi);
               identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-CsI");
               PART.SetIdentifyingTelescope(identifying_telescope);
               PART.SetIdentification(&partID, identifying_telescope);
            }
         }
      }
   }
   else if (partID.IsType("Si-CsI")) {
      int zz = partID.Z;
      KVIdentificationResult* si1si2 = PART.GetIdentificationResult("Si-Si");
//      std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
      if (si1si2) {
         if (si1si2->IDattempted && si1si2->IDquality < KVIDZAGrid::kICODE4 && zz < si1si2->Z) {
            ChangeReconstructedTrajectory(PART, "SI2");
            partID = *(si1si2);
            identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
            PART.SetIdentifyingTelescope(identifying_telescope);
            PART.SetIdentification(&partID, identifying_telescope);
         }
      }
   }
}

void KVFAZIAGroupReconstructor::ChangeReconstructedTrajectory(KVReconstructedNucleus& PART, const char* det)
{
   // set detector channel to zero (peut-etre)
   // change recon trajectory
   // modify stopping detector

   PART.GetReconstructionTrajectory()->IterateFrom();
   PART.GetReconstructionTrajectory()->GetNextNode();
   KVGeoDetectorNode* node = PART.GetReconstructionTrajectory()->GetNextNode();

   KVGroup* group = PART.GetGroup();
   KVGeoDNTrajectory* traj = (KVGeoDNTrajectory*)node->GetTrajectories()->First();

   const KVReconNucTrajectory* newtraj = (const KVReconNucTrajectory*)group->GetTrajectoryForReconstruction(traj, node);
   PART.ModifyReconstructionTrajectory(newtraj);
}
