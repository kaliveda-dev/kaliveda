//Created by KVClassFactory on Sun Oct 27 09:22:29 2019
//Author: John Frankland,,,

#include "KVINDRACsITotLightReconstructor.h"

ClassImp(KVINDRACsITotLightReconstructor)

void KVINDRACsITotLightReconstructor::Calculate() const
{
   //Private method
   //This function is called by GetLumiereTotale and GetTauZero, the return value depends
   //on the value of 'mode' (kLumiere or kTau).
   //The 'TauZero' in the code is the variable 'x'.
   //In either case, if problems are encountered in the calculation, the value returned
   //is zero and fLumTotStatus should give information on the problem (GetStatusLumiere).

   //pedestal-corrected fast and slow components from raw data
   Double_t rap_corr = fFast->GetValue();
   Double_t len_corr = fSlow->GetValue();

   if (rap_corr < 0 || len_corr < 0) {
      fLumTotStatus = NEGATIVE_PEDESTAL_CORRECTED_VALUE;
      const_cast<KVINDRACsITotLightReconstructor*>(this)->SetValue(0);
      return;
   }
   Double_t x;
   Double_t x2;
   Double_t x3;
   Double_t bx;

   Double_t dx;
   Double_t rp = (len_corr / rap_corr) * (c1 / c2);
   if (rp >= 1 && rap_corr <= 10) {
      //calculation without correction for PM gain
      x = 600;
      x2 = TMath::Exp(-tau1 / x);
      x3 = TMath::Exp(-tau2 / x);
      bx = x2 - x3;
      fLumTotStatus = NO_GAIN_CORRECTION;
   }
   else {
      //iterative calculation of total light output
      fLumTotStatus = CALCULATED_WITH_GAIN_CORRECTION;
      x = p0 + p1 * rp;
      x = x - 100;
      Double_t at = 1. - TMath::Exp(-tau0 / tau);
      at = tau * at;
      Int_t niter = 0;
      do {
         niter++;
         Double_t xx = x * x;
         Double_t x1 = TMath::Exp(-tau0 / x);
         x2 = TMath::Exp(-tau1 / x);
         x3 = TMath::Exp(-tau2 / x);
         Double_t ax = 1. - x1;
         ax = ax - at / x;
         bx = x2 - x3;
         Double_t fx = bx / ax - rp;
         Double_t apx = -tau0 * x1;
         apx = (apx + at) / xx;
         Double_t bpx = tau1 * x2 - tau2 * x3;
         bpx = bpx / xx;
         Double_t fpx = (bpx * ax - apx * bx) / (ax * ax);
         dx = -fx / fpx;
         x = x + dx;
      }
      while (TMath::Abs(dx / x) >= eps && niter <= 50);

      if (niter > 50 || x <= tau) {
         fLumTotStatus = CALCULATION_NOT_CONVERGED;
         const_cast<KVINDRACsITotLightReconstructor*>(this)->SetValue(0.0);
         return;
      }
   }

   const_cast<KVINDRACsITotLightReconstructor*>(this)->SetValue(len_corr * (x - tau) / (x * bx));
}

//_______________________________________________________________________________________________

UInt_t KVINDRACsITotLightReconstructor::GetStatusLumiere() const
{
   //Status concerning the calculation of the total light output for
   //this CsI detector. The possible values are
   //(you can use the named constants for comparisons):
   //
   //~~~~~~~~~~~~~~~~~~~~~~~~~
   // NOT_CALCULATED                    = 0
   // CALCULATED_WITH_GAIN_CORRECTION   = 1
   // NO_GAIN_CORRECTION                = 4
   // NEGATIVE_PEDESTAL_CORRECTED_VALUE = 15
   // CALCULATION_NOT_CONVERGED         = 12
   //~~~~~~~~~~~~~~~~~~~~~~~~~

   return fLumTotStatus;
}

//_______________________________________________________________________________________________

Bool_t KVINDRACsITotLightReconstructor::LightIsGood() const
{
   //Returns kTRUE if total light output has been successfully calculated
   //for the CsI, either with (GetStatusLumiere()=CALCULATED_WITH_GAIN_CORRECTION)
   //or without (GetStatusLumiere()=NO_GAIN_CORRECTION) iterative gain correction.

   if (GetStatusLumiere() == CALCULATED_WITH_GAIN_CORRECTION
         || GetStatusLumiere() == NO_GAIN_CORRECTION)
      return kTRUE;

   return kFALSE;
}

Int_t KVINDRACsITotLightReconstructor::GetStatus(const TString& what) const
{
   // Get status of last calculation of total light:
   //
   //~~~~~~~~~~~{.cpp}
   //what = "LightIsGood"   :  returns either 1 (true) or 0 (false), as given by LightIsGood()
   //
   //what = "StatusLumiere" :  returns value of GetStatusLumiere()
   //~~~~~~~~~~~

   if (what == "LightIsGood") return LightIsGood();
   else if (what == "StatusLumiere") return GetStatusLumiere();
   return -1;
}
