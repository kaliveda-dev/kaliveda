//Created by KVClassFactory on Thu Dec 15 16:01:29 2016
//Author: John Frankland,,,

#ifndef __KVINDRAEVENTSELECTOR_H
#define __KVINDRAEVENTSELECTOR_H

#include "KVEventSelector.h"

#include <KVINDRADBRun.h>
#include <KVINDRAReconEvent.h>

/**
\class KVINDRAEventSelector
\brief Base class for analysis of reconstructed INDRA events
\ingroup INDRAnalysis
*/

class KVINDRAEventSelector : public KVEventSelector {

protected:
   KVINDRADBRun* fCurrentRun;   //! current run

public:
   KVINDRAEventSelector(TTree* /*arg1*/ = 0);
   virtual ~KVINDRAEventSelector();

   void Init(TTree* tree);

   void SetCurrentRun(KVDBRun* r)
   {
      fCurrentRun = (KVINDRADBRun*)r;
   }
   KVINDRADBRun* GetCurrentRun() const
   {
      return fCurrentRun;
   }
   KVINDRAReconEvent* GetEvent() const
   {
      return (KVINDRAReconEvent*)KVEventSelector::GetEvent();
   }
   static void Make(const Char_t* kvsname = "MyOwnINDRASelector");

   ClassDef(KVINDRAEventSelector, 1) //Base class for analysis of reconstructed INDRA events
};

#endif
