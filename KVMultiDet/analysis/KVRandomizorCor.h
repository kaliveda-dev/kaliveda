//Created by KVClassFactory on Mon Jun 26 17:31:12 2017
//Author: Eric BONNET

#ifndef __KVRANDOMIZORCOR_H
#define __KVRANDOMIZORCOR_H

#include "KVRandomizor.h"

/**
 \class KVRandomizorCor
\ingroup Math
 */

class KVRandomizorCor : public KVRandomizor {
public:
   KVRandomizorCor(Int_t ndim);

   virtual ~KVRandomizorCor();

   virtual Double_t ComputeValue(Double_t* pos);

   ClassDef(KVRandomizorCor, 1) //KVRandomizor
};

#endif
