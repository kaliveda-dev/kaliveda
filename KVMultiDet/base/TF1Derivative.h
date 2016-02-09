//Created by KVClassFactory on Fri Nov  6 11:04:13 2015
//Author: John Frankland,,,

#ifndef __TF1DERIVATIVE_H
#define __TF1DERIVATIVE_H

#include "TF1.h"

class TF1Derivative : public TF1 {
   TF1* fFunction;
   int fOrder;

public:
   TF1Derivative();
   TF1Derivative(TF1* func, int order = 1);
   virtual ~TF1Derivative();
   void Copy(TObject& obj) const;

   Double_t EvalPar(const Double_t* x, const Double_t* params = 0);

   ClassDef(TF1Derivative, 1) //Numerical derivative of a TF1
};

#endif
