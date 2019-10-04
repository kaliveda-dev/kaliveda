//Created by KVClassFactory on Thu Jun 13 12:20:05 2019
//Author: John Frankland,,,

#ifndef __KVACQPARAMSIGNAL_H
#define __KVACQPARAMSIGNAL_H

#include "KVDetectorSignal.h"
#include "KVACQParam.h"

class KVACQParamSignal : public KVDetectorSignal {
   KVACQParam* fParam;

public:
   KVACQParamSignal()
      : KVDetectorSignal(), fParam(nullptr)
   {}
   KVACQParamSignal(KVACQParam* p)
      : KVDetectorSignal(p->GetType(), p->GetDetector()), fParam(p)
   {}

   virtual ~KVACQParamSignal()
   {}

   Double_t GetValue() const
   {
      return fParam->GetData();
   }
   void SetValue(Double_t x)
   {
      fParam->SetData(x);
   }
   void Reset()
   {
      fParam->Clear();
   }
   ClassDef(KVACQParamSignal, 1) //Wrapper signal for KVACQParam objects
};

#endif
