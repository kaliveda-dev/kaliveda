//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#ifndef __KVSIMEVENT_H
#define __KVSIMEVENT_H

#include "KVTemplateEvent.h"
#include "KVSimNucleus.h"

/**
  \class KVSimEvent
  \brief Container class for simulated nuclei, KVSimNucleus
  \ingroup Simulation
\ingroup NucEvents
 */
class KVSimEvent : public KVTemplateEvent<KVSimNucleus> {
public:

   KVSimEvent(Int_t mult = 50)
      : KVTemplateEvent(mult)
   {}
   virtual ~KVSimEvent() {}
   virtual void Print(Option_t* t = "") const;

   Double_t GetTotalCoulombEnergy() const;

   ClassDef(KVSimEvent, 5) //Events from simulation
};

#endif
