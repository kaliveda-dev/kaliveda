//Created by KVClassFactory on Tue Jul 23 15:24:27 2019
//Author: John Frankland,,,

#include "KVSmoothIPDist.h"
#include "TMath.h"

#include <TH1.h>

ClassImp(KVSmoothIPDist)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSmoothIPDist</h2>
<h4>Realistic impact parameter distribution with smooth fall-off at large b</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Double_t smooth_pb(Double_t* x, Double_t* par)
{
   // Smooth impact parameter distribution (arbitrary normalisation par[0])
   // x = b
   // par[1] = b0
   // par[2] = delta_b
   // f(b) = [0] * x * [ 1 + exp((x - [1])/[2]) ]**-1

   if (par[0] == 0) return 0;

   if (par[2] == 0) {
      //sharp cutoff
      if (x[0] <= par[1]) return par[0] * x[0];
      return 0;
   }

   return par[0] * x[0] / (1. + TMath::Exp((x[0] - par[1]) / par[2]));
}

Double_t sigma_inel(Double_t* x, Double_t* p)
{
   // reaction cross-section [mb] as function of b0 and deltab
   // x=b0
   // p[0]=deltab

   if (p[0] == 0) {
      // sharp cut-off
      return TMath::Pi() * TMath::Power(x[0], 2) * 10.;
   }
   return -10 * TMath::TwoPi() * TMath::Power(p[0], 2) *
          TMath::DiLog(-TMath::Exp(x[0] / p[0]));
}

double ana_centrality(double* x, double* par)
{
   // Analytic expression for centrality
   //x=b
   //par(0)=b_0
   //par(1)=deltab

   double b = x[0];
   if (b <= 0) return 0;
   double b0 = par[0];
   if (b0 <= 0) return 0;
   double db = par[1];
   if (db == 0) {
      // sharp cut-off
      if (b > b0) return 1;
      return TMath::Power(b / b0, 2);
   }
   const double pi_sqr_over_6 = TMath::Power(TMath::Pi(), 2) / 6.;
   double exp_b_minus_b0_over_db = TMath::Exp((b - b0) / db);
   double db_sqr = db * db;
   double sigma = sigma_inel(&b0, &db) / 10.; // in fm**2
   double cb = -TMath::DiLog(-TMath::Exp(b0 / db)) - pi_sqr_over_6
               + (b * b - b0 * b0) / 2. / db_sqr
               - (b / db) * TMath::Log(1 + exp_b_minus_b0_over_db)
               - TMath::DiLog(-exp_b_minus_b0_over_db);
   cb *= TMath::TwoPi() * db_sqr / sigma;
   return cb;
}

KVSmoothIPDist::KVSmoothIPDist()
   : fHisto(nullptr),
     fIPdist("smooth_pb", smooth_pb, 0., 20., 3),
     fSigmaR("sigmaR", sigma_inel, 0., 20., 1),
     fCentrality("centrality", ana_centrality, 0., 20., 2)
{
   // By default initialize a distribution which is triangular (sharp cut-off) between b=0 and b=1
   fIPdist.SetParNames("Norm", "b_{0}", "#Delta b");
   fSigmaR.SetParName(0, "#Delta b");
   fCentrality.SetParNames("b_{0}", "#Delta b");
   fIPdist.SetParameter(0, TMath::TwoPi() * 10.); // default normalisation for dsig/db in mb/fm
   // sharp cut-off distribution from b=0 to b=1
   SetB0(1.0);
   SetDeltaB(0.0);
}

KVSmoothIPDist::KVSmoothIPDist(TH1* h)
   : fHisto(h),
     fIPdist(Form("%s_smooth_pb", h->GetName()), smooth_pb, 0., 50., 3),
     fSigmaR(Form("%s_sigmaR", h->GetName()), sigma_inel, 0., 50., 1),
     fCentrality(Form("%s_centrality", h->GetName()), ana_centrality, 0., 20., 2)
{
   // Fit the given impact parameter distribution

   fIPdist.SetParNames("Norm", "b_{0}", "#Delta b");
   fSigmaR.SetParName(0, "#Delta b");
   fCentrality.SetParNames("b_{0}", "#Delta b");
   FitIPDist(h);
}

void KVSmoothIPDist::FitIPDist(TH1* h)
{
   // Fit impact parameter distribution in histogram

   Double_t minb = h->GetXaxis()->GetBinLowEdge(1);
   Double_t maxb = h->GetXaxis()->GetBinUpEdge(h->GetXaxis()->GetNbins());
   Double_t maxy = h->GetMaximum();
   Double_t bmax = h->GetBinCenter(h->GetMaximumBin());
   fIPdist.SetParLimits(0, 1., maxy);
   fIPdist.SetParLimits(1, minb, maxb);
   fIPdist.SetParLimits(2, 1.e-6, maxb - minb);
   fIPdist.SetParameters(maxy / 10., bmax, 0.4);
   h->Fit(&fIPdist, "I");
   fSigmaR.SetParameter(0, GetDeltaB());
   fCentrality.SetParameter(0, GetB0());
   fCentrality.SetParameter(1, GetDeltaB());
}

Double_t KVSmoothIPDist::GetCrossSection() const
{
   // Returns calculated total reaction cross-section in [mb] using
   // current values of b0 and delta_b parameters

   return fSigmaR.Eval(GetB0());
}

Double_t KVSmoothIPDist::GetCrossSectionPerEvent() const
{
   // Get cross-section per event in [mb] for last fit

   return fHisto->GetEntries() ? GetCrossSection() / fHisto->GetEntries() : 0.;
}

void KVSmoothIPDist::SetDeltaB_WithConstantCrossSection(Double_t deltab, Double_t sigmaR)
{
   // Change deltab and modify b0 in order to have total cross section sigmaR
   // If sigmaR=0 (default), keep current total cross section

   Double_t sigma = sigmaR > 0 ? sigmaR : const_cast<KVSmoothIPDist*>(this)->GetCrossSection();
   SetDeltaB(deltab);
   Double_t b0 = fSigmaR.GetX(sigma);
   SetB0(b0);
}

Double_t KVSmoothIPDist::Calculate_b(Double_t centrality) const
{
   // calculate b for given value of centrality using current parameters

   return fCentrality.GetX(centrality);
}
