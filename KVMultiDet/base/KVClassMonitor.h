//Created by KVClassFactory on Thu Jun 17 14:19:50 2010
//Author: John Frankland,,,

#ifndef __KVCLASSMONITOR_H
#define __KVCLASSMONITOR_H

#include "KVBase.h"
#include "TEnv.h"

class KVClassMonitor : public KVBase {
   TEnv fClassStats;//store class instance statistics
   TEnv fClassStatsInit;//intitial class instance statistics

public:
   KVClassMonitor();
   virtual ~KVClassMonitor();

   virtual void Check();
   virtual void CompareToInit();
   virtual void SetInitStatistics();

   ClassDef(KVClassMonitor, 1) //Track potential memory leaks
};

#endif
