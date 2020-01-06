//Created by KVClassFactory on Fri Feb  8 09:58:44 2013
//Author: dgruyer

#ifndef __KVLIGHTENERGYCSIFULL_H
#define __KVLIGHTENERGYCSIFULL_H

#include "KVLightEnergyCsI.h"
#include "KVIonRangeTableMaterial.h"

class KVLightEnergyCsIFull : public KVLightEnergyCsI {

   Double_t gamma(double z, double a, double e);
   Double_t gamma_ziegler(double z, double a, double e);
   Double_t gamma_hbg(double z, double a, double e);
   Double_t dLightIntegral(double* x, double* par);
   Double_t dLightIntegralApprox(double* x, double* par);
   Double_t sp_e(double z, double a, double e);
   Double_t sp_n(double z, double a, double e);
   Double_t GetDeltaFraction(Double_t beta, Double_t beta_delta);
   Double_t GetLight(double* x, double* par);
   Double_t GetLightApprox(double* x, double* par);
   Double_t GetLightApproxSilicon(double* x, double* par);

   void init();

protected:
   Int_t    fLightFormula;        //!light formula (see NIMa of Marian)
   Double_t fZmed;                //!Z of detector material
   Double_t fAmed;                //!A of detector material (CsI)
   TF1* fDlight;                  //!function to integrate to get fLight

public:
   enum LightFormula {
      kExact,
      kApproxIntegral,
      kApprox,
      kApproxSilicon
   };

   KVLightEnergyCsIFull();
   virtual ~KVLightEnergyCsIFull()
   {
      SafeDelete(fDlight);
   }

   void SetLightFormula(Int_t form)
   {
      fLightFormula = form;
      init();
   }
   void SetLightFormula(const TString&);
   Int_t GetLightFormula()
   {
      return fLightFormula;
   }

   virtual void Print(Option_t* opt = "") const;
   void SetOptions(const KVNameValueList& opt);

   ClassDef(KVLightEnergyCsIFull, 1) //Light-energy calibration for CsI detectors using the full expression of Marian Parlog
};

#endif
