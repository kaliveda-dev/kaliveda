
///
/// @file KVIVDB_e503.h
///
/// @section Description
///
/// Database class for the E503 experiment. This class inherits from that of our
/// sister experiment (E494s) and adds the functionality required to implement
/// the VAMOS identification corrections.
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed Jun 11 15:00:35 CEST 2014
///

#ifndef __KVIVDB_E503_H__
#define __KVIVDB_E503_H__

// C
#include <cstdlib>
#include <cassert>

// C++
#include <fstream>
#include <iostream>
#include <vector>

// ROOT
#include "Rtypes.h"
#include "TError.h"
#include "TIterator.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"

// KaliVeda (Standard)
#include "KVDBParameterSet.h"
#include "KVIDGridManager.h"
#include "KVIVDB.h"
#include "KVVAMOS.h"

class KVIVDB_e503 : public KVIVDB {
   KVDBTable* id_correction_parameters_; ///< ID correction parameters table
   Bool_t verbose_; ///< Enable for more verbose console output

protected:

   /// Initialisation function
   virtual void Init();

   /// @brief Read the id correction configuration file specified by the
   ///  environment variable VAMOSIDCorrectionList
   virtual Bool_t ReadIdCorrectionParameters();

   /// @brief Read the correction file using the "Correction File Format"
   /// @param path Absoulte path of the correction file
   virtual Bool_t ReadCorrectorFile(const TString& path);

   // @brief Copy constructor (Private - fobid copy construction)
   KVIVDB_e503(const KVIVDB_e503&);

   // @brief Assignment operator (Private - forbid assignment)
   KVIVDB_e503& operator=(const KVIVDB_e503&);


public:

   /// Default Constructor
   KVIVDB_e503();

   /// @brief Constructor with dataset name
   /// @param name Dataset name
   KVIVDB_e503(const Char_t* name);

   /// Default Destructor
   virtual ~KVIVDB_e503();

   /// @brief Build database - Read calibration/correction parameters
   virtual void Build();
   virtual void set_verbose(Bool_t status = kTRUE);

   ClassDef(KVIVDB_e503, 1)
};

#endif



