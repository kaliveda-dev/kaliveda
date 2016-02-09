
///
/// @file KVIVUpDater_e503.h
///
/// @section Description
///
/// KVIVUpDater class for the E503 experiment. This class inherits from that of
/// our sister experiment (E494s) and adds the functionality required to
/// implement the VAMOS identification corrections.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed Jun 11 15:09:35 CEST 2014
///

#ifndef __KVIVUPDATER_E503_H__
#define __KVIVUPDATER_E503_H__

// C
#include <cstdlib>
#include <cassert>

// C++
#include <stdexcept>

// KaliVeda (Standard)
#include "KVDBParameterSet.h"
#include "KVDBRecord.h"
#include "KVDBRun.h"
#include "KVINDRADB.h"
#include "KVIVUpDater.h"

// KaliVeda (VAMOS)
#include "KVIDHarpeeICSi_e503.h"
#include "KVIDHarpeeSiCsI_e503.h"
#include "KVVAMOS.h"

class KVIVUpDater_e503 : public KVIVUpDater {
   /// @brief Copy constructor (Private - forbid copy construction)
   KVIVUpDater_e503(const KVIVUpDater_e503&);

   /// @brief Assignment operator (Private - forbid assignment)
   KVIVUpDater_e503& operator=(const KVIVUpDater_e503&);

public:

   /// @brief Default Constructor
   KVIVUpDater_e503();

   /// @brief Destructor
   virtual ~KVIVUpDater_e503();

   /// @brief Set parameters for a particular run
   /// @param run The run number
   virtual void SetParameters(UInt_t run);

   ClassDef(KVIVUpDater_e503, 1)
};

#endif

