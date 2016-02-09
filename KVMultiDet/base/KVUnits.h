#ifndef KVUNITS__H
#define KVUNITS__H

#include <Rtypes.h>

namespace KVUnits {
   // UNITS
   // Standard units are:
   //   [L]   cm
   //   [M]  g
   //   [P]   Torr
   //   [V]   cc/ml
   // lengths
   const long double cm = 1.0l;
   const long double um = 1.e-4l;
   const long double mm = 1.e-1l;
   const long double m = 1.e+2l;
   //  masses
   const long double g = 1.0l;
   const long double kg = 1.e+3l;
   const long double mg = 1.e-3l;
   const long double ug = 1.e-6l;
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
};

#endif
