
///
/// @file Corrections_e503.h
///
/// @section Description
///
/// A set of non-member, non-friend functions which together act as a wrapper
/// for the standard KaliVeda identification functions and enable the
/// identified nucleus to be modified in accordance with the implemented
/// identification corrections.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 13:58:10  2016
///

#ifndef __CORRECTIONS_E503_H__
#define __CORRECTIONS_E503_H__

#include "AbsorberEnergies.h"
#include "CorrectionData.h"
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"
#include "KVVAMOSReconNuc.h"

/// @brief Namespace for the identification correction code, to prevent
/// polluting the global namespace.
namespace idc {

/// @brief Return codes for the idc::Identify function
/// @see idc::Identify()
   enum IDReturnCodes {
      kAllOK = 0,
      kBadTimeCal,           //  1
      kBasicIdNotOk,         //  2
      kBothInherited,        //  3
      kICNotInherited,       //  4
      kICSiNullParlist,      //  5
      kInvalidIdCode,        //  6
      kNoIdResult,           //  7
      kNoIdTelescope,        //  8
      kNoTarget,             //  9
      kNotCalibrated,        //  10
      kNotIdentified,        //  11
      kNotInherited,         //  12
      kNotStoppingDetector,  //  13
      kNullParset,           //  14
      kResidualEnergyFail,   //  15
      kSegmentZero,          //  16
      kSiCsINullParlist,     //  17
      kUnCharged,            //  18
      kUnChargedResult,      //  19
      kZIdNotOk,             //  20
      kCorrectionsFailed
   };


/// @brief Identification function for E503.
///
/// This function acts as a wrapper to the standard identification routines and
/// MODIFIES THE IDENTIFIED NUCLEUS by implementing the identification
/// correction functions via idc::ApplyCorrections.
///
/// @param n The nucleus to be identified
///
/// @param energy_data An optional pointer to an idc::AbsorberEnergies object,
/// this object is then used to store the energy the nucleus deposits in each of
/// the absorbers of the VAMOS setup.
///
/// @return Return code from the enum idc::IDReturnCodes.
   UChar_t Identify(
      KVVAMOSReconNuc* const n,
#if __cplusplus < 201103L
      AbsorberEnergies* const energy_data = NULL
#else
      AbsorberEnergies* const energy_data = nullptr
#endif
   );

/// @brief Applies the identification corrections to the nucleus
///
/// Once the nucleus has been identified using the standard KaliVeda routines
/// one can supply the nucleus to this function and it will apply all of the
/// implemented identification corrections. This function MODIFIES THE SUPPLIED
/// NUCLEUS!
///
/// @param n The nucleus to be corrected.
/// @param data Pointer to an idc::CorrectionData object containing all of the
/// necessary values for the correction procedure (We don't trust routines of
/// the type n->GetSomeValue() which can rely on mass values etc. we have not
/// set yet)
///
/// @return Boolean indicating success or failure in applying the corrections.
   Bool_t ApplyCorrections(
      KVVAMOSReconNuc* const n,
      const CorrectionData* const data
   );

/// @brief Applies the ID corrections for the ionisation chamber -> silicon
/// telescope.
///
/// Generally called from idc::ApplyCorrections, here we apply any corrections
/// which are required in the ionisation chamber -> silicon identification
/// telescope the particle passed through.
///
/// @param n The nucleus to be corrected.
/// @param idt The identification telescope.
/// @param data Pointer to an idc::CorrectionData object containing all of the
/// necessary values for the correction procedure (We don't trust routines of
/// the type n->GetSomeValue() which can rely on mass values etc. we have not
/// set yet)
///
/// @return Boolean indicating success or failure in applying the corrections.
   Bool_t ApplyIcSiCorrections(
      KVVAMOSReconNuc* const n,
      const KVIDHarpeeICSi_e503* const idt,
      const CorrectionData* const data
   );

/// @brief Applies the ID corrections for the silicon -> caesium iodide
/// telescope.
///
/// Generally called from idc::ApplyCorrections, here we apply any corrections
/// which are required in the silicon -> caesium iodide identification
/// telescope the particle passed through.
///
/// @param n The nucleus to be corrected
/// @param idt The identification telescope.
/// @param data Pointer to an idc::CorrectionData object containing all of the
/// necessary values for the correction procedure (We don't trust routines of
/// the type n->GetSomeValue() which can rely on mass values etc. we have not
/// set yet)
///
/// @return Boolean indicating success or failure in applying the corrections.
   Bool_t ApplySiCsiCorrections(
      KVVAMOSReconNuc* const n,
      const KVIDHarpeeSiCsI_e503* const idt,
      const CorrectionData* const data
   );

/// @brief Apply M.Boisjoli's correction to the A/Q value.
///
/// This algorithm has been taken directly from the old code
/// (VAMOS/deprecated/Identificationv).
///
/// @param parset Parameter set associated with this correction
/// @param uncorrected Uncorrected A/Q value
/// @param pid The particle identification code
///
/// @return Corrected A/Q value
   Double_t CorrectAoverQ(
      KVDBParameterSet* const parset,
      Double_t uncorrected,
      Double_t pid
   );

} // namespace idc


#endif
