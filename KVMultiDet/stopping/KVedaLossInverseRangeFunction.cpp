//Created by KVClassFactory on Tue Jan 26 14:52:11 2016
//Author: John Frankland,,,

#include "KVedaLossInverseRangeFunction.h"
#include "TF1.h"
#include "TMath.h"

#define LOG(x) TMath::Log(x)
#define EXP(x) TMath::Exp(x)

ClassImp(KVedaLossInverseRangeFunction)

KVedaLossInverseRangeFunction::KVedaLossInverseRangeFunction(TF1* range_func,
      Int_t A,
      Double_t riso,
      Int_t ninter)
{
   // Initialise from VEDALOSS range function object
   //
   // \param A is mass of ion used in call to KVedaLossMaterial::GetRangeFunction
   // \param riso is value corresponding to previous call to KVedaLossMaterial::GetRangeFunction
   // \param ninter number of points in graph used for interpolation
   //
   // We make a graph from 0.01 MeV/nucleon to EMAX.
   //
   // We divide the calculated range by the current value of riso,
   // thereby effectively ensuring that we always calculate range
   // values corresponding to riso=1

   Double_t emin, emax, estep;
   range_func->GetRange(emin, emax);

   TGraph e_vs_range;
   emin = 0.01;
   emax /= A;

   Double_t logriso = LOG(riso);

   estep = (LOG(emax) - LOG(emin)) / (ninter - 1.);

   for (int i = 0; i < ninter; ++i) {
      double loge = LOG(emin) + i * estep;
      // we divide the calculated range by the current value of riso,
      // thereby effectively ensuring that we always calculate range
      // values corresponding to riso=1
      double R = LOG(range_func->Eval(EXP(loge) * A)) - logriso;
      e_vs_range.SetPoint(i, R, loge);
   }

#ifdef USING_ROOT5
   fInterpol = new TSpline3(range_func->GetTitle(), &e_vs_range);
#else
   fInterpol.reset(new TSpline3(range_func->GetTitle(), &e_vs_range));
#endif
}

Double_t KVedaLossInverseRangeFunction::GetEnergyPerNucleon(Double_t range, Double_t riso)
{
   // \param range of ion in \f$g/cm^2\f$
   // \param riso takes into account any change in ion mass and/or material mass
   // \returns corresponding energy in MeV/nucleon

   if (range <= 0) return 0;
   return EXP(fInterpol->Eval(LOG(range) - LOG(riso)));
}
