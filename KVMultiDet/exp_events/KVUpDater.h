/*
$Id: KVUpDater.h,v 1.6 2007/10/01 15:03:38 franklan Exp $
$Revision: 1.6 $
$Date: 2007/10/01 15:03:38 $
$Author: franklan $
*/

#ifndef KVUPDATER_H
#define KVUPDATER_H

#include "TObject.h"
#include "TString.h"
#include "KVDBRun.h"

class KVMultiDetArray;

/**
\class KVUpDater
\brief Abstract class implementing necessary methods for setting multidetector parameters
for each run of the current dataset
\ingroup Identification,Calibration
*/

class KVUpDater {

protected:
   TString fDataSet;            //!name of dataset associated
   KVMultiDetArray* fArray;     //!associated array
public:

   KVUpDater();
   virtual ~ KVUpDater();
   void SetArray(KVMultiDetArray*);

   virtual void SetParameters(UInt_t);
   virtual void SetIdentificationParameters(UInt_t) ;
   virtual void SetCalibrationParameters(UInt_t);
   virtual void SetTarget(KVDBRun*);
   virtual void SetIDGrids(UInt_t);
   virtual void SetCalibParameters(KVDBRun*);

   static KVUpDater* MakeUpDater(const Char_t* uri, KVMultiDetArray*);

   ClassDef(KVUpDater, 0)       //Base class handling setting of multidetector parameters for each run
};
#endif
