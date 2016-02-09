//Created by KVClassFactory on Mon Sep  7 11:38:39 2015
//Author: ,,,

#ifndef __KVI2_H
#define __KVI2_H

#include "KVSignal.h"

class KVI2 : public KVSignal {

protected:
   void init();

public:
   KVI2();
   KVI2(const char* name);

   virtual ~KVI2();
   void Copy(TObject& obj) const;

   virtual void SetDefaultValues();
   virtual void LoadPSAParameters();
   virtual Bool_t IsCurrent() const
   {
      return kTRUE;
   }

   virtual void TreateSignal();
   virtual KVPSAResult* GetPSAResult() const;
   virtual Double_t ComputeBaseLine();

   ClassDef(KVI2, 1) //I2 channel of SI2
};

#endif
