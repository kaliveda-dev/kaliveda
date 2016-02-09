
///
/// @file CorrectionData.h
///
/// @section Description
///
/// Basic class acting as a container for any identification date which need to
/// be passed into the idc::ApplyCorrections function. We prefer to pass in this
/// data manually as the relevant getters in KVVAMOSReconNuc are liable to
/// recalculate values we don't want it to!
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 14:03:01  2016
///

#ifndef __CORRECTION_DATA_H__
#define __CORRECTION_DATA_H__

#include "Rtypes.h"
#include "KVIDTelescope.h"

/// @brief Namespace for the identification correction code, to prevent
/// polluting the global namespace.
namespace idc {

   class CorrectionData {

      /// @brief Copy function
      /// @param rhs Target to copy
      void Copy(const CorrectionData& rhs);

   public:

      /// @brief Default constructor
      CorrectionData();
      /// @brief Default destructor
      virtual ~CorrectionData();

      /// @brief Copy constructor
      /// @param rhs Target to copy
      CorrectionData(const CorrectionData& rhs);

      /// @brief Assignment operator
      /// @param rhs Target to copy
      CorrectionData& operator=(const CorrectionData& rhs);

      /// @brief Identifying telescope
      KVIDTelescope* idt;
      /// @brief Z value returned from the standard ID routine
      Int_t z_value;
      /// @brief A value returned from the standard ID routine
      Int_t a_value;
      /// @brief Real Z returned from the standard ID routine
      Double_t z_real;
      /// @brief Real A returned from the standard ID routine
      Double_t a_real;
      /// @brief Particle ID returned from the standard ID routine
      Double_t pid;
      /// @brief Total energy of the identified particle
      Double_t energy;

      ClassDef(CorrectionData, 1);
   };

} // namespace idc

#endif
