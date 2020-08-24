//Created by KVClassFactory on Thu Dec 15 14:55:48 2016
//Author: John Frankland,,,

#include "KVPROOFLiteBatch.h"

#include <TProof.h>

ClassImp(KVPROOFLiteBatch)

KVPROOFLiteBatch::KVPROOFLiteBatch(const Char_t* name)
   : KVBatchSystem(name)
{
}

//____________________________________________________________________________//

KVPROOFLiteBatch::~KVPROOFLiteBatch()
{
   // Destructor
}

void KVPROOFLiteBatch::SubmitTask(KVDataAnalyser* da)
{
   // Run analysis on PROOFLite facility

   // Open PROOFLite session and initialise KaliVeda package
   if (!gProof) {
      TProof* p = TProof::Open("");
      p->ClearCache();//to avoid problems with compilation of KVParticleCondition
      // enable KaliVeda on PROOF cluster
      if (p->EnablePackage("KaliVeda") != 0) {
         // first time, need to 'upload' package
         TString fullpath = KVBase::GetETCDIRFilePath("KaliVeda.par");
         p->UploadPackage(fullpath);
         p->EnablePackage("KaliVeda");
      }
   }
#ifdef WITH_CPP11
   da->SetProofMode(KVDataAnalyser::EProofMode::Lite);
#else
   da->SetProofMode(KVDataAnalyser::Lite);
#endif
   SetAnalyser(da);
   da->SubmitTask();
}

//____________________________________________________________________________//

