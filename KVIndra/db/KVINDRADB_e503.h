/*
$Id: KVINDRADB_e503.h,v 1.1 2009/01/22 15:39:26 franklan Exp $
$Revision: 1.1 $
$Date: 2009/01/22 15:39:26 $
*/

//Created by KVClassFactory on Wed Jan 21 13:44:30 2009
//Author: Chbihi

#ifndef __KVINDRADB_E503_H
#define __KVINDRADB_E503_H

#include "KVINDRADB.h"
using namespace std;

/**
\class KVINDRADB_e503
\brief Database for INDRA-VAMOS experiments E503 & E494S (2007)
\ingroup INDRADB

 Updated by A. Chbihi on 20/08/2009
 In addition to set the VAMOS scalers in the data base using ReadVamosScalers()  method
  it set the Vamos Brho's and angle.
 The new method of this class is ReadVamosBrhoAndAngle ().

 modified by M. F. Rivet november 2012 - separate files for pedestals Chio/Si
 and for calibration telescopes
*/
class KVINDRADB_e503 : public KVINDRADB {

protected:

   KVDBTable* fDeltaPed;   //-> table of pedestal corrections

   void init();
   virtual void ReadPedestalCorrection();
   virtual void ReadDeltaPedestal(ifstream& ifile);
   virtual void ReadVamosScalers() ;
   virtual void ReadVamosBrhoAndAngle() ;
   virtual void ReadPedestalList() ;

public:
   KVINDRADB_e503();
   KVINDRADB_e503(const Char_t*) ;
   virtual ~KVINDRADB_e503();

   void Build() ;

   ClassDef(KVINDRADB_e503, 1) //Database for e503 & e494s experiments with VAMOS
};

#endif
