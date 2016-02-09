
///
/// @file AbsorberEnergies.cpp
///
/// @section Description
///
/// Basic class acting as a container for the energy loss information of a
/// particle passing through the VAMOS absorber materials.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 14:07:48  2016
///

#include "AbsorberEnergies.h"

ClassImp(idc::AbsorberEnergies)

namespace idc {

   AbsorberEnergies::AbsorberEnergies() :
      target(-1.),
      strip_foil(-1.),
      dc1(-1.),
      dc1_active(-1.),
      sed(-1.),
      dc2(-1.),
      dc2_active(-1.),
      ic(-1.),
      ic_active(-1.),
      si(-1.),
      iso_sicsi(-1.),
      csi(-1.)
   {

   }

   AbsorberEnergies::~AbsorberEnergies()
   {

   }

   AbsorberEnergies::AbsorberEnergies(const AbsorberEnergies& rhs)
   {
      Copy(rhs);
   }

   AbsorberEnergies& AbsorberEnergies::operator=(const AbsorberEnergies& rhs)
   {
      Copy(rhs);
      return *this;
   }

   void AbsorberEnergies::Reset()
   {
      target = -1.;
      strip_foil = -1.;
      dc1 = -1.;
      dc1_active = -1.;
      sed = -1.;
      dc2 = -1.;
      dc2_active = -1.;
      ic = -1.;
      ic_active = -1.;
      si = -1.;
      iso_sicsi = -1.;
      csi = -1.;
   }

   void AbsorberEnergies::Copy(const AbsorberEnergies& rhs)
   {
      target = rhs.target;
      strip_foil = rhs.strip_foil;
      dc1 = rhs.dc1;
      dc1_active = rhs.dc1_active;
      sed = rhs.sed;
      dc2 = rhs.dc2;
      dc2_active = rhs.dc2_active;
      ic = rhs.ic;
      ic_active = rhs.ic_active;
      si = rhs.si;
      iso_sicsi = rhs.iso_sicsi;
      csi = rhs.csi;
   }

} // namespace idc

