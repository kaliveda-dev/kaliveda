//Created by KVClassFactory on Fri Feb  7 13:42:32 2020
//Author: John Frankland,,,

#include "KVDataPatch_INDRA_CorrectEtalonTrajectories.h"

#include <KVINDRAReconNuc.h>
#include <KVReconstructedEvent.h>

ClassImp(KVDataPatch_INDRA_CorrectEtalonTrajectories)

KVDataPatch_INDRA_CorrectEtalonTrajectories::KVDataPatch_INDRA_CorrectEtalonTrajectories()
   : KVDataPatch(), fRandomAngles(false)
{
   // Default constructor
   SetName(ClassName());
   SetTitle(Class()->GetTitle());
}

void KVDataPatch_INDRA_CorrectEtalonTrajectories::ApplyToEvent(KVReconstructedEvent* e)
{
   // Will be called before ApplyToParticle
   //
   // Just need to know if event has mean or random angles

   fRandomAngles = dynamic_cast<KVReconstructedEvent*>(e)->HasRandomAngles();
}

void KVDataPatch_INDRA_CorrectEtalonTrajectories::ApplyToParticle(KVReconstructedNucleus* N)
{
   // Apply the following correction to INDRA data:
   //    all particles identified in etalon modules by either CSI_R_L or CI_CSI
   //    identification were treated as if they passed through the etalon detectors.
   //    we check the coherency of this with the energy of the particle and
   //    correct when necessary

   if (!N->IsOK()) return; //only apply to well-identified/calibrated particles

   KVINDRAReconNuc* n = dynamic_cast<KVINDRAReconNuc*>(N);

   KVDetector* stop_det(nullptr), *si75(nullptr), *sili(nullptr);
   if (n->GetRingNumber() >= 10 &&
         (stop_det = n->GetStoppingDetector())->IsType("CSI")
         &&
         ((si75 = n->GetReconstructionTrajectory()->GetDetector("SI75"))
          || (sili = n->GetReconstructionTrajectory()->GetDetector("SILI")))
      ) {
      // could particle have passed through SI75 and SILI ?
      n->GetReconstructionTrajectory()->IterateBackFrom();
      KVNucleus avatar(n->GetZ(), n->GetA(), n->GetMomentum());
      Double_t E0 = avatar.GetE();
      KVGeoDetectorNode* node(nullptr);
      while ((node = n->GetReconstructionTrajectory()->GetNextNode())) {
         E0 -= node->GetDetector()->GetELostByParticle(&avatar);
         if (E0 <= 0) break;
      }
      if (node && !node->GetDetector()->IsType("CSI")) {
         // particle could not have reached CsI: therefore it is on recon trajectory without etalon detector(s)
         TIter next_tr(stop_det->GetNode()->GetTrajectories());
         KVGeoDNTrajectory* tr;
         while ((tr = (KVGeoDNTrajectory*)next_tr())) {
            // find trajectory starting from csi with neither si75 nor sili
            bool no_si75 = !si75 || !tr->Contains(si75->GetNode());
            bool no_sili = !sili || !tr->Contains(sili->GetNode());
            if (no_si75 && no_sili) {
               // name of this trajectory is title of recon trajectory we want
               // replace particle's trajectory with corresponding recon trajectory
               n->ReplaceReconTraj(tr->GetName());
               // redefine particle angles from new trajectory
               if (fRandomAngles) n->GetAnglesFromReconstructionTrajectory("random");
               else n->GetAnglesFromReconstructionTrajectory("mean");
               break;
            }
         }
      }
   }
}

void KVDataPatch_INDRA_CorrectEtalonTrajectories::PrintPatchInfo() const
{
   std::cout <<  "Apply the following correction to INDRA data:\n";
   std::cout <<  "   all particles identified in etalon modules by either CSI_R_L or CI_CSI\n";
   std::cout <<  "   identification were treated as if they passed through the etalon detectors.\n";
   std::cout <<  "   we check the coherency of this with the energy of the particle and\n";
   std::cout <<  "   correct when necessary\n";
}

