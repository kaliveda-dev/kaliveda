//Created by KVClassFactory on Mon Sep  7 11:38:39 2015
//Author: ,,,

#ifndef __KVQL1_H
#define __KVQL1_H

#include "KVSignal.h"

class KVQL1 : public KVSignal {

protected:
   void init();

public:
   KVQL1();
   KVQL1(const char* name);

   virtual ~KVQL1();
   void Copy(TObject& obj) const;

   virtual void SetDefaultValues();
   virtual void LoadPSAParameters();
   virtual Bool_t IsCharge() const
   {
      return kTRUE;
   }

   virtual void TreateSignal();
   virtual void GetPSAResult(KVDetector*) const;

   ClassDef(KVQL1, 1) //QL1 channel of SI1
};

#endif
