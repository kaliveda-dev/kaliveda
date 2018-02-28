//Created by KVClassFactory on Thu Jun 24 11:04:34 2010
//Author: John Frankland,,,

#ifndef __KVRUNGEKUTTA_H
#define __KVRUNGEKUTTA_H

#include "KVBase.h"

#define SAFETY 0.9
#define PGROW -0.2
#define PSHRNK -0.25
#define ERRCON 1.89e-4
#define MAXSTP 10000
#define TINY 1.0e-30

/*The value ERRCON equals (5/SAFETY) raised to the power (1/PGROW), see use below.*/

class KVRungeKutta : public KVBase {
   static Double_t a2, a3, a4, a5, a6, b21;
   static Double_t b31, b32, b41, b42, b43;
   static Double_t b51, b52, b53, b54;
   static Double_t b61, b62, b63;
   static Double_t b64, b65, c1 ;
   static Double_t c3, c4, c6;
   static Double_t dc5;
protected:
   Int_t nvar; // number of independent variables/equations
   Double_t eps; // precision required
   Double_t hmin; // minimum allowed step size
   Int_t nok; // number of good steps taken
   Int_t nbad; // number of bad steps taken

   Double_t* y, *yscal, *dydx, *yerr, *ytemp, *yout;
   Double_t x, hnext, hdid;
   Double_t* ak2, *ak3, *ak4, *ak5, *ak6;
   Double_t dc1, dc3, dc4, dc6;
   Bool_t fOK;
   Bool_t fInitialDeriv;
   virtual void rkqs(Double_t htry);
   virtual void rkck(Double_t h);

public:
   KVRungeKutta() {};
   KVRungeKutta(Int_t N, Double_t PREC = 1.e-8, Double_t MINSTEP = 0.0);
   virtual ~KVRungeKutta();

   virtual void Integrate(Double_t* ystart, Double_t x1, Double_t x2, Double_t h1);

   // This method must be implemented in child classes in order to fill the
   // array DYDX[nvar] with the calculated values of the derivatives given
   // the value of X and the values of Y[nvar]
   virtual void CalcDerivs(Double_t X, Double_t* Y, Double_t* DYDX) = 0;

   Int_t GetNGoodSteps() const
   {
      // Return number of succesful steps taken in integration
      return nok;
   }
   Int_t GetNBadSteps() const
   {
      // Return number of bad (but retried and fixed) steps taken in integration
      return nbad;
   }
   Bool_t IsOK() const
   {
      // Can be used after call to Integrate to check if successful
      return fOK;
   }

   ClassDef(KVRungeKutta, 0) //Adaptive step-size 4th order Runge-Kutta ODE integrator from Numerical Recipes
};

#endif
