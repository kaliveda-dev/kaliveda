/*
$Id: KVINDRADB_e475s.h,v 1.2 2007/09/21 07:44:09 franklan Exp $
$Revision: 1.2 $
$Date: 2007/09/21 07:44:09 $
*/

//Created by KVClassFactory on Tue Sep 18 11:17:53 2007
//Author: bonnet

#ifndef __KVINDRADB_E475S_H
#define __KVINDRADB_E475S_H

#include "KVINDRADB.h"
#include "KVDBTable.h"

#include "Riostream.h"
#include "TString.h"

class KVINDRADB_e475s : public KVINDRADB {
   void init();

protected:

   KVDBTable* fCalibrations;     //-> table of calibrations

public:

   KVINDRADB_e475s();
   KVINDRADB_e475s(const Char_t* name);
   virtual ~KVINDRADB_e475s();

   virtual void Build();
   void ReadCalibrations();
   void ReadCalibFile(std::ifstream& fin, TString, TString);

   void ReadPedestalList();
   void ReadPedestalFile(std::ifstream& fin, TString, TString);

   ClassDef(KVINDRADB_e475s, 1) //Database for E475s experiment
};

#endif
