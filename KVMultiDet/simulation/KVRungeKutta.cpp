//Created by KVClassFactory on Thu Jun 24 11:04:34 2010
//Author: John Frankland,,,

#include "KVRungeKutta.h"
#include "TMath.h"

ClassImp(KVRungeKutta)



Double_t KVRungeKutta::a2 = 0.2;
Double_t KVRungeKutta::a3 = 0.3;
Double_t KVRungeKutta::a4 = 0.6;
Double_t KVRungeKutta::a5 = 1.0;
Double_t KVRungeKutta::a6 = 0.875;
Double_t KVRungeKutta::b21 = 0.2;
Double_t KVRungeKutta::b31 = 3.0 / 40.0;
Double_t KVRungeKutta::b32 = 9.0 / 40.0;
Double_t KVRungeKutta::b41 = 0.3;
Double_t KVRungeKutta::b42 = -0.9;
Double_t KVRungeKutta::b43 = 1.2;
Double_t KVRungeKutta::b51 = -11.0 / 54.0;
Double_t KVRungeKutta::b52 = 2.5;
Double_t KVRungeKutta::b53 = -70.0 / 27.0;
Double_t KVRungeKutta::b54 = 35.0 / 27.0;
Double_t KVRungeKutta::b61 = 1631.0 / 55296.0;
Double_t KVRungeKutta::b62 = 175.0 / 512.0;
Double_t KVRungeKutta::b63 = 575.0 / 13824.0;
Double_t KVRungeKutta::b64 = 44275.0 / 110592.0;
Double_t KVRungeKutta::b65 = 253.0 / 4096.0;
Double_t KVRungeKutta::c1 = 37.0 / 378.0;
Double_t KVRungeKutta::c3 = 250.0 / 621.0;
Double_t KVRungeKutta::c4 = 125.0 / 594.0;
Double_t KVRungeKutta::c6 = 512.0 / 1771.0;
Double_t KVRungeKutta::dc5 = -277.00 / 14336.0;

KVRungeKutta::KVRungeKutta(Int_t N, Double_t PREC, Double_t MINSTEP)
   : KVBase("RK4NR", "Runge-Kutta ODE integrator with adaptive step-size control"),
     nvar(N), eps(PREC), hmin(MINSTEP)
{
   // Set up integrator for N independent variables
   // PREC = required precision (default: 1.e-8)
   // MINSTEP = minimum allowed stepsize (default: 0)

   y = new Double_t [N];
   yscal = new Double_t [N];
   dydx = new Double_t [N];
   yerr = new Double_t [N];
   yout = new Double_t [N];
   ytemp = new Double_t [N];
   ak2 = new Double_t [N];
   ak3 = new Double_t [N];
   ak4 = new Double_t [N];
   ak5 = new Double_t [N];
   ak6 = new Double_t [N];

   dc1 = c1 - 2825.0 / 27648.0;
   dc3 = c3 - 18575.0 / 48384.0;
   dc4 = c4 - 13525.0 / 55296.0;
   dc6 = c6 - 0.25;

   fInitialDeriv = kFALSE;
}

KVRungeKutta::~KVRungeKutta()
{
   // Destructor
   delete [] y;
   delete [] yscal;
   delete [] dydx;
   delete [] yerr;
   delete [] yout;
   delete [] ytemp;
   delete [] ak2;
   delete [] ak3;
   delete [] ak4;
   delete [] ak5;
   delete [] ak6;
}

void KVRungeKutta::Integrate(Double_t* ystart, Double_t x1, Double_t x2, Double_t h1)
{
   // Runge-Kutta driver with adaptive stepsize control.
   // Integrate nvar starting values ystart[0,...,nvar-1] from x1 to x2 with accuracy eps.
   // h1 should be set as a guessed first stepsize.
   // after call, GetNGoodSteps() and GetNBadSteps() give the number of good
   // and bad (but retried and fixed) steps taken,
   // and ystart values are replaced by values at the end of the integration interval.

   x = x1;
   Double_t h = TMath::Sign(h1, x2 - x1);
   nok = nbad = 0;
   fOK = kTRUE;

   for (int i = 0; i < nvar; i++) y[i] = ystart[i];

   for (int nstp = 1; nstp <= MAXSTP; nstp++) { //Take at most MAXSTP steps.

      // calculate derivatives before performing step
      fInitialDeriv = kTRUE;
      CalcDerivs(x, y, dydx);
      fInitialDeriv = kFALSE;

      for (Int_t i = 0; i < nvar; i++)
         yscal[i] = TMath::Abs(y[i]) + TMath::Abs(dydx[i] * h) + TINY;

      if ((x + h - x2) * (x + h - x1) > 0.0) h = x2 - x;

      rkqs(h);
      if (!fOK) {
         Error("Integrate", "integration stopped at x=%g", x);
         return;
      }

      if (hdid == h) ++nok;
      else ++nbad;
      if ((x - x2) * (x2 - x1) >= 0.0) {
         for (int i = 0; i < nvar; i++) ystart[i] = y[i];
         return;
      }

      if (TMath::Abs(hnext) <= hmin) {
         Error("Integrate",  "Step size %g too small", hnext);
         Error("Integrate", "integration stopped at x=%g", x);
         return;
      }
      h = hnext;
   }
   Error("Integrate", "Too many steps");
   Error("Integrate", "integration stopped at x=%g", x);
}

void KVRungeKutta::rkqs(Double_t htry)
{
   // Fifth-order Runge-Kutta step with monitoring of local truncation error to ensure accuracy and
   // adjust stepsize. Input are the dependent variable vector y[1..n] and its derivative dydx[1..n]
   // at the starting value of the independent variable x. Also input are the stepsize to be attempted
   // htry, the required accuracy eps, and the vector yscal[1..n] against which the error is
   // scaled. On output, y and x are replaced by their new values, hdid is the stepsize that was
   // actually accomplished, and hnext is the estimated next stepsize. derivs is the user-supplied
   // routine that computes the right-hand side derivatives.

   Double_t errmax, htemp, xnew;

   Double_t h = htry;
   for (;;) {
      rkck(h);
      errmax = 0.0;
      for (int i = 0; i < nvar; i++) errmax = TMath::Max(errmax, TMath::Abs(yerr[i] / yscal[i]));
      errmax /= eps;
      if (errmax <= 1.0) break;
      htemp = SAFETY * h * TMath::Power(errmax, PSHRNK);
      h = (h >= 0.0 ? TMath::Max(htemp, 0.1 * h) : TMath::Min(htemp, 0.1 * h));
      xnew = x + h;
      if (xnew == x) {
         Error("rkqs", "stepsize underflow");
         fOK = kFALSE;
         return;
      }
   }
   if (errmax > ERRCON) hnext = SAFETY * h * TMath::Power(errmax, PGROW);
   else hnext = 5.0 * h;
   x += (hdid = h);
   for (int i = 0; i < nvar; i++) y[i] = yout[i];
}

void KVRungeKutta::rkck(Double_t h)
{
   // Given values for n variables y[1..n] and their derivatives dydx[1..n] known at x, use
   // the fifth-order Cash-Karp Runge-Kutta method to advance the solution over an interval h
   // and return the incremented variables as yout[1..n]. Also return an estimate of the local
   // truncation error in yout using the embedded fourth-order method. The user supplies the routine
   // derivs(x,y,dydx) , which returns derivatives dydx at x.

   for (int i = 0; i < nvar; i++)
      ytemp[i] = y[i] + b21 * h * dydx[i];
   CalcDerivs(x + a2 * h, ytemp, ak2);
   for (int i = 0; i < nvar; i++)
      ytemp[i] = y[i] + h * (b31 * dydx[i] + b32 * ak2[i]);
   CalcDerivs(x + a3 * h, ytemp, ak3);
   for (int i = 0; i < nvar; i++)
      ytemp[i] = y[i] + h * (b41 * dydx[i] + b42 * ak2[i] + b43 * ak3[i]);
   CalcDerivs(x + a4 * h, ytemp, ak4);
   for (int i = 0; i < nvar; i++)
      ytemp[i] = y[i] + h * (b51 * dydx[i] + b52 * ak2[i] + b53 * ak3[i] + b54 * ak4[i]);
   CalcDerivs(x + a5 * h, ytemp, ak5);
   for (int i = 0; i < nvar; i++)
      ytemp[i] = y[i] + h * (b61 * dydx[i] + b62 * ak2[i] + b63 * ak3[i] + b64 * ak4[i] + b65 * ak5[i]);
   CalcDerivs(x + a6 * h, ytemp, ak6);
   for (int i = 0; i < nvar; i++)
      yout[i] = y[i] + h * (c1 * dydx[i] + c3 * ak3[i] + c4 * ak4[i] + c6 * ak6[i]);
   for (int i = 0; i < nvar; i++)
      yerr[i] = h * (dc1 * dydx[i] + dc3 * ak3[i] + dc4 * ak4[i] + dc5 * ak5[i] + dc6 * ak6[i]);
}

/** \example ClassTraj.cpp
# Example of use of the KVRungeKutta class

Implementation of a classical trajectory calculation for heavy ion collisions

### Friction parameter
`alpha=240` gives maximum fusion X-section for 40Ca+60Ni@6MeV/u (bfus<=5.25fm: 866mb)

`alpha=750` gives 66mb of fusion for 84Kr(605 MeV)+238U
   - measured fusion-fission Xsec from PRL 33, 502 (1974): 55+/-15 mb

\include ClassTraj.h
*/
