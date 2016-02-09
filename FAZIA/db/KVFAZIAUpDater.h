//Created by KVClassFactory on Mon Sep 14 10:58:04 2015
//Author: ,,,

#ifndef __KVFAZIAUpDater_H
#define __KVFAZIAUpDater_H

#include "KVUpDater.h"

class KVFAZIAUpDater : public KVUpDater {

public:
   KVFAZIAUpDater();
   virtual ~KVFAZIAUpDater();
   virtual void SetCalibParameters(KVDBRun*);
   void SetPSAParameters(KVDBRun*);
   void SetCalibrations(KVDBRun*);

   ClassDef(KVFAZIAUpDater, 1) //handle FAZIA detectors configuration for a given run
};

#endif
