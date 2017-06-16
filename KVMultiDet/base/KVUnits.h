#ifndef KVUNITS__H
#define KVUNITS__H

#include <Rtypes.h>

namespace KVUnits {
   // UNITS
   // Standard units are:
   //   [L]   cm
   //   [M]   g
   //   [P]   Torr
   //   [V]   cc/ml
   //   [E]   MeV
   //   [t]   fm/c
   //   [D]   nuc/fm3
   //   [T]   MeV/kb
   // lengths
   const long double cm = 1.0l;
   const long double fm = 1.e-13l;
   const long double um = 1.e-4l;
   const long double mm = 1.e-1l;
   const long double m = 1.e+2l;
   //  masses
   const long double g = 1.0l;
   const long double kg = 1.e+3l;
   const long double mg = 1.e-3l;
   const long double ug = 1.e-6l;
   const long double MeVpc2 = 1.782662e-27l;
   // pressures
   const long double torr = 1.0l;
   const long double atm = 760.l;
   const long double Pa = atm / 101325.l;
   const long double mbar = 100.l * Pa;
   // volumes
   const long double cc = 1.0l;
   const long double litre = 1.e+3l;
   const long double cl = 10.l * cc;
   const long double ml = cc;
   // energies
   const long double MeV = 1.0l;
   const long double keV = 1.e-3l;
   const long double eV = 1.e-6l;
   const long double Joule = 6.241509e+12l;
   // times
   const long double fmpc = 1.0l;
   const long double s = 2.997925e+23l;
   // densities
   const long double nucpfm3 = 1.0l;
   const long double gpcm3 = 6.022141e-16;   // 1./(KVNucleus::kAMU*KVUnits::MeVpc2/TMath::Power(KVUnits::fm,3.))
   // temperatures
   const long double MeVpkb = 1.0l;
   const long double GK = 8.617331e-02;
};

#endif
