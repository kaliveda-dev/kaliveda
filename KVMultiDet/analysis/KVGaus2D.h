//Created by KVClassFactory on Tue Feb  7 16:49:15 2012
//Author: bonnet

#ifndef __KVGAUS2D_H
#define __KVGAUS2D_H

#include "KVAutoFit.h"
#include "TF1.h"

/**
\class KVGaus2D
\brief Handle semi-automatic fit to 2D Gaussian distributions
\ingroup Math
*/

class KVGaus2D : public KVAutoFit {

protected:

   virtual Bool_t NewFunction_2D();
   virtual TF1* ReloadFunction_2D(const Char_t*, Int_t);
   virtual Double_t f2D(Double_t* xx, Double_t* para);

public:

   KVGaus2D(Bool_t batch = kFALSE);
   KVGaus2D(const KVGaus2D&) ;
   ROOT_COPY_ASSIGN_OP(KVGaus2D)
   virtual ~KVGaus2D();

   ClassDef(KVGaus2D, 1) //Fit gaussien a deux dimensions
   void ExtraDrawing();
};

#endif
