//Created by KVClassFactory on Fri Jul 25 10:05:03 2014
//Author: John Frankland,,,

#ifndef __KVGEMINI_H
#define __KVGEMINI_H

#include "KVBase.h"

class TTree;

class KVSimNucleus;
class KVSimEvent;
class CYrast;

class KVGemini : public KVBase {

   int part_index;//! used for labelling decay products
//   static CYrast* yrast;

public:
   KVGemini();
   virtual ~KVGemini();

   void DecaySingleNucleus(KVSimNucleus&, KVSimEvent*);
   void DecayEvent(const KVSimEvent*, KVSimEvent*);
   void FillTreeWithEvents(KVSimNucleus&, Int_t, TTree*, TString branchname = "");
   void FillTreeWithArrays(KVSimNucleus&, Int_t, TTree*, TString mode = "EThetaPhi");

   Float_t GetMaxSpinWithFissionBarrier(int, int);
   Float_t GetFissionBarrierRLDM(int z, int a, float J);
   Float_t GetFissionBarrierSierk(int z, int a);

   ClassDef(KVGemini, 1) //Interface to gemini++
};

#ifndef __CINT__
// Exception(s) thrown by KVGemini
#include <exception>
class gemini_bad_decay : public std::exception {
   virtual const char* what() const throw()
   {
      return "problem with gemini decay: CNucleus::abortEvent==true";
   }
};
#endif
#endif
