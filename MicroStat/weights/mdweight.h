//Created by KVClassFactory on Thu May  7 11:02:24 2015
//Author: John Frankland,,,

#ifndef __MDWEIGHT_H
#define __MDWEIGHT_H

#include "StatWeight.h"
#include "TF1.h"
#include "KVHashList.h"

namespace MicroStat {

   class mdweight : public StatWeight {
   private:
      Double_t log2pi, log10twelve;
      Double_t eDisp, massTot, massTot0, px, py, pz;
      KVHashList fKEDist;//!
      Double_t A, B;
      static Double_t edist(Double_t*, Double_t*);

      TF1* getKEdist(Int_t, Double_t);

   protected:

   public:
      mdweight();
      virtual ~mdweight();

      virtual void SetWeight(KVEvent* e, Double_t E);

      void initGenerateEvent(KVEvent* partition);
      void resetGenerateEvent();
      virtual void nextparticleGenerateEvent(Int_t, KVNucleus*);

      void printKElist() const
      {
         fKEDist.Print();
      }

      ClassDef(mdweight, 1) //Calculate molecular dynamics ensemble weights for events
   };

}/*  namespace MicroStat */

#endif
