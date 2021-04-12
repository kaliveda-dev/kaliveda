//Created by KVClassFactory on Sun Oct 27 09:22:29 2019
//Author: John Frankland,,,

#ifndef __KVINDRACSITOTLIGHTRECONSTRUCTOR_H
#define __KVINDRACSITOTLIGHTRECONSTRUCTOR_H

#include "KVDetectorSignal.h"

#include <KVINDRADetector.h>

/**
   \class KVINDRACsITotLightReconstructor
   \ingroup INDRACalibration
   \brief Calculation of light output from fast and slow components for INDRA CsI detectors

   As the calculation requires pedestal-corrected values of fast and slow components
   (detector signals "R" and "L" respectively), this signal is only defined for a detector if
   the corresponding signals ("R_PedCor" and "L_PedCor") are defined.
*/

//status values for calculation of total light output
#define NOT_CALCULATED 0
#define CALCULATED_WITH_GAIN_CORRECTION 1
#define NO_GAIN_CORRECTION 4
#define NEGATIVE_PEDESTAL_CORRECTED_VALUE 15
#define CALCULATION_NOT_CONVERGED 12

class KVINDRACsITotLightReconstructor : public KVDetectorSignal {
   KVDetectorSignal* fFast;//! pedestal corrected fast component, "R_PedCor"
   KVDetectorSignal* fSlow;//! pedestal corrected slow component, "L_PedCor"

   mutable UInt_t fLumTotStatus;        //status of light calculation
   Double_t tau;// ring/module-dependent 'tau' of PM base in ns
   Double_t c2;
   Double_t p0;
   Double_t p1;
   Double_t eps;
   /* Cette variable n'est pas utilisee, et ne figure pas dans e.g. le fortran de la 4eme campagne
    Float_t pre=0.4318;
   */
   Double_t c1;
   Double_t tau0;
   Double_t tau1;
   Double_t tau2;

   void Calculate() const;

   UInt_t GetStatusLumiere() const;
   Bool_t LightIsGood() const;

public:
   KVINDRACsITotLightReconstructor()
      : KVDetectorSignal("TotLight"), fFast(nullptr), fSlow(nullptr), fLumTotStatus(NOT_CALCULATED),
        tau(20)
   {
      p0 = 400;
      p1 = 900;
      eps = 1.e-4;
      /* Cette variable n'est pas utilisee, et ne figure pas dans e.g. le fortran de la 4eme campagne
       Float_t pre=0.4318;
      */
      c1 = 1.5;
      tau0 = 390.;
      tau1 = 1590.;
      tau2 = 3090.;
   }
   KVINDRACsITotLightReconstructor(const KVINDRADetector* det)
      : KVDetectorSignal("TotLight", det),
        fFast(det->GetDetectorSignal("R_PedCor")), fSlow(det->GetDetectorSignal("L_PedCor")),
        fLumTotStatus(NOT_CALCULATED), tau(20)
   {
      p0 = 400;
      p1 = 900;
      eps = 1.e-4;
      /* Cette variable n'est pas utilisee, et ne figure pas dans e.g. le fortran de la 4eme campagne
       Float_t pre=0.4318;
      */
      c1 = 1.5;
      tau0 = 390.;
      tau1 = 1590.;
      tau2 = 3090.;
      int ring = det->GetRingNumber();
      int module = det->GetModuleNumber();
      /****************************************************************************
      * Definition of PM rise-time constant tau
      * Comment based on a note from Bernard Borderie:
      *   rings 11-16 use PM bases with a different component, thus tau is 60 nsec
      *   ring 16, module 5 is an exception, it has a tau of 20 nsec !!
      *   ring 5, module 11 is also an exception, it has a tau of 60 nsec !!
      *   (all others have tau=20ns)
      */
      if (ring >= 11 && ring <= 16)
         tau = 60.;
      if (ring == 16 && module == 5)
         tau = 20.;
      if (ring == 5 && module == 11)
         tau = 60.;
      /****************************************************************************/
      if (ring > 3) {
         c2 = 2.2;
      }
      else {
         c2 = 3.3;
      }
   }

   virtual ~KVINDRACsITotLightReconstructor() {}

   Double_t GetValue(const KVNameValueList& = "") const
   {
      Calculate();
      return KVDetectorSignal::GetValue();
   }
   void Reset()
   {
      KVDetectorSignal::Reset();
      fLumTotStatus = NOT_CALCULATED;
   }
   Int_t GetStatus(const TString&) const;

   Bool_t IsRaw() const
   {
      // Override KVDetectorSignal method. Althought this class does not derive from KVCalibratedSignal,
      // it should not be considered as a raw parameter - it is calculated from pedestal-corrected
      // fast & slow components of CsI light output.
      return kFALSE;
   }

   ClassDef(KVINDRACsITotLightReconstructor, 1) //Calculation of light output from fast and slow components for INDRA CsI detectors
};

#endif
