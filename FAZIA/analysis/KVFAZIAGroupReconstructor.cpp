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
   // Perform energy calibration of (previously identified) charged particle
   //
   // "Gamma" particles identified in CsI are given the equivalent energy of proton,
   // if the CsI detector is calibrated

   KVFAZIADetector* det = (KVFAZIADetector*)PART->GetStoppingDetector();

   if (det->IsType("CsI") && PART->GetIDCode() == 0) { // gammas
      if (det->IsCalibrated()) {
         double Ep = det->GetDetectorSignalValue("Energy", "Z=1,A=1");
         PART->SetEnergy(Ep);
         PART->SetIsCalibrated();
         PART->SetECode(1);
         PART->SetParameter("FAZIA.ECSI", Ep);
      }
   }

   if (PART->GetIDCode() == 5) { // stopped in SI1, no PSA identification
      if (det->IsCalibrated()) {
         double Ep = det->GetEnergy();
         PART->SetEnergy(Ep);
         PART->SetIsCalibrated();
         PART->SetECode(1);
         PART->SetParameter("FAZIA.ESI1", Ep);
      }
   }

   // particle identified in Si1 PSA, detector is calibrated
   if (PART->GetIDCode() == 11) {
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetIdentifyingTelescope()->GetDetector(1);
      if (si1->IsCalibrated()) {
         double e1 = si1->GetCorrectedEnergy(PART, -1., kFALSE);
         if (e1 <= 0) {
            Warning("CalibrateParticle",
                    "IDCODE=11 Z=%d A=%d calibrated SI1 E=%f",
                    PART->GetZ(), PART->GetA(), e1);
            return;
         }
         PART->SetParameter("FAZIA.ESI1", e1);
         PART->SetEnergy(e1);
         PART->SetIsCalibrated();
         PART->SetECode(1);
      }
   }

   // particle identified in Si1-Si2
   if (PART->GetIDCode() == 12) {
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetIdentifyingTelescope()->GetDetector(1);
      KVFAZIADetector* si2 = (KVFAZIADetector*)PART->GetIdentifyingTelescope()->GetDetector(2);
      if (si1->IsCalibrated() && si2->IsCalibrated()) { //  with both detectors calibrated
         double e1(-1), e2(-1);
         bool calc_e1(false), calc_e2(false);
         if ((e2 = si2->GetEnergy()) > 0) {
            if (!((e1 = si1->GetEnergy()) > 0)) { // if SI1 not fired, we calculate from SI2 energy
               e1 = si1->GetDeltaEFromERes(PART->GetZ(), PART->GetA(), e2);
               calc_e1 = true;
            }
         }
         else if ((e1 = si1->GetEnergy()) > 0) {
            // if SI2 not fired, we calculate from SI1 energy
            e2 = si1->GetEResFromDeltaE(PART->GetZ(), PART->GetA());
            calc_e2 = true;
         }

         PART->SetParameter("FAZIA.ESI1", (calc_e1 ? -e1 : e1));
         PART->SetParameter("FAZIA.ESI2", (calc_e2 ? -e2 : e2));
         if (e1 > 0 && e2 > 0) {
            PART->SetEnergy(e1 + e2);
            PART->SetIsCalibrated();
            PART->SetECode((calc_e1 || calc_e2) ? 2 : 1);
         }
      }
   }

   // particle identified in Si2-CsI or CsI
   if (PART->GetIDCode() == 23 || PART->GetIDCode() == 33) {
      KVFAZIADetector* si2 = (KVFAZIADetector*)PART->GetReconstructionTrajectory()->GetDetector("SI2");
      KVFAZIADetector* csi = (KVFAZIADetector*)PART->GetStoppingDetector();
      KVFAZIADetector* si1 = (KVFAZIADetector*)PART->GetReconstructionTrajectory()->GetDetector("SI1");

      if (!(si1 && si2 && csi)) {
         Error("CalibrateParticle",
               "IDCODE=23 si1=%s si2=%s csi=%s",
               (si1 ? si1->GetName() : "?"), (si2 ? si2->GetName() : "?"), (csi ? csi->GetName() : "?"));
      }
      // treat case of uncalibrated CsI detector
      if (!csi->IsCalibrated()) {
         // case where SI1 && SI2 are calibrated & present in event
         // (STRICTLY SPEAKING, FIRST NEED TO CHECK THAT NOTHING ELSE STOPPED IN SI1 (for Si2-CsI id) or SI2 (for CsI id)):
         // THIS SHOULD BE DONE IN IDENTIFICATION COHERENCY CHECKS
         if (si1->IsCalibrated() && si1->GetEnergy() && si2->IsCalibrated() && si2->GetEnergy()) {
            // calculate total delta-E in (SI1+SI2) then use to calculate CsI energy
            double deltaE = si1->GetEnergy() + si2->GetEnergy();
            KVDetector si1si2("Si", si1->GetThickness() + si2->GetThickness());
            double ecsi = si1si2.GetEResFromDeltaE(PART->GetZ(), PART->GetA(), deltaE);
            PART->SetParameter("FAZIA.ESI1", si1->GetEnergy());
            PART->SetParameter("FAZIA.ESI2", si2->GetEnergy());
            PART->SetParameter("FAZIA.ECSI", -ecsi);
            PART->SetEnergy(deltaE + ecsi);
            PART->SetIsCalibrated();
            PART->SetECode(2); // CsI energy calculated
            //PART->ls();
         }
      }
   }

   if (PART->IsCalibrated()) {
      // check for energy loss coherency
      KVNucleus avatar;
      avatar.SetZAandE(PART->GetZ(), PART->GetA(), PART->GetKE());

      Double_t chi2 = 0.;
      int ndet = 0;
      const char* detnames[] = {"SI1", "SI2", "CSI"};
      KVGeoDetectorNode* node = 0;
      // iterating over detectors starting from the target
      // compute the theoretical energy loss of the avatar
      // compare to the calibrated/calculated energy
      // remove this energy from the avatar energy
      PART->GetReconstructionTrajectory()->IterateBackFrom();
      while ((node = PART->GetReconstructionTrajectory()->GetNextNode())) {
         det = (KVFAZIADetector*)node->GetDetector();
         Double_t temp = det->GetELostByParticle(&avatar);
         PART->SetParameter(Form("FAZIA.avatar.E%s", detnames[det->GetIdentifier()]), temp);
         chi2 += ((det->GetEnergyLoss() - temp) / det->GetEnergyLoss()) * ((det->GetEnergyLoss() - temp) / det->GetEnergyLoss());
         avatar.SetKE(avatar.GetKE() - temp);
         ndet++;
      }
      chi2 /= ndet;

      // stores avatar energy loss and chi2 in particle parameter list for further checks
      PART->SetParameter("FAZIA.avatar.chi2", chi2);

      // in case the avatar still has energy (+- epsilon ?) we consider it as punch through particle
      // if chi2>10, the calibration is incoherent with calculated energy losses (why 10 ??? to be checked)
      // question : do we really need these two distinct cases ?
//      if (avatar.GetKE() > 0 || chi2 > 10.) PART->SetECode(4);

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
      //PART->ls();
   }
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
   KVGroupReconstructor::IdentifyParticle(PART);

   // check for failed PSA identification for particles stopped in Si1
   // change status => KVReconstructedNucleus::kStatusStopFirstStage
   // estimation of minimum Z from energy loss (if Si1 calibrated)
   // IDCODE = 5
   if (PART.GetStoppingDetector()->IsLabelled("SI1") && !PART.IsIdentified()) {
      PART.SetStatus(KVReconstructedNucleus::kStatusStopFirstStage);
      TreatStatusStopFirstStage(PART);
      PART.SetIDCode(5);
      return;
   }

   // Coherency checks for identifications
   if (PART.IsIdentified()) {
      if (partID.IsType("CsI")) {
         if (partID.IDcode == 0) { // gammas
            // look at Si1-Si2 identification.
            // if a correct identification was obtained, we ignore the gamma in CsI and change to Si1-Si2
#ifndef WITH_CPP11
            std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
#else
            auto si1si2 = id_by_type.find("Si-Si");
#endif
            if (si1si2 != id_by_type.end()) {
               if (si1si2->second->IDOK && si1si2->second->IDquality < KVIDZAGrid::kICODE4) {
                  //               Info("IdentifyParticle","Got GAMMA in CsI, changing to Si1Si2 identification [Z=%d A=%d]",
                  //                    si1si2->second->Z, si1si2->second->A);
                  ChangeReconstructedTrajectory(PART);
                  partID = *(si1si2->second);
                  identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
                  PART.SetIdentifyingTelescope(identifying_telescope);
                  PART.SetIdentification(&partID, identifying_telescope);
                  //PART.Print();
               }
            }
         }
         else {
            // As a general rule, we prefer Si-CsI identification to CsI identification, if both
            // are available and both were successful
            // THIS SHOULD ONLY BE DONE IF WE ARE SURE THAT A 2ND PARTICLE DID NOT STOP IN SI2
            // I.E. AFTER CHECKING THAT CSI & SI-CSI IDENTIFICATIONS ARE COHERENT
            // BUT IF THIS IS THE CASE, WHY CHANGE ?
#ifndef WITH_CPP11
            std::map<std::string, KVIdentificationResult*>::iterator si2csi = id_by_type.find("Si-CsI");
#else
            auto si2csi = id_by_type.find("Si-CsI");
#endif
            if (si2csi != id_by_type.end()) {
               if (si2csi->second->IDOK && si2csi->second->IDquality < KVIDZAGrid::kICODE4) {

                  // To be coherent, Si-CsI identification should not give
                  //    1) same Z but larger A
                  //    2) larger Z
                  //Info("IdentifyParticle","Prefer SiCsI identification over CSI [Z=%d A=%d]",PART.GetZ(),PART.GetA());
                  partID = *(si2csi->second);
                  identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-CsI");
                  PART.SetIdentifyingTelescope(identifying_telescope);
                  PART.SetIdentification(&partID, identifying_telescope);
                  //PART.Print();
               }
            }
         }
      }
      else if (partID.IsType("Si-CsI")) {
         int zz = partID.Z;
#ifndef WITH_CPP11
         std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
#else
         auto si1si2 = id_by_type.find("Si-Si");
#endif
         if (si1si2 != id_by_type.end()) {
            if (si1si2->second->IDOK && si1si2->second->IDquality < KVIDZAGrid::kICODE4 && zz < si1si2->second->Z) {
               //            Info("IdentifyParticle","SiCsI identification [Z=%d A=%d] changed to SiSi identification [Z=%d A=%d]",
               //                 PART.GetZ(),PART.GetA(),si1si2->second->Z,si1si2->second->A);
               ChangeReconstructedTrajectory(PART);
               partID = *(si1si2->second);
               identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("Si-Si");
               PART.SetIdentifyingTelescope(identifying_telescope);
               PART.SetIdentification(&partID, identifying_telescope);
               //PART.Print();
            }
         }
      }
   }
}

void KVFAZIAGroupReconstructor::ChangeReconstructedTrajectory(KVReconstructedNucleus& PART)
{
   // change recon trajectory (modifies stopping detector)

   PART.GetReconstructionTrajectory()->IterateFrom();
   PART.GetReconstructionTrajectory()->GetNextNode();
   KVGeoDetectorNode* node = PART.GetReconstructionTrajectory()->GetNextNode();

   KVGroup* group = PART.GetGroup();
   KVGeoDNTrajectory* traj = (KVGeoDNTrajectory*)node->GetTrajectories()->First();

   const KVReconNucTrajectory* newtraj = (const KVReconNucTrajectory*)group->GetTrajectoryForReconstruction(traj, node);
   PART.ModifyReconstructionTrajectory(newtraj);
}
