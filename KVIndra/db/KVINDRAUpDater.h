/*
$Id: KVINDRAUpDater.h,v 1.6 2007/10/01 15:03:38 franklan Exp $
$Revision: 1.6 $
$Date: 2007/10/01 15:03:38 $
$Author: franklan $
*/

#ifndef KVINDRAUPDATER_H
#define KVINDRAUPDATER_H

#include "KVUpDater.h"

class KVDBRun;
class KVINDRA;

/**
\class KVINDRAUpDater
\brief Class implementing necessary methods for setting INDRA parameters for each run, using information stored in the KVINDRADB database.
\ingroup INDRADB
*/
class KVINDRAUpDater: public KVUpDater {

public:

   KVINDRAUpDater();
   virtual ~ KVINDRAUpDater()
   {
   }

   virtual void SetParameters(UInt_t run);
   virtual void SetCalibrationParameters(UInt_t);

   virtual void SetTrigger(KVDBRun*);
   virtual void SetGains(KVDBRun*);

   virtual void CheckStatusOfDetectors(KVDBRun*);
   virtual void SetCalibParameters(KVDBRun*);
   virtual void SetChIoPressures(KVDBRun*);
   //virtual void SetPedestals(KVDBRun*);

   virtual void SetChVoltParameters(KVDBRun*);
   virtual void SetLitEnergyCsIParameters(KVDBRun*);
   virtual void SetCsIGainCorrectionParameters(KVDBRun*);
   virtual void SetVoltEnergyChIoSiParameters(KVDBRun*);
   virtual void SetPHDs(KVDBRun*);
//   virtual void SetChIoSiPedestals(KVDBRun*);
//   virtual void SetCsIPedestals(KVDBRun*);

   KVINDRA* GetINDRA();

   ClassDef(KVINDRAUpDater, 0)  //Class handling setting of INDRA parameters for each run
};

#endif
