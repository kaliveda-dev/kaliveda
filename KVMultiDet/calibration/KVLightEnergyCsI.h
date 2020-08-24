#ifndef KV_LIGHT_ENERGY_CSI_H
#define KV_LIGHT_ENERGY_CSI_H

#include "KVCalibrator.h"

/**
  \class KVLightEnergyCsI
  \brief Approximate light-energy calibration for CsI detectors
\ingroup Calibration

The function Double_t CalculLumiere(Double_t * x, Double_t * par) (in the source file KVLightEnergyCsI.cpp)
is used to parameterize the total light output as a function of deposited energy (in MeV).

See M. Parlog et al., Nuclear Instruments and Methods in Physics Research A 482 (2002) 693–706

This function has 4 parameters, a1, a2, a3 & a4. In order to correctly reproduce the light-energy
relationship for all ions, two parameterizations should be used: one for Z=1 and another for Z>1.
The main difference is the gain parameter, a1, which compensates the understimation of total
light output for high energy protons.

The parameter a3 normally has a fixed value (a3=6), but this is not "hard-coded" : it should be fixed
when fitting data.
*/

class KVLightEnergyCsI: public KVCalibrator {

   Double_t CalculLumiere(Double_t*, Double_t*);

protected:
   mutable Double_t Z;
   mutable Double_t A;

public:
   KVLightEnergyCsI();
   virtual ~ KVLightEnergyCsI() {}

   virtual Double_t Compute(Double_t chan, const KVNameValueList& z_and_a = "") const;
   virtual Double_t Invert(Double_t, const KVNameValueList& z_and_a = "") const;

   ClassDef(KVLightEnergyCsI, 1)        //Light-energy calibration for CsI detectors
};

#endif
