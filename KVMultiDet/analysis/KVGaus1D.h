//Created by KVClassFactory on Tue Feb  7 16:09:40 2012
//Author: bonnet

#ifndef __KVGAUS1D_H
#define __KVGAUS1D_H

#include "KVAutoFit.h"

class KVGaus1D : public KVAutoFit {

protected:
   Bool_t NewFunction_1D();

public:
   KVGaus1D();
   KVGaus1D(const KVGaus1D&) ;
   virtual ~KVGaus1D();

   void Gather();

   ClassDef(KVGaus1D, 1) //Fit gaussien a une dimension
};

#endif
