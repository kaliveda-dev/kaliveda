//Created by KVClassFactory on Fri Jul 25 10:05:03 2014
//Author: John Frankland,,,

#ifndef __KVGEMINI_H
#define __KVGEMINI_H

#include "KVBase.h"

class TTree;

class KVSimNucleus;
class KVSimEvent;
class CYrast;

/**
   \class KVGemini
\brief Interface to GEMINI++
\ingroup Simulation

 You can use this class as an interface to Gemini++ to perform statistical
 decay of excited nuclei. In case of problems with the decay (CHucleus::abortEvent==1)
 we throw a standard exception of type gemini_bad_decay. This should be caught
 in your code when using DecaySingleNucleus() or DecayEvent():
~~~~{.cpp}
     KVGemini gem;
     KVSimEvent *hot, *cold;
     try {
        gem.DecayEvent(hot,cold);
     }
     catch (gemini_bad_decay& e) {
        // bad event, could not calculate all decays
     }
~~~~
*/

class KVGemini : public KVBase {

   int part_index;//! used for labelling decay products
//   static CYrast* yrast;

public:
   KVGemini();
   virtual ~KVGemini();

   void DecaySingleNucleus(KVSimNucleus&, KVSimEvent*, bool = true);
   void DecayEvent(const KVSimEvent*, KVSimEvent*, bool = true);
   void FillTreeWithEvents(KVSimNucleus&, bool, Int_t, TTree*, TString branchname = "");
   void FillTreeWithArrays(KVSimNucleus&, bool, Int_t, TTree*, TString mode = "EThetaPhi");

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
