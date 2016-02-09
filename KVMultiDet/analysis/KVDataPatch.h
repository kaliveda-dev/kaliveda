//Created by KVClassFactory on Thu Jan 30 10:29:29 2014
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_H
#define __KVDATAPATCH_H

#include "KVBase.h"
class KVEvent;
class KVNucleus;
class KVDataPatch : public KVBase {

public:
   KVDataPatch();
   KVDataPatch(const Char_t* name, const Char_t* title = "");
   virtual ~KVDataPatch();

   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t runnumber,
                             TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist) = 0;
   virtual Bool_t IsEventPatch() = 0;
   virtual Bool_t IsParticlePatch() = 0;
   virtual void ApplyToEvent(KVEvent*) = 0;
   virtual void ApplyToParticle(KVNucleus*) = 0;

   virtual void PrintPatchInfo() const = 0;

   void Print(Option_t* /*opt*/ = "") const;

   ClassDef(KVDataPatch, 1) //Correction to be applied to reconstructed calibrated data
};

#endif
