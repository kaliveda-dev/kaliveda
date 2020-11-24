//Created by KVClassFactory on Thu Jan 30 14:22:15 2014
//Author: John Frankland,,,

#include "KVDataPatch_INDRA_camp5_PHDcorrection.h"
#include <TStreamerInfo.h>
#include <KVINDRAReconNuc.h>
#include <KVTarget.h>
#include "KVMultiDetArray.h"

ClassImp(KVDataPatch_INDRA_camp5_PHDcorrection)


KVDataPatch_INDRA_camp5_PHDcorrection::KVDataPatch_INDRA_camp5_PHDcorrection()
{
   // Default constructor
   SetName(ClassName());
   SetTitle(Class()->GetTitle());
}

KVDataPatch_INDRA_camp5_PHDcorrection::~KVDataPatch_INDRA_camp5_PHDcorrection()
{
   // Destructor
}

Bool_t KVDataPatch_INDRA_camp5_PHDcorrection::IsRequired(TString dataset, TString datatype, Int_t,
      TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist)
{
   // Patch is applied to all runs of INDRA 5th campaign 'root' data written with KaliVeda
   // version <1.8.10 and KVINDRAReconNuc class version < 11

   TStreamerInfo* i = (TStreamerInfo*)streamerinfolist->FindObject("KVINDRAReconNuc");
   return (dataset == "INDRA_camp5" && datatype == "root" && dataseries == "1.8"
           && datareleasenumber < 10 && i->GetClassVersion() < 11);
}

void KVDataPatch_INDRA_camp5_PHDcorrection::ApplyToParticle(KVReconstructedNucleus* n)
{
   // Patch is applied to all identified & calibrated nuclei with Z>10 on rings 1-9.
   // The particle energy is recalibrated.

   KVINDRAReconNuc* N = (KVINDRAReconNuc*)n;
   if (N->IsIdentified() && N->IsCalibrated() && N->GetZ() > 10 && N->GetRingNumber() < 10) {
      KVTarget* t = gMultiDetArray->GetTarget();
      if (t) {
         // make sure target is in correct state to calculate
         // target energy losses of fragments
         if (t->IsIncoming()) t->SetIncoming(kFALSE);
         if (!t->IsOutgoing()) t->SetOutgoing(kTRUE);
      }
      N->Calibrate();//recalibrate particle
   }
}

void KVDataPatch_INDRA_camp5_PHDcorrection::PrintPatchInfo() const
{
   std::cout << "Particle-level patch applied to all runs of INDRA 5th campaign 'root'" << std::endl;
   std::cout << "data written with KaliVeda version <1.8.10 and KVINDRAReconNuc class" << std::endl;
   std::cout << "version < 11." << std::endl;
   std::cout << "Patch is applied to all identified & calibrated nuclei with Z>10 on rings 1-9." << std::endl;
   std::cout << "The particle energy is recalibrated." << std::endl;
}
