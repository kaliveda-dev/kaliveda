//Created by KVClassFactory on Thu Mar  2 10:50:41 2017
//Author: Diego Gruyer

#ifndef __KVEXCITEDSTATE_H
#define __KVEXCITEDSTATE_H

#include "TNamed.h"
#include "KVString.h"

/**
\class KVExcitedState
\brief Excited state of atomic nucleus
\ingroup NucProp
*/

class KVExcitedState: public TNamed {
public:
   TString fJPi;
   TString fGamma;

   Double_t fEnergy;
   Double_t fWidth;
   Int_t fSpin;
   Int_t fParity;
   Bool_t fIsOdd;

public:
   KVExcitedState();
   virtual ~KVExcitedState();

   void print();
   void set(Double_t ee, Double_t ww, Double_t jj, Int_t pi);
   void set(Double_t ee, const char* jpi, const char* t12);
   void computeParity();
   void computeSpin();
   void computeWidth();
   Double_t eval(Double_t excit);

   ClassDef(KVExcitedState, 1) //
};

#endif
