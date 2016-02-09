//Created by KVClassFactory on Mon Sep  7 11:38:39 2015
//Author: ,,,

#ifndef __KVQH1_H
#define __KVQH1_H

#include "KVSignal.h"

class KVQH1 : public KVSignal {

protected:
   void init();

public:
   KVQH1();
   KVQH1(const char* name);

   virtual ~KVQH1();
   void Copy(TObject& obj) const;

   virtual void SetDefaultValues();
   virtual void LoadPSAParameters();
   virtual Bool_t IsCharge() const
   {
      return kTRUE;
   }

   virtual void TreateSignal();
   virtual KVPSAResult* GetPSAResult() const;

   ClassDef(KVQH1, 1) //QH1 channel of SI1
};

#endif
