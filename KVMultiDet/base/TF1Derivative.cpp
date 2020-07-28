//Created by KVClassFactory on Fri Nov  6 11:04:13 2015
//Author: John Frankland,,,

#include "TF1Derivative.h"

ClassImp(TF1Derivative)


TF1Derivative::TF1Derivative() : TF1(), fFunction(nullptr), fOrder(1)
{
}

TF1Derivative::TF1Derivative(TF1* func, int order) : TF1(*func), fFunction(func), fOrder(order)
{
   // Constructor with function to differentiate and order required (up to 3rd order).
}


TF1Derivative::~TF1Derivative()
{
   // Destructor
}

//________________________________________________________________

void TF1Derivative::Copy(TObject& obj) const
{

   TF1::Copy(obj);
   TF1Derivative& CastedObj = (TF1Derivative&)obj;
   CastedObj.fFunction = fFunction;
   CastedObj.fOrder = fOrder;
}

Double_t TF1Derivative::EvalPar(const Double_t* x, const Double_t* p)
{
   // Returns fOrder-th derivative of function at x (fOrder=1,2,3)

   fFunction->EvalPar(x, p);
   Double_t result = 0;
   switch (fOrder) {
      case 2:
         result = fFunction->Derivative2(*x, (double*)p);
         break;
      case 3:
         result = fFunction->Derivative3(*x, (double*)p);
         break;
      case 1:
      default:
         result = fFunction->Derivative(*x, (double*)p);
   }

   return result;
}


