
///
/// @file AbsorberEnergies.h
///
/// @section Description
///
/// Basic class acting as a container for the energy loss information of a
/// particle passing through the VAMOS absorber materials.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 14:07:48  2016
///

#ifndef __STAGE_ENERGIES_H__
#define __STAGE_ENERGIES_H__

#include <cassert>
#include "Rtypes.h"

/// @brief Namespace for the identification correction code, to prevent
/// polluting the global namespace.
namespace idc {

   class AbsorberEnergies {

      /// @brief Copy function
      /// @param rhs Target to copy
      void Copy(const AbsorberEnergies& rhs);

   public:

      /// @brief Default constructor
      AbsorberEnergies();
      /// @brief Default destructor
      virtual ~AbsorberEnergies();

      /// @brief Copy constructor
      /// @param rhs Target to copy
      AbsorberEnergies(const AbsorberEnergies& rhs);
      /// @brief Assignment operator
      /// @param rhs Target to copy
      AbsorberEnergies& operator=(const AbsorberEnergies& rhs);

      /// @brief Reset the stored data to their default values
      void Reset();

      /// @brief Energy loss in the target
      Double_t target;
      /// @brief Energy loss in the strip foil
      Double_t strip_foil;
      /// @brief Energy loss in drift chamber 1
      Double_t dc1;
      /// @brief Energy loss in the ACTIVE region of drift chamber 1
      Double_t dc1_active;
      /// @brief Energy loss in the SED
      Double_t sed;
      /// @brief Energy loss in drift chamber 2
      Double_t dc2;
      /// @brief Energy loss in the ACTIVE region of drift chamber 2
      Double_t dc2_active;
      /// @brief Energy loss in the ionisation chamber
      Double_t ic;
      /// @brief Energy loss in the ACTIVE region of the ionisation chamber
      Double_t ic_active;
      /// @brief Energy loss in the silicon detector
      Double_t si;
      /// @brief Energy loss in the isobutane dead-layer
      Double_t iso_sicsi;
      /// @brief Energy loss in the caesium iodide detector
      Double_t csi;

      ClassDef(AbsorberEnergies, 1);
   };

} // namespace idc

#endif
