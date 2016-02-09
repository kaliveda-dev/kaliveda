//Created by KVClassFactory on Thu Oct 11 18:23:43 2012
//Author: Dijon Aurore

#ifndef __KVIVUPDATER_H
#define __KVIVUPDATER_H

#include "KVINDRAUpDater.h"

class KVIVUpDater : public KVINDRAUpDater {

public:
   KVIVUpDater();
   virtual ~KVIVUpDater();

   virtual void SetVamosCalibAndConfParams(KVDBRun*);
   virtual void SetPedestals(KVDBRun*);
   virtual void SetChIoSiPedestals(KVDBRun*);
   virtual void SetSi75SiLiPedestals(KVDBRun*);

   virtual void SetIDGrids(UInt_t);
   virtual void SetParameters(UInt_t run);
   virtual void SetPedestalCorrections(KVDBRun*);
   virtual void SetChVoltRefGains();

   ClassDef(KVIVUpDater, 1) //Class for setting INDRA-VAMOS parameter for each run (e494s/e503 experiment)
};

#endif
