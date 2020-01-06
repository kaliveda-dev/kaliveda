#include "KVLightEnergyCsI.h"
#include "TMath.h"
//#include "KVCsI.h"

ClassImp(KVLightEnergyCsI)

////////////////////////////////////////////////////////////////////////////
//Light-energy calibration for INDRA CsI detectors.
//
//The function Double_t CalculLumiere(Double_t * x, Double_t * par) (in the source file KVLightEnergyCsI.cpp)
//is used to parameterize the total light output as a function of deposited energy (in MeV).
//
// See M. Parlog et al., Nuclear Instruments and Methods in Physics Research A 482 (2002) 693–706
//
//This function has 4 parameters, a1, a2, a3 & a4. In order to correctly reproduce the light-energy
//relationship for all ions, two parameterizations should be used: one for Z=1 and another for Z>1.
//The main difference is the gain parameter, a1, which compensates the understimation of total
//light output for high energy protons.
//
//The parameter a3 normally has a fixed value (a3=6), but this is not "hard-coded" : it should be fixed
//when fitting data.

//___________________________________________________________________________

Double_t KVLightEnergyCsI::CalculLumiere(Double_t* x, Double_t* par)
{
   //Calcul de la lumiere totale a partir de Z, A d'une particule et son energie
   //
   //~~~~~~~~~~~~~~~~~~
   // x[0] = energie (MeV)
   // par[0] = a1 : gain factor
   // par[1] = a2 : nuclear & recombination quenching term
   // par[2] = a3 : threshold (MeV/u) for delta-ray production
   // par[3] = a4 : fractional energy loss removed by delta rays
   //~~~~~~~~~~~~~~~~~~
   //

   Double_t energie = x[0];
   Double_t c1 = par[0];
   Double_t c2 = Z * Z * A * par[1];
   Double_t c3 = A * par[2];
   Double_t c4 = par[3];
   Double_t T = 8 * A;
   Double_t c4_new = c4 / (1. + TMath::Exp((c3 - energie) / T));
   Double_t c0 = c4 / (1. + TMath::Exp(c3 / T));

   Double_t lumcalc = c1 * energie;
   if (c2 > 0.0) {
      Double_t xm = -c1 * c0 * c2 * TMath::Log(c2 / (c2 + c3));
      lumcalc = lumcalc - c1 * c2 * TMath::Log(1. + energie / c2) + c1 * c2 * c4_new * TMath::Log((energie + c2) / (c3 + c2)) + xm;
   }

   return lumcalc;
}

KVLightEnergyCsI::KVLightEnergyCsI(): KVCalibrator()
{
   //default initialisations
   SetType("LightEnergyCsI");
   SetCalibFunction(new TF1("fLight_CsI", this, &KVLightEnergyCsI::CalculLumiere, 0., 10000., 4));
   SetUseInverseFunction();
}

//___________________________________________________________________________
Double_t KVLightEnergyCsI::Compute(Double_t light, const KVNameValueList& z_and_a) const
{
   // Calculate the calibrated energy (in MeV) for a given total light output.
   //
   // The Z and A of the particle should be given as extra parameters:
   //
   //~~~~~~~~~~~~~~~{.cpp}
   //   Compute(light, "Z=3,A=7");
   //~~~~~~~~~~~~~~~
   //

   Z = z_and_a.GetIntValue("Z");
   A = z_and_a.GetIntValue("A");
   return KVCalibrator::Compute(light, z_and_a);
}

Double_t KVLightEnergyCsI::Invert(Double_t energy, const KVNameValueList& z_and_a) const
{
   // Given the calibrated (or simulated) energy in MeV,
   // calculate the corresponding total light output according to the
   // calibration parameters (useful for filtering simulations).
   //
   // The Z and A of the particle should be given as extra parameters:
   //
   //~~~~~~~~~~~~~~~{.cpp}
   //   Invert(light, "Z=3,A=7");
   //~~~~~~~~~~~~~~~
   //

   Z = z_and_a.GetIntValue("Z");
   A = z_and_a.GetIntValue("A");
   return KVCalibrator::Invert(energy, z_and_a);
}
