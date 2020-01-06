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
//   // Perform energy calibration of (previously identified) particle


   KVNucleus avatar;
   //printf("start Calibrate\n");
   Int_t ntot = PART->GetReconstructionTrajectory()->GetN();
   if (ntot < 1)
      return;
   Bool_t punch_through = kFALSE;
   Bool_t incoherency = kFALSE;
   Bool_t pileup = kFALSE;
   Bool_t check_error = kFALSE;

//   double error_si1(0), error_si2(0); // error_csi=0;
   std::vector<Double_t> eloss(ntot);
   for (Int_t ii = 0; ii < ntot; ii += 1) eloss[ii] = 0;

   KVFAZIADetector* det = 0;
   Int_t ndet = 0;
   Int_t ndet_calib = 0;
   Double_t etot = 0;
   Double_t fESI1, fESI2, fECSI;
   fESI1 = fESI2 = fECSI = 0;

   PART->GetReconstructionTrajectory()->IterateFrom();
   KVGeoDetectorNode* node;
   KVNameValueList part_id(Form("Z=%d,A=%d", PART->GetZ(), PART->GetA()));
   while ((node = PART->GetReconstructionTrajectory()->GetNextNode())) {
      det = (KVFAZIADetector*)node->GetDetector();

      if (det->IsCalibrated()) {
         eloss[ntot - ndet - 1] = det->GetDetectorSignalValue("Energy", part_id);
         if (det->GetIdentifier() == KVFAZIADetector::kSI1) {
            fESI1 = eloss[ntot - ndet - 1];
         }
         else if (det->GetIdentifier() == KVFAZIADetector::kSI2) {
            fESI2 = eloss[ntot - ndet - 1];
         }
         else if (det->GetIdentifier() == KVFAZIADetector::kCSI) {
            fECSI = eloss[ntot - ndet - 1];
         }
         // set calibrated energy loss in detector
         det->SetEnergyLoss(eloss[ntot - ndet - 1]);
         etot += eloss[ntot - ndet - 1];
         ndet_calib += 1;
      }
      ndet += 1;
   }
   if (ndet == ndet_calib) {
      Double_t E_targ = 0;
      PART->SetEnergy(etot);

      if (PART->IsAMeasured()) {
         Double_t etot_avatar = 0;
         Double_t chi2 = 0;
         avatar.SetZAandE(PART->GetZ(), PART->GetA(), PART->GetKE());
         for (Int_t nn = ntot - 1; nn >= 0; nn -= 1) {
            det = (KVFAZIADetector*)PART->GetDetector(nn);
            Double_t temp = det->GetELostByParticle(&avatar);
            etot_avatar += temp;
            chi2 += TMath::Power((eloss[ntot - 1 - nn] - temp) / eloss[ntot - 1 - nn], 2.);
            avatar.SetKE(avatar.GetKE() - temp);
         }

         chi2 /= ndet;
         if (avatar.GetKE() > 0) {
            //Warning("Calibrate", "Incoherence energie residuelle %lf (PUNCH THROUGH) %s", avatar.GetKE(),GetStoppingDetector()->GetName());
            punch_through = kTRUE;
            //         } else if (TMath::Abs(etot - etot_avatar) > 1e-3) {
         }
         else if (chi2 > 10.) {
            //Warning("Calibrate", "Incoherence %lf != %lf", etot, etot_avatar);
            incoherency = kTRUE;
         }
      }

      if (PART->GetZ() && PART->GetEnergy() > 0) {
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(PART);
         PART->SetTargetEnergyLoss(E_targ);
      }

      Double_t E_tot = PART->GetEnergy() + E_targ;
      PART->SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      PART->GetAnglesFromStoppingDetector();
      PART->SetECode(0);
      if (punch_through)   PART->SetECode(2);
      if (incoherency)     PART->SetECode(3);
      if (check_error)     PART->SetECode(5); //
      if (pileup)          PART->SetECode(4); //

      PART->SetIsCalibrated();
   }
   else {
      if (((KVFAZIADetector*)PART->GetStoppingDetector())->GetIdentifier() == KVFAZIADetector::kCSI
            && !(PART->GetStoppingDetector()->IsCalibrated()) && ndet_calib == 2) {
         if (PART->GetZ() > 0) {
//            if (!PART->IsAMeasured()) {
//               if (GetZ() == 1)       SetA(1);
//               else if (GetZ() == 2)  SetA(4);
//               else if (GetZ() == 20) SetA(48);
//               else {
//                  SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
//               }
//            }

            Double_t E_targ = 0;
            fECSI = ((KVDetector*)PART->GetStoppingDetector()->GetNode()->GetDetectorsInFront()->First())->GetEResFromDeltaE(PART->GetZ(), PART->GetA(), fESI2);
            PART->SetEnergy(fECSI + fESI1 + fESI2);

            E_targ = ((KVMultiDetArray*)GetGroup()->GetArray())->GetTargetEnergyLossCorrection(PART);
            Double_t E_tot = PART->GetEnergy() + E_targ;
            PART->SetECode(1);
            PART->SetIsCalibrated();
            PART->SetEnergy(E_tot);
            PART->GetAnglesFromStoppingDetector();
         }
      }
   }

//   if(PART->IsCalibrated()) Info("CalibrateParticle","calibrate particle (%d,%d) E = %lf MeV/A",PART->GetZ(),PART->GetA(), PART->GetKE()/PART->GetA());

}

void KVFAZIAGroupReconstructor::PostReconstructionProcessing()
{
   // Copy FPGA energy values to reconstructed particle parameter lists
   // Set values in detectors for identification/calibration procedures

   for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
      KVReconstructedNucleus* rnuc = it.pointer<KVReconstructedNucleus>();

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

         // now copy all detector signals to reconstructed particle parameter list...
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
   // Check for gammas identified in CsI which hide another particle stopping in Si2 or Si1

   KVGroupReconstructor::IdentifyParticle(PART);
   if (partID.IsType("CsI") && partID.IDquality == KVIDGCsI::kICODE10) {
      // look at Si1-Si2 identification
      std::map<std::string, KVIdentificationResult*>::iterator si1si2 = id_by_type.find("Si-Si");
      if (si1si2 != id_by_type.end()) {
         if (si1si2->second->IDattempted && si1si2->second->IDquality < KVIDZAGrid::kICODE4) {
            Info("IdentifyParticle", "Gamma identified in %s replaced with Si1-Si2 identification:", PART.GetStoppingDetector()->GetName());
            si1si2->second->Print();
            partID = *(si1si2->second);
            identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("CsI");
            PART.SetIdentifyingTelescope(identifying_telescope);
            PART.SetIdentification(&partID, identifying_telescope);
         }
      }
   }
}
