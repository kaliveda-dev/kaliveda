//Created by KVClassFactory on Wed Aug 28 11:48:51 2013
//Author: Guilain ADEMARD

#ifndef __KVRECOMBINATION_H
#define __KVRECOMBINATION_H

#include "KVCalibrator.h"

class KVRecombination : public KVCalibrator {

   TF1* fParlog;    //! Parlog formula for PHD = f(E,Z,A)
   TF1* fDeltaEphd; //! deltaE calculated including PHD
   Int_t fZ;        //! Z of nucleus to be calibrated
   Int_t fA;        //! A of nucleus to be calibrated
   Bool_t wrong;     //!

public:

   void init();

   KVRecombination();
   KVRecombination(KVDetector*);
   virtual ~KVRecombination();

   virtual Double_t Compute(Double_t E) const;
   virtual Double_t operator()(Double_t);
   virtual Double_t Invert(Double_t);

   void SetZandA(Int_t z, Int_t a)
   {
      fZ = z;
      fA = a;
   };
   Int_t GetZ() const
   {
      return fZ;
   };
   Int_t GetA() const
   {
      return fA;
   };

   Double_t PHDParlog(Double_t* x, Double_t* par);
   TF1*     GetParlogPHDFunction(Int_t Z, Int_t A);
   Double_t ELossActive(Double_t* x, Double_t* par);
   TF1*     GetELossFunction(Int_t Z, Int_t A, Bool_t Wrong = kFALSE);


   ClassDef(KVRecombination, 1) //Silicon PHD described by electron-hole recombination model
};

#endif
