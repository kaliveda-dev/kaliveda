//Created by KVClassFactory on Thu May  7 11:02:02 2015
//Author: John Frankland,,,

#include "mcweight.h"

ClassImp(MicroStat::mcweight)

namespace MicroStat {

   mcweight::mcweight()
   {
      // Default constructor
   }

   mcweight::~mcweight()
   {
      // Destructor
   }

   void mcweight::SetWeight(KVEvent*, Double_t)
   {
      // Calculate event weight for given available energy

   }

   void mcweight::initGenerateEvent(KVEvent*)
   {

   }

   void mcweight::resetGenerateEvent()
   {

   }

   void mcweight::nextparticleGenerateEvent(Int_t, KVNucleus*)
   {

   }

}/*  namespace MicroStat */
