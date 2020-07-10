//Created by KVClassFactory on Fri Jan 15 18:14:06 2010
//Author: John Frankland,,,

#ifndef __KVIMPACTPARAMETER_H
#define __KVIMPACTPARAMETER_H
#include "KVBase.h"
#include "TH1.h"
#include "TString.h"
#include "TGraph.h"
#include "TF1.h"
#include "KVHistoManipulator.h"
#include <vector>

namespace KVImpactParameters {
   /**
   \class cavata_prescription
   \ingroup ImpactParameters

   \brief Impact parameter estimation neglecting using sharp cut-off approximation

   This class implements impact parameter estimation using the method of Cavata et al., *Phys. Rev.* **C** 72, 1460(1990).
   Given an observable \f$X\f$ which is assumed monotonically
   decreasing with impact parameter, the upper limit \f$b\leq b_0\f$ corresponding to a cut
   \f$X\geq X_0\f$ is estimated by equating the fraction of the total measured cross-section retained
   by the cut with the ratio of the cross-sections for the two triangular impact parameter
   distributions \f$b\leq b_0\f$ and \f$b\leq b_{max}\f$, where \f$\sigma=\pi b_{max}^2\f$ is the total
   measured cross-section for *all* reactions:
   \f[
   \frac{b_0}{b_{max}}=\left[\int_{X_0}^{\infty} P(X) dX \right]^{\frac{1}{2}}
   \f]
   where \f$P(X)\f$ is the probability distribution of \f$X\f$ for all measured reactions.

   This relationship is strictly true if fluctuations of \f$X\f$ for a fixed \f$b\f$ are
   negligible, which is never the case in heavy-ion collisions at intermediate energies.
   The Cavata prescription implies a one-to-one correspondence between each value of \f$X\f$
   i.e. each event and a unique value of \f$b\f$, which is clearly not true. Although for
   peripheral collisions this prescription gives a reasonable estimation of the mean value of
   impact parameter associated with selected events, it greatly underestimates the widths of
   the actual distributions of impact parameter, and the more "central" the event selection the
   more Cavata underestimates the mean impact parameters, giving a false impression that
   higher and higher cuts in \f$X\f$ produce event samples with smaller and smaller \f$\langle b\rangle\f$.

   \sa bayesian_estimator
    */
   class cavata_prescription : public KVBase {
      TH1* fData; ///< histogram containing distribution of ip-related observable
      TString fEvol; ///< how the observable evolves with b
      TGraph* fIPScale; ///< derived relation between observable and impact-parameter
      TGraph* fXSecScale; ///< derived relation between observable and cross-section
      TF1* fObsTransform; ///< function for transforming observable into impact parameter
      TF1* fObsTransformXSec; ///< function for transforming observable into cross-section
      KVHistoManipulator HM; ///< for scaling transormations of histograms, graphs, etc.
      Double_t Bmax; ///< maximum of ip scale
      Double_t Smax; ///< maximum of cross-section scale

      void make_scale(Int_t npoints);
#ifndef WITH_CPP11
      KVImpactParameter(const KVImpactParameter&) : KVBase() {}; // copying is not possible
#endif
   public:
#ifdef WITH_CPP11
      cavata_prescription(const cavata_prescription&) = delete; // copying is not possible
#endif
      cavata_prescription()
      {
         fData = nullptr;
         fIPScale = nullptr;
         fXSecScale = nullptr;
         fObsTransform = nullptr;
         fObsTransformXSec = nullptr;
         Bmax = 1.0;
      }
      ////////////////////////////////////////////////////////
      // Constructor for impact parameter estimator
      //
      // \param[in] h     Histogram containing distribution of the observable used to calculate impact parameter.
      // \param[in] evol  Determine whether observable is monotonically decreasing ("D") or increasing ("C")
      cavata_prescription(TH1* h, Option_t* evol = "D");
      virtual ~cavata_prescription();

      void MakeScale(Int_t npoints = 100, Double_t bmax = 1.0);
      void MakeAbsoluteScale(Int_t npoints = 100, Double_t bmax = 1.0);
      TGraph* GetScale() const
      {
         // \return pointer to TGraph showing relationship between the observable
         // histogrammed in fData and the impact parameter.
         // \warning only call after calling MakeScale() or MakeAbsoluteScale().
         return fIPScale;
      }
      TGraph* GetXSecScale() const
      {
         // \return pointer to TGraph showing relationship between the observable
         // histogrammed in fData and the cross section.
         // \warning only call after calling MakeScale() or MakeAbsoluteScale().
         return fXSecScale;
      }
      Double_t BTransform(Double_t*, Double_t*);
      Double_t XTransform(Double_t*, Double_t*);
      TF1* GetTransFunc() const
      {
         // \return pointer to function giving b for any value of observable
         return fObsTransform;
      }
      TF1* GetXSecTransFunc() const
      {
         // \return pointer to function giving cross section for any value of observable
         return fObsTransformXSec;
      }
      Double_t GetImpactParameter(Double_t obs)
      {
         // \param[in] obs value of observable
         // \returns impact parameter for given value of the observable.
         return fObsTransform->Eval(obs);
      }
      Double_t GetCrossSection(Double_t obs)
      {
         // \param[in] obs value of observable
         // \returns cross section for given value of the observable.
         return fObsTransformXSec->Eval(obs);
      }
      Double_t GetObservable(Double_t b)
      {
         // \param[in] b impact parameter
         // \returns corresponding value of observable
         return fObsTransform->GetX(b);
      }
      Double_t GetObservableXSec(Double_t sigma)
      {
         // \param[in] sigma cross-section
         // \returns corresponding value of observable
         return fObsTransformXSec->GetX(sigma);
      }
      TH1* GetIPDistribution(TH1* obs, Int_t nbinx = 100, Option_t* norm = "");
      TGraph* GetIPEvolution(TH2* obscor, TString moment, TString axis = "Y");
      TH1* GetXSecDistribution(TH1* obs, Int_t nbinx = 100, Option_t* norm = "");
      TGraph* GetXSecEvolution(TH2* obscor, TString moment, TString axis = "Y");

      static Double_t GetXSecFromIP(Double_t bmax)
      {
         // Utility function
         // \param[in] bmax maximum value of impact parameter
         // \returns cross-section in [mb] corresponding to maximum impact parameter in [fm] using sharp cut-off approximation
         return 10.*TMath::Pi() * pow(bmax, 2);
      }
      static Double_t GetIPFromXSec(Double_t xsec)
      {
         // Utility function
         // \param[in] xsec cross-section
         // \returns maximum impact parameter in [fm] corresponding to cross-section in [mb] using sharp cut-off approximation
         return pow(xsec / 10. / TMath::Pi(), 0.5);
      }
      std::vector<Double_t> SliceXSec(Int_t nslices, Double_t totXsec);
      double GetMeanBForSCA(double bmin, double bmax) const;
      double GetSigmaBForSCA(double bmin, double bmax) const;

      ClassDef(cavata_prescription, 2) //Impact parameter analysis tools
   };
}
#endif
