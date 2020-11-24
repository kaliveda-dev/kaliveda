//Created by KVClassFactory on Fri Feb  7 16:31:27 2020
//Author: John Frankland,,,

#include "KVDataPatch_INDRA_SanityChecks.h"

#include <KVINDRAReconNuc.h>
#include <KVINDRA.h>

ClassImp(KVDataPatch_INDRA_SanityChecks)

KVDataPatch_INDRA_SanityChecks::KVDataPatch_INDRA_SanityChecks()
   : KVDataPatch()
{
   // Default constructor
   SetName(ClassName());
   SetTitle(Class()->GetTitle());
}

void KVDataPatch_INDRA_SanityChecks::ApplyToParticle(KVReconstructedNucleus* N)
{
   // General sanity checks:
   //  - any particle with "good" ID codes should have:
   //        IsIdentified() = true
   //        IsZMeasured() = true
   //        GetIdentifyingTelescope() returns pointer to ID telescope
   //  - any particle with IsZMeasured()=true should have:
   //        1 <= Z <= 92
   //  - any particle with 'good' calib codes should have
   //        E>0
   //  - any particle identified in CsI-RL should have
   //        Z<6
   //        no 4H, no 5He or 7He

   KVINDRAReconNuc* n = dynamic_cast<KVINDRAReconNuc*>(N);
   if ((n->GetIDCode() >= 2 && n->GetIDCode() <= 4) || n->GetIDCode() == 6) {
      if (!n->IsIdentified() || !n->IsZMeasured() || !n->GetIdentifyingTelescope()) {
         // should this be labelled code5 (stopped first stage) ?
         if (n->GetReconstructionTrajectory()->GetN() == 1 && !n->GetStoppingDetector()->IsType("CSI"))
            n->SetIDCode(kIDCode5);
         else
            n->SetIDCode(kIDCode14);

         gIndra->AcceptParticleForAnalysis(n);
      }
      else if (n->IsZMeasured()) {
         if (n->GetZ() < 1 || n->GetZ() > 92) {
            n->SetIDCode(kIDCode14);
            n->SetIsUnidentified();
            n->SetZMeasured(kFALSE);
            gIndra->AcceptParticleForAnalysis(n);
         }
      }
      if (n->GetIDCode() == 2 && n->GetIdentifyingTelescope()->IsType("CSI_R_L") && n->IsAMeasured()) {
         if (n->GetZ() > 5) {
            n->SetIDCode(kIDCode14);
            n->SetIsUnidentified();
            n->SetZMeasured(kFALSE);
            n->SetAMeasured(kFALSE);
            gIndra->AcceptParticleForAnalysis(n);
         }
         else {
            if ((n->GetZ() == 1 && n->GetA() > 3)
                  || ((n->GetZ() == 2 && (n->GetA() < 3 || n->GetA() == 5 || n->GetA() > 6)))) {
               n->SetIDCode(kIDCode14);
               n->SetIsUnidentified();
               n->SetZMeasured(kFALSE);
               n->SetAMeasured(kFALSE);
               gIndra->AcceptParticleForAnalysis(n);
            }
         }
      }
   }
   if (n->GetECode() > 0 && n->GetECode() < 3) {
      if (n->GetE() <= 0) {
         n->SetECode(kECode0);
         n->SetIsUncalibrated();
         gIndra->AcceptParticleForAnalysis(n);
      }
   }
}

void KVDataPatch_INDRA_SanityChecks::PrintPatchInfo() const
{
   std::cout <<  "General sanity checks:\n";
   std::cout <<  "  - any particle with 'good' ID codes should have\n";
   std::cout <<  "    IsIdentified() = true\n";
   std::cout <<  "    IsZMeasured() = true\n";
   std::cout <<  "    GetIdentifyingTelescope() returns pointer to ID telescope\n";
   std::cout <<  "  - any particle with IsZMeasured()=true should have:\n";
   std::cout <<  "        1 <= Z <= 92\n";
   std::cout <<  "  - any particle with 'good' calib codes should have\n";
   std::cout <<  "        E>0\n";
   std::cout <<  "  - any particle identified in CsI-RL should have\n";
   std::cout <<  "        Z<6\n";
   std::cout <<  "        no 4H, no 5He or 7He\n";
}

