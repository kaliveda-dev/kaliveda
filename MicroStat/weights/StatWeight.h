//Created by KVClassFactory on Thu May  7 11:00:37 2015
//Author: John Frankland,,,

#ifndef __STATWEIGHT_H
#define __STATWEIGHT_H

#include "KVTemplateEvent.h"

namespace MicroStat {

   /**
   \class StatWeight
   \brief Abstract base class for calculating statistical weights for events
   \ingroup MicroStat \ingroup Simulation
   */

   class StatWeight : public TObject {
   private:
      void init();

      Double_t fWeight; //calculated weight
      Long64_t fIndex;  //index of corresponding partition
      Double_t fEDisp;  //available kinetic energy - set by SetWeight(KVEvent*, Double_t)

   protected:
      void setWeight(Double_t w)
      {
         fWeight = w;
      }
      void setAvailableEnergy(Double_t e)
      {
         fEDisp = e;
      }

   public:
      StatWeight();
      virtual ~StatWeight();

      virtual void SetWeight(KVEvent* e, Double_t E) = 0;
      Double_t GetWeight() const
      {
         return fWeight;
      }
      Double_t GetAvailableEnergy() const
      {
         return fEDisp;
      }

      void SetIndex(Long64_t i)
      {
         fIndex = i;
      }
      Long64_t GetIndex() const
      {
         return fIndex;
      }

      void ls(Option_t* = "") const;
      Bool_t IsSortable() const
      {
         return kTRUE;
      }

      Int_t Compare(const TObject* obj) const;

      void GenerateEvent(KVEvent* partition, KVEvent* event);
      virtual void initGenerateEvent(KVEvent* partition) = 0;
      virtual void resetGenerateEvent() = 0;
      virtual void nextparticleGenerateEvent(Int_t, KVNucleus*) = 0;

      ClassDef(StatWeight, 1) //Abstract base class for calculating statistical weights for events
   };

}/*  namespace MicroStat */

#endif
