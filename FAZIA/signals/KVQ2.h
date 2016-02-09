//Created by KVClassFactory on Mon Sep  7 11:38:39 2015
//Author: ,,,

#ifndef __KVQ2_H
#define __KVQ2_H

#include "KVSignal.h"

class KVQ2 : public KVSignal {

protected:
   void init();

public:
   KVQ2();
   KVQ2(const char* name);

   virtual ~KVQ2();
   void Copy(TObject& obj) const;

   virtual void SetDefaultValues();
   virtual void LoadPSAParameters();
   virtual Bool_t IsCharge() const
   {
      return kTRUE;
   }

   virtual void TreateSignal();
   virtual KVPSAResult* GetPSAResult() const;

   ClassDef(KVQ2, 1) //charge Q2 channel of SI2
};

#endif
