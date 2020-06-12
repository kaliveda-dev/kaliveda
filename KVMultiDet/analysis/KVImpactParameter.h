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

/**
\class KVImpactParameter
\brief Impact parameter analysis tools
\ingroup Analysis

Use a histogram containing the distribution of some observable strongly-correlated with
the impact parameter in order to transform distributions of said observable into
impact parameter distributions (using the method of C. Cavata <I>et al.</I>, <I>Phys. Rev.</I> <B>C42</B>, 1760 (1990)), and to calculate the evolution of other quantities as a function
of the impact parameter.<br>
To use, start with a pointer to a 1-D histogram of the observable, TH1* data:<br>
<code>
KVImpactParameter ip(data);
ip.MakeScale(npoints, bmax);
</code>
The <a href="#KVImpactParameter:MakeScale">MakeScale</a> method calculates the relationship between the observable and the
impact parameter, using
END_HTML
BEGIN_LATEX
\hat{b}^{2} = \frac{\int^{\infty}_{x=X} Y(x) dx}{\int_{0}^{\infty} Y(x) dx}
END_LATEX
BEGIN_HTML
To obtain the impact parameter distribution for some selection of events,
you need the distribution of the observable for the selection, TH1* obs_sel,
and then use:<br>
<code>
TH1* ip_dist_sel = ip.GetIPDistribution(obs_sel);
</code>
To obtain the impact-parameter evolution of some quantity, take a TH2* obscor, containing
the bidimensional plot of the quantity as a function of the observable, and then use<br>
<code>
TGraph* ip_evol = ip.GetIPEvolution(obscor, "GetMean");
</code>
e.g. to have the mean value of the quantity as a function of impact parameter.
*/

class KVImpactParameter : public KVBase {
   TH1* fData; // histogram containing distribution of ip-related observable
   TString fEvol; // how the observable evolves with b
   TGraph* fIPScale; // derived relation between observable and impact-parameter
   TGraph* fXSecScale; // derived relation between observable and cross-section
   TF1* fObsTransform; // function for transforming observable into impact parameter
   TF1* fObsTransformXSec; // function for transforming observable into cross-section
   KVHistoManipulator HM;
   Double_t Bmax; // maximum of ip scale
   Double_t Smax; // maximum of cross-section scale

   void make_scale(Int_t npoints);
#ifdef WITH_CPP11
   KVImpactParameter(const KVImpactParameter&) = delete; // copying is not possible
#else
   KVImpactParameter(const KVImpactParameter&) : KVBase() {}; // copying is not possible
#endif
public:
   KVImpactParameter()
   {
      fData = nullptr;
      fIPScale = nullptr;
      fXSecScale = nullptr;
      fObsTransform = nullptr;
      fObsTransformXSec = nullptr;
      Bmax = 1.0;
   }
   KVImpactParameter(TH1*, Option_t* evol = "D");
   virtual ~KVImpactParameter();

   void MakeScale(Int_t npoints = 100, Double_t bmax = 1.0);
   void MakeAbsoluteScale(Int_t npoints = 100, Double_t bmax = 1.0);
   TGraph* GetScale() const
   {
      // Return pointer to TGraph showing relationship between the observable
      // histogrammed in fData and the impact parameter.
      // Call after MakeScale.
      return fIPScale;
   };
   TGraph* GetXSecScale() const
   {
      // Return pointer to TGraph showing relationship between the observable
      // histogrammed in fData and the cross section.
      // Call after MakeScale.
      return fXSecScale;
   };
   Double_t BTransform(Double_t*, Double_t*);
   Double_t XTransform(Double_t*, Double_t*);
   TF1* GetTransFunc() const
   {
      // return pointer to function giving b for any value of observable
      return fObsTransform;
   };
   TF1* GetXSecTransFunc() const
   {
      // return pointer to function giving cross section for any value of observable
      return fObsTransformXSec;
   };
   Double_t GetImpactParameter(Double_t obs)
   {
      // Calculate value of impact parameter for given value of the observable.
      return fObsTransform->Eval(obs);
   };
   Double_t GetCrossSection(Double_t obs)
   {
      // Calculate value of cross section for given value of the observable.
      return fObsTransformXSec->Eval(obs);
   };
   Double_t GetObservable(Double_t b)
   {
      // Calculate value of observable corresponding to given value of impact parameter.
      return fObsTransform->GetX(b);
   };
   Double_t GetObservableXSec(Double_t sigma)
   {
      // Calculate value of observable corresponding to given value of cross section.
      return fObsTransformXSec->GetX(sigma);
   };
   TH1* GetIPDistribution(TH1* obs, Int_t nbinx = 100, Option_t* norm = "");
   TGraph* GetIPEvolution(TH2* obscor, TString moment, TString axis = "Y");
   TH1* GetXSecDistribution(TH1* obs, Int_t nbinx = 100, Option_t* norm = "");
   TGraph* GetXSecEvolution(TH2* obscor, TString moment, TString axis = "Y");

   static Double_t GetXSecFromIP(Double_t bmax)
   {
      // static utility function
      // returns cross-section in [mb] corresponding to impact parameter in [fm]
      return 10.*TMath::Pi() * pow(bmax, 2);
   }
   static Double_t GetIPFromXSec(Double_t xsec)
   {
      // static utility function
      // returns impact parameter in [fm] corresponding to cross-section in [mb]
      return pow(xsec / 10. / TMath::Pi(), 0.5);
   }
   std::vector<Double_t> SliceXSec(Int_t nslices, Double_t totXsec);

   ClassDef(KVImpactParameter, 2) //Impact parameter analysis tools
};

#endif
