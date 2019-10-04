//Created by KVClassFactory on Mon Sep  7 11:38:39 2015
//Author: ,,,

#ifndef __KVI1_H
#define __KVI1_H

#include "KVSignal.h"

class KVI1 : public KVSignal {

protected:
   void init();

public:
   KVI1();
   KVI1(const char* name);

   virtual ~KVI1();
   void Copy(TObject& obj) const;

   virtual void SetDefaultValues();
   virtual void LoadPSAParameters();
   virtual Bool_t IsCurrent() const
   {
      return kTRUE;
   }

   virtual void TreateSignal();
   virtual void GetPSAResult(KVDetector*) const;
   virtual Double_t ComputeBaseLine();

   ClassDef(KVI1, 1) //I1 channel of SI1
};

#endif
