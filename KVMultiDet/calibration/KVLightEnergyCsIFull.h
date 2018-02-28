//Created by KVClassFactory on Fri Feb  8 09:58:44 2013
//Author: dgruyer

#ifndef __KVLIGHTENERGYCSIFULL_H
#define __KVLIGHTENERGYCSIFULL_H

#include "KVCalibrator.h"
#include "KVIonRangeTableMaterial.h"
//#include "TF1.h"

class KVLightEnergyCsIFull : public KVCalibrator {
public:
   enum LightFormula {
      kExact,
      kApproxIntegral,
      kApprox,
      kApproxSilicon
   };

protected:
   Int_t    fLightFormula;        //!light formula (see NIMa of Marian)
   Double_t fZmed;                //!Z of detector material
   Double_t fAmed;                //!A of detector material (CsI)
   Double_t u;

   UInt_t fZ;                   //!Z of nucleus to be calibrated
   UInt_t fA;                   //!A of nucleus to be calibrated

   KVIonRangeTableMaterial* fMaterialTable; //! range table for CsI

public:
   TF1* fLight;           //function parameterising light output as function of (energy, Z, A) with the full expression
   TF1* fDlight;          //function to integrate to get fLight

public:
   virtual Double_t Compute(Double_t chan) const;
   virtual Double_t operator()(Double_t chan);
   virtual Double_t Invert(Double_t);

   TF1* GetDLight()
   {
      return fDlight;
   }

public:
   KVLightEnergyCsIFull();
   KVLightEnergyCsIFull(const Char_t* name, const Char_t* type, KVDetector* kvd, Int_t lightFormula = kExact);//(KVDetector* kvd, Int_t lightFormula = kExact);
   virtual ~KVLightEnergyCsIFull();
   void Copy(TObject& obj) const;
   void init();

   void SetLightFormula(Int_t form)
   {
      fLightFormula = form;
   }
   Int_t GetLightFormula()
   {
      return fLightFormula;
   }

   Double_t dLightIntegral(double* x, double* par);
   Double_t dLightIntegralApprox(double* x, double* par);

   Double_t GetDeltaFraction(Double_t beta, Double_t beta_delta);
   Double_t GetLight(double* x, double* par);
   Double_t GetLightApprox(double* x, double* par);
   Double_t GetLightApproxSilicon(double* x, double* par);

   void SetZ(UInt_t z)
   {
      fZ = z;
   }
   void SetA(UInt_t a)
   {
      fA = a;
   }
   UInt_t GetZ() const
   {
      return fZ;
   }
   UInt_t GetA() const
   {
      return fA;
   }

   virtual void Print(Option_t* opt = "") const;

public:
   Double_t sp_e(double z, double a, double e);
   Double_t sp_n(double z, double a, double e);

protected:
   Double_t gamma(double z, double a, double e);
   Double_t gamma_ziegler(double z, double a, double e);
   Double_t gamma_hbg(double z, double a, double e);

   ClassDef(KVLightEnergyCsIFull, 1) //Light-energy calibration for CsI detectors using the full expression of Marian Parlog
};

#endif
