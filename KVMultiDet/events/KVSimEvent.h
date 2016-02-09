//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#ifndef __KVSIMEVENT_H
#define __KVSIMEVENT_H

#include "KVEvent.h"

class KVSimEvent : public KVEvent {
public:

   KVSimEvent(Int_t mult = 50, const char* classname = "KVSimNucleus");
   virtual ~KVSimEvent();

   ClassDef(KVSimEvent, 3) //Events from simulation
};

#endif
