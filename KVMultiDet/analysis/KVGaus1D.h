//Created by KVClassFactory on Tue Feb  7 16:09:40 2012
//Author: bonnet

#ifndef __KVGAUS1D_H
#define __KVGAUS1D_H

#include "KVAutoFit.h"

/**
\class KVGaus1D
\brief Handle semi-automatic fit to 1D Gaussian distributions
\ingroup Math
*/

class KVGaus1D : public KVAutoFit {

protected:
   Bool_t NewFunction_1D();

public:
   KVGaus1D(Bool_t batch = kFALSE);
   KVGaus1D(const KVGaus1D&) ;
   ROOT_COPY_ASSIGN_OP(KVGaus1D)
   virtual ~KVGaus1D();

   void Gather();

   ClassDef(KVGaus1D, 1) //Fit gaussien a une dimension
};

#endif
