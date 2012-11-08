//Created by KVClassFactory on Thu Oct 11 18:23:43 2012
//Author: Dijon Aurore

#ifndef __KVINDRAUPDATER_E494S_H
#define __KVINDRAUPDATER_E494S_H

#include "KVINDRAUpDater.h"

class KVINDRAUpDater_e494s : public KVINDRAUpDater
{

   public:
   KVINDRAUpDater_e494s();
   virtual ~KVINDRAUpDater_e494s();
   
   virtual void SetPedestals(KVDBRun *);
   virtual void SetChIoSiPedestals(KVDBRun *);
   virtual void SetSi75SiLiPedestals(KVDBRun *);
   
   virtual void SetIDGrids(UInt_t);
   virtual void SetParameters(UInt_t run);
   virtual void SetPedestalCorrections(KVDBRun *);
   virtual void SetChVoltRefGains();

   ClassDef(KVINDRAUpDater_e494s,1)//Class for setting INDRA-VAMOS parameter for each run (e494s/e503 experiment)
};

#endif
