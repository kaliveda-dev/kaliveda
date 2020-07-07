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

   Impact parameter estimation neglecting fluctuations using sharp cut-off approximation
    */
   class cavata_prescription : public KVBase {
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
      cavata_prescription(const cavata_prescription&) = delete; // copying is not possible
#else
      KVImpactParameter(const KVImpactParameter&) : KVBase() {}; // copying is not possible
#endif
   public:
      cavata_prescription()
      {
         fData = nullptr;
         fIPScale = nullptr;
         fXSecScale = nullptr;
         fObsTransform = nullptr;
         fObsTransformXSec = nullptr;
         Bmax = 1.0;
      }
      cavata_prescription(TH1*, Option_t* evol = "D");
      virtual ~cavata_prescription();

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
      double GetMeanBForSCA(double bmin, double bmax) const;
      double GetSigmaBForSCA(double bmin, double bmax) const;

      ClassDef(cavata_prescription, 2) //Impact parameter analysis tools
   };
}
#endif
