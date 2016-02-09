//Created by KVClassFactory on Thu May  7 11:02:02 2015
//Author: John Frankland,,,

#ifndef __MCWEIGHT_H
#define __MCWEIGHT_H

#include "StatWeight.h"

namespace MicroStat {

   class mcweight : public StatWeight {

   protected:

   public:
      mcweight();
      virtual ~mcweight();

      virtual void SetWeight(KVEvent* e, Double_t E);
      void initGenerateEvent(KVEvent* partition);
      void resetGenerateEvent();
      virtual void nextparticleGenerateEvent(Int_t, KVNucleus*);

      ClassDef(mcweight, 1) //Calculate microcanonical statistical weights for events
   };

}/*  namespace MicroStat */

#endif
