//Created by KVClassFactory on Wed Nov 16 14:10:43 2011
//Author: bonnet

#ifndef __KVINDRAUPDATER_E613_H
#define __KVINDRAUPDATER_E613_H

#include "KVINDRAUpDater.h"

class KVINDRAUpDater_e613 : public KVINDRAUpDater {

public:
   KVINDRAUpDater_e613();
   virtual ~KVINDRAUpDater_e613();

   virtual void SetParameters(UInt_t run);
   virtual void SetGains(KVDBRun*);
   virtual void SetPedestals(KVDBRun*);
   virtual void SetChVoltParameters(KVDBRun*);

   ClassDef(KVINDRAUpDater_e613, 1) //Sets run parameters for INDRA_e613 dataset
};

#endif
