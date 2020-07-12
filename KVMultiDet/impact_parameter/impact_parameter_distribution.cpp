//Created by KVClassFactory on Tue Jul 23 15:24:27 2019
//Author: John Frankland,,,

#include "impact_parameter_distribution.h"
#include "TMath.h"

#include <TH1.h>

ClassImp(KVImpactParameters::impact_parameter_distribution)

namespace KVImpactParameters {

   /////////////////////////////////////////////////////////////////////////////////
   /// Smooth impact parameter distribution (with arbitrary normalisation par[0])
   /// \param[in] x[0] \f$b\f$
   /// \param[in] par[0] normalisation parameter \f$A\f$
   /// \param[in] par[1] \f$b0\f$
   /// \param[in] par[2] \f$\Delta b\f$
   /// \returns  \f$Ab\left[1+\exp\left(\frac{b-b_{0}}{\Delta b}\right)\right]^{-1}\f$
   Double_t smooth_pb(Double_t* x, Double_t* par)
   {
      if (par[0] == 0) return 0;

      if (par[2] == 0) {
         //sharp cutoff
         if (x[0] <= par[1]) return par[0] * x[0];
         return 0;
      }

      return par[0] * x[0] / (1. + TMath::Exp((x[0] - par[1]) / par[2]));
   }

   /////////////////////////////////////////////////////////////////////////////////
   /// Total reaction cross-section in [mb] as a function of \f$b_0\f$ and \f$\Delta b\f$
   /// \param[in] x[0] \f$b_0\f$
   /// \param[in] par[0] \f$\Delta b\f$
   /// \returns \f$\sigma_{R}=-2\pi(\Delta b)^{2}\mathrm{Li}_{2}\left(-\exp\left(\frac{b_{0}}{\Delta b}\right)\right)\f$
   Double_t sigma_inel(Double_t* x, Double_t* p)
   {
      if (p[0] == 0) {
         // sharp cut-off
         return TMath::Pi() * TMath::Power(x[0], 2) * 10.;
      }
      return -10 * TMath::TwoPi() * TMath::Power(p[0], 2) *
             TMath::DiLog(-TMath::Exp(x[0] / p[0]));
   }

   /////////////////////////////////////////////////////////////////////////////////
   /// Centrality \f$c_b\f$ as a function of impact parameter \f$b\f$
   /// \param[in] x[0] \f$b\f$
   /// \param[in] par[0] \f$b_0\f$
   /// \param[in] par[1] \f$\Delta b\f$
   /// \returns \f$c_{b}=\frac{2\pi(\Delta b)^{2}}{\sigma_{R}}\left[\mathrm{-Li}_{2}\left(-\exp\left(\frac{b_{0}}{\Delta b}\right)\right)-\frac{\pi^{2}}{6}+\frac{(b^{2}-b_{0}^{2})}{2(\Delta b)^{2}}-\frac{b}{\Delta b}\ln\left(1+\exp\left((b-b_{0})/\Delta b\right)\right)-\mathrm{Li}_{2}\left(-\mathrm{e}^{(b-b_{0})/\Delta b}\right)\right]\f$
   double ana_centrality(double* x, double* par)
   {
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

   impact_parameter_distribution::impact_parameter_distribution()
      : fHisto(nullptr),
        fIPdist("smooth_pb", smooth_pb, 0., 20., 3),
        fSigmaR("sigmaR", sigma_inel, 0., 20., 1),
        fCentrality("centrality", ana_centrality, 0., 20., 2)
   {
      // Initialize a default distribution which is triangular (sharp cut-off approximation) with
      // \f$b_0=1\f$ and \f$\Delta b=0\f$.
      fIPdist.SetParNames("Norm", "b_{0}", "#Delta b");
      fSigmaR.SetParName(0, "#Delta b");
      fCentrality.SetParNames("b_{0}", "#Delta b");
      fIPdist.SetParameter(0, TMath::TwoPi() * 10.); // default normalisation for dsig/db in mb/fm
      // sharp cut-off distribution from b=0 to b=1
      SetB0(1.0);
      SetDeltaB(0.0);
   }

   impact_parameter_distribution::impact_parameter_distribution(TH1* h)
      : fHisto(h),
        fIPdist(Form("%s_smooth_pb", h->GetName()), smooth_pb, 0., 50., 3),
        fSigmaR(Form("%s_sigmaR", h->GetName()), sigma_inel, 0., 50., 1),
        fCentrality(Form("%s_centrality", h->GetName()), ana_centrality, 0., 20., 2)
   {
      // Fit \f$P(b)\f$ to the distribution in the histogram
      // \param[in] h histogram containing impact parameter distribution

      fIPdist.SetParNames("Norm", "b_{0}", "#Delta b");
      fSigmaR.SetParName(0, "#Delta b");
      fCentrality.SetParNames("b_{0}", "#Delta b");
      FitIPDist(h);
   }

   void impact_parameter_distribution::FitIPDist(TH1* h)
   {
      // Fit impact parameter distribution given in histogram.
      // \param[in] h histogram containing impact parameter distribution

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

   Double_t impact_parameter_distribution::GetCrossSection() const
   {
      // \returns Total reaction cross-section \f$\sigma_R\f$ in [mb] using
      // current values of \f$b_0\f$ and \f$\Delta b\f$.

      return fSigmaR.Eval(GetB0());
   }

   Double_t impact_parameter_distribution::GetCrossSectionPerEvent() const
   {
      // \returns Cross-section per event in [mb] for last fitted histogram. This is the total reaction
      // cross-section calculated using GetCrossSection() divided by the number of entries in the histogram.

      return fHisto->GetEntries() ? GetCrossSection() / fHisto->GetEntries() : 0.;
   }

   void impact_parameter_distribution::SetDeltaB_WithConstantCrossSection(Double_t deltab, Double_t sigmaR)
   {
      // Changes \f$\Delta b\f$ and \f$b_0\f$ for a given total cross-section.
      // \param[in] deltab new value of \f$\Delta b\f$
      // \param[in] sigmaR required total cross-section \f$\sigma_R\f$. If sigmaR=0 (default),
      //keep current total cross section

      Double_t sigma = sigmaR > 0 ? sigmaR : const_cast<impact_parameter_distribution*>(this)->GetCrossSection();
      SetDeltaB(deltab);
      Double_t b0 = fSigmaR.GetX(sigma);
      SetB0(b0);
   }

   Double_t impact_parameter_distribution::Calculate_b(Double_t centrality) const
   {
      // \returns value of impact parameter \f$b\f$ for given value of centrality \f$c_b\f$ using current parameters
      // \param[in] centrality value of \f$c_b\f$

      return fCentrality.GetX(centrality);
   }
}
