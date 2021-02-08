//Created by KVClassFactory on Thu Jul 19 15:38:10 2018
//Author: eindra

#ifndef __KVRECONEVENTSELECTOR_H
#define __KVRECONEVENTSELECTOR_H

#include "KVEventSelector.h"
#include "KVDBRun.h"
#include "KVReconstructedEvent.h"

/**
   \class KVReconEventSelector
\brief Base class for user analysis of reconstructed data
\ingroup Analysis
*/

class KVReconEventSelector : public KVEventSelector {
   KVDBRun* fCurrentRun;//current run being analysed

public:
   KVReconEventSelector(TTree* arg1 = 0) : KVEventSelector(arg1), fCurrentRun(nullptr)
   {
      SetBranchName("ReconEvent");
      SetParticleConditionsParticleClassName("KVReconstructedNucleus");
   }
   virtual ~KVReconEventSelector() {}

   void SetCurrentRun(KVDBRun* r)
   {
      fCurrentRun = r;
   }
   KVDBRun* GetCurrentRun() const
   {
      return fCurrentRun;
   }
   void Init(TTree* tree);

   Int_t GetEventNumber()
   {
      // returns number of currently analysed event
      // N.B. this may be different to the TTree/TChain entry number etc.
      return GetEvent()->GetNumber();
   }
   KVReconstructedEvent* GetEvent() const
   {
      return (KVReconstructedEvent*)KVEventSelector::GetEvent();
   }
   static void Make(const Char_t* kvsname);

   ClassDef(KVReconEventSelector, 1) //Base analysis class for reconstructed data
};

#endif
