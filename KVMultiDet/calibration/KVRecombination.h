//Created by KVClassFactory on Wed Aug 28 11:48:51 2013
//Author: Guilain ADEMARD

#ifndef __KVRECOMBINATION_H
#define __KVRECOMBINATION_H

#include "KVCalibrator.h"

/**
\class KVRecombination
\brief Silicon PHD described by electron-hole recombination model

<p>
The static TF1 "fParlog" calculates the PHD of a particle of incident energy
E, charge Z and Mass A calculated according to Parlog's formula (see Phys. Res. A 428 (1999) 379):
</p>

~~~~
      PHD(Ed) = 1/2 * Ed *( 1-1/X * ln|1+X| + X * ln|1+1/X| )
 with X      = a*A*Z**2/Ed
      Ed     = energy lost by particle in detector (=E if particle stops)
~~~~

<p>Only one parameter (a) can be set using method<br>
SetParameters(a) or SetParameter(0,a).
The charge and the mass numbers have to be set first the calculation using<br>
method SetZandA( Z, A).
</p>
*/

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

   virtual Double_t Compute(Double_t, const KVNameValueList& = "") const;
   virtual Double_t Invert(Double_t, const KVNameValueList& = "") const;

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
