//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#ifndef __KVSIMEVENT_H
#define __KVSIMEVENT_H

#include "KVEvent.h"

/**
  \class KVSimEvent
  \brief Container class for simulated nuclei, KVSimNucleus
  \ingroup Simulation
\ingroup NucEvents
 */
class KVSimEvent : public KVEvent {
public:

   KVSimEvent(Int_t mult = 50, const char* classname = "KVSimNucleus");
   virtual ~KVSimEvent();
   virtual void Print(Option_t* t = "") const;

   Double_t GetTotalCoulombEnergy() const;

   ClassDef(KVSimEvent, 3) //Events from simulation
};

#endif
