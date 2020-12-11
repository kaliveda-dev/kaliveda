//Created by KVClassFactory on Thu May  7 11:00:37 2015
//Author: John Frankland,,,

#include "StatWeight.h"

ClassImp(MicroStat::StatWeight)

namespace MicroStat {

   void StatWeight::init()
   {
      // Default initialisations

      fWeight = 0;
   }

   StatWeight::StatWeight()
   {
      // Default constructor
      init();
   }

   //________________________________________________________________

   StatWeight::~StatWeight()
   {
      // Destructor
   }

   void StatWeight::ls(Option_t*) const
   {
      std::cout << ClassName() << " index = " << GetIndex() <<
                "  available energy = " << GetAvailableEnergy() <<
                "  weight = " << GetWeight() << std::endl;
   }

   Int_t StatWeight::Compare(const TObject* obj) const
   {
      // sort according to weight

      Double_t w = ((StatWeight*)obj)->GetWeight();
      if (w < fWeight) return -1;
      return (w == fWeight ? 0 : 1);
   }

   void StatWeight::GenerateEvent(KVEvent* partition, KVEvent* event)
   {
      // Generate a full kinematical event using the statistical weight
      // for the given partition with given available kinetic energy.
      // Before calling this method, either call
      //    initGenerateEvent(...)  // the first time, or
      //    resetGenerateEvent(...) // on subsequent calls

      // initialise the event
      event->Clear();
      Int_t mult = 0;
      KVNucleus* part;
      while ((part = partition->GetNextParticle())) {
         event->AddParticle()->SetZandA(part->GetZ(), part->GetA());
         ++mult;
      }

      // generate momenta
      while ((part = event->GetNextParticle())) {
         nextparticleGenerateEvent(mult, part);
         --mult;
      }
   }

}/*  namespace MicroStat */

