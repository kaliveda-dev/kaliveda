//Created by KVClassFactory on Mon Jun 26 17:31:07 2017
//Author: Eric BONNET

#ifndef __KVRANDOMIZORIND_H
#define __KVRANDOMIZORIND_H

#include "KVRandomizor.h"

/**
  \class KVRandomizor
  \ingroup Math
 */

class KVRandomizorInd : public KVRandomizor {
public:
   KVRandomizorInd(Int_t ndim);

   virtual ~KVRandomizorInd();
   virtual Double_t ComputeValue(Double_t* pos);

   virtual Double_t  ComputeValue0(Double_t);
   virtual Double_t  ComputeValue1(Double_t);
   virtual Double_t  ComputeValue2(Double_t);
   virtual Double_t  ComputeValue3(Double_t);
   virtual Double_t  ComputeValue4(Double_t);
   virtual Double_t  ComputeValue5(Double_t);

   ClassDef(KVRandomizorInd, 1) //KVRandomizor
};

#endif
