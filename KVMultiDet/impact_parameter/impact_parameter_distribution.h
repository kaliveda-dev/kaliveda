//Created by KVClassFactory on Tue Jul 23 15:24:27 2019
//Author: John Frankland,,,

#ifndef __KVSMOOTHIPDIST_H
#define __KVSMOOTHIPDIST_H

#include "KVBase.h"
#include "TF1.h"

namespace KVImpactParameters {
   /**
   \class impact_parameter_distribution
   \ingroup ImpactParameters
   \brief Class implementing parametrizable impact parameter distributions

   Unbiased impact parameter distributions are determined by simple geometric considerations,
   \f[
   P(b)\,\mathrm{d}b=\frac{2\pi b}{\sigma_{R}}\,\mathrm{d}b
   \f]
   The sharp cut-off approximation assumes that the measured reaction cross-section \f$\sigma_R\f$
   for any selected data has a triangular distribution up to the impact parameter \f$b_{\mathrm{max}}\f$
   where \f$\sigma_R=\pi b^2_{\mathrm{max}}\f$ and the centrality is given in this case by
   \f$c_b=(b/b_{\mathrm{max}})^2\f$.

   More generally, simulations of reactions detected by large multidetector arrays operating with
   a minimum bias multiplicity trigger indicate that the impact parameter distribution for the
   measured collisions is triangular with a more gradual fall-off at the largest impact parameters,
   and can be well-described by the distribution
   \f[
   P(b)=\frac{2\pi b}{\sigma_{R}}\left[1+\exp\left(\frac{b-b_{0}}{\Delta b}\right)\right]^{-1}
   \f]

   In this case the total reaction cross-section is given by
   \f[
   \sigma_{R}=-2\pi(\Delta b)^{2}\mathrm{Li}_{2}\left(-\exp\left(\frac{b_{0}}{\Delta b}\right)\right)
   \f]
   and the centrality is related to impact parameter by
   \f[
   c_{b}=\frac{2\pi(\Delta b)^{2}}{\sigma_{R}}\left[\mathrm{-Li}_{2}\left(-\exp\left(\frac{b_{0}}{\Delta b}\right)\right)-\frac{\pi^{2}}{6}+\frac{(b^{2}-b_{0}^{2})}{2(\Delta b)^{2}}-\frac{b}{\Delta b}\ln\left(1+\exp\left((b-b_{0})/\Delta b\right)\right)-\mathrm{Li}_{2}\left(-\mathrm{e}^{(b-b_{0})/\Delta b}\right)\right]
   \f]
    */
   class impact_parameter_distribution : public KVBase {

   private:
      TH1*  fHisto;///<! last fitted histogram
      TF1 fIPdist;///< impact parameter distribution
      TF1 fSigmaR;///< total reaction cross section
      TF1 fCentrality;///< centrality as function of impact parameter

   public:
      impact_parameter_distribution();
      impact_parameter_distribution(TH1*);
      virtual ~impact_parameter_distribution() {}

      void FitIPDist(TH1*);

      Double_t GetCrossSection() const;
      Double_t GetB0() const
      {
         // \returns current value of \f$b_0\f$
         return fIPdist.GetParameter(1);
      }
      Double_t GetDeltaB() const
      {
         // \returns current value of \f$\Delta b\f$
         return fIPdist.GetParameter(2);
      }
      Double_t GetCrossSectionPerEvent() const;
      void SetB0(Double_t x)
      {
         // \param[in] x value of \f$b_0\f$
         fIPdist.SetParameter(1, x);
         fCentrality.SetParameter(0, x);
      }
      void SetDeltaB(Double_t x)
      {
         // \param[in] x value of \f$\Delta b\f$
         fIPdist.SetParameter(2, x);
         fSigmaR.SetParameter(0, x);
         fCentrality.SetParameter(1, x);
      }
      void SetDeltaB_WithConstantCrossSection(Double_t deltab, Double_t sigmaR = 0);
      TF1& GetIPDist()
      {
         // \returns reference to TF1 implementing impact parameter distribution
         // (actually the differential cross-section in [mb fm\f${}^{-1}\f$])
         return fIPdist;
      }
      void Draw(Option_t* opt = "")
      {
         // Draw the impact parameter distribution (differential cross-section in [mb fm\f${}^{-1}\f$])
         // \param[in] opt Usual drawing option, passed to TF1::Draw()
         fIPdist.Draw(opt);
      }
      const TF1& GetSigmaR()
      {
         // \returns reference to TF1 implementing total cross-section in [mb]
         // as a function of \f$b_0\f$.
         return fSigmaR;
      }
      const TF1& GetCentrality()
      {
         // \returns reference to TF1 implementing centrality
         // as a function of \f$b\f$.
         return fCentrality;
      }
      Double_t Calculate_b(Double_t centrality) const;

      void NormalizeIPDistToCrossSection()
      {
         // Changes impact parameter distribution from differential cross-section to probability distribution \f$P(b)\f$

         fIPdist.SetParameter(0, 1. / GetCrossSection());
      }

      Double_t GetRelativeCrossSection(double b) const
      {
         // \return differential cross-section relative to \f$2\pi b\f$
         if (b > 0) return fIPdist.Eval(b) / (b * fIPdist.GetParameter(0));
         return 0;
      }

      ClassDef(impact_parameter_distribution, 1) //Realistic impact parameter distribution with smooth fall-off at large b
   };
}
#endif
