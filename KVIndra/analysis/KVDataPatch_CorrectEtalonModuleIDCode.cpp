//Created by KVClassFactory on Thu Jan 30 15:16:45 2014
//Author: John Frankland,,,

#include "KVDataPatch_CorrectEtalonModuleIDCode.h"
#include <KVINDRAReconNuc.h>
#include <KVINDRAReconEvent.h>
#include "TClass.h"

ClassImp(KVDataPatch_CorrectEtalonModuleIDCode)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataPatch_CorrectEtalonModuleIDCode</h2>
<h4>Patch for correcting bad id-code & calibration for particles stopping in CsI member of etalon modules [data written with v1.8.11]</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVDataPatch_CorrectEtalonModuleIDCode::KVDataPatch_CorrectEtalonModuleIDCode()
{
   // Default constructor
   SetName(ClassName());
   SetTitle(Class()->GetTitle());
}

KVDataPatch_CorrectEtalonModuleIDCode::~KVDataPatch_CorrectEtalonModuleIDCode()
{
   // Destructor
}

Bool_t KVDataPatch_CorrectEtalonModuleIDCode::IsRequired(TString dataset, TString datatype, Int_t, TString dataseries, Int_t datareleasenumber, const TList*)
{
   // Patch is applied to all runs of INDRA 5th campaign 'root' or 'ident' data
   // written with KaliVeda version 1.8.11

   return (dataset == "INDRA_camp5"
           && (datatype == "root" || datatype == "ident")
           && dataseries == "1.8" && datareleasenumber == 11);
}

void KVDataPatch_CorrectEtalonModuleIDCode::ApplyToEvent(KVEvent* e)
{
   // Just used to set pointer to current event
   fEvent = (KVINDRAReconEvent*)e;
}

void KVDataPatch_CorrectEtalonModuleIDCode::ApplyToParticle(KVNucleus* n)
{
   // Patch is applied to all identified particles stopping in the CsI member of
   // an etalon module which have general ID code 5.
   // If the CsI identification is available and was succesful, then either
   //    (1) for gammas we set the correct ID code and set the energy = 0
   //    (2) for charged particles we correct the ID code and the energy
   //        which is probably overestimated (due to taking into account the
   //        Si75/SiLi in the calibration when in fact they were not hit)

   KVINDRAReconNuc* N = (KVINDRAReconNuc*)n;
   if (N->IsIdentified() && N->GetRingNumber() > 9 && N->StoppedInCsI() && (N->GetSiLi() || N->GetSi75())
         && N->GetCodes().TestIDCode(kIDCode5)) {
      KVIdentificationResult* idr = N->GetIdentificationResult(1);
      if (idr->IDattempted && idr->IsType("CSI_R_L") && idr->IDOK) {
         if (idr->IDcode == kIDCode0) {
            // gamma
            // just set general IDcode gamma and set energy = 0
            N->SetIdentification(idr);
            N->SetEnergy(0.);
            N->GetParameters()->SetValue("KVDataPatch_CorrectEtalonModuleIDCode", "correction applied");
         } else {
            // charged particle
            // needs re-identifying and re-calibrating
            KVIdentificationResult ID;
            Bool_t ok = N->CoherencyChIoCsI(ID);
            if (ok) {
               N->SetIdentification(&ID);
               N->Calibrate();
               N->GetParameters()->SetValue("KVDataPatch_CorrectEtalonModuleIDCode", "correction applied");
            } else {
               Warning("ApplyToParticle", "Rustine failed for the following particle:");
               N->GetParameters()->SetValue("KVDataPatch_CorrectEtalonModuleIDCode", "correction failed");
               N->Print();
            }
         }
         // update particle's 'OK' status
         N->SetIsOK(fEvent->CheckCodes(N->GetCodes()));
      }
   }
}

void KVDataPatch_CorrectEtalonModuleIDCode::PrintPatchInfo() const
{
   std::cout << "Correct bad attribution of IDCode & calibration of particles stopping" << std::endl;
   std::cout << "in CsI detectors behind etalon telescopes." << std::endl;
   std::cout << "Gammas (IDcode=0) and light charged particles (IDcode=2) were given" << std::endl;
   std::cout << "IDcode=5 due to a bug in v1.8.11 (lp:bug#1271634)" << std::endl << std::endl;
   std::cout << "The status of the correction can be obtained from parameter" << std::endl;
   std::cout << "KVDataPatch_CorrectEtalonModuleIDCode: " << std::endl;
   std::cout << "   ='correction applied' or 'correction failed'" << std::endl;
}

