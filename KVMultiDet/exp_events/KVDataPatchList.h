//Created by KVClassFactory on Thu Jan 30 11:06:10 2014
//Author: John Frankland,,,

#ifndef __KVDATAPATCHLIST_H
#define __KVDATAPATCHLIST_H

#include "KVBase.h"
#include "KVUniqueNameList.h"
#include <KVReconstructedEvent.h>

/**
\class KVDataPatchList
\brief List of corrections to be applied to reconstructed calibrated data
\ingroup Analysis
*/

class KVDataPatchList : public KVBase {
   static KVUniqueNameList* fPatchList;//list of all defined patches
   void FillPatchList();
   void init();

protected:
   KVUniqueNameList fActiveEventPatches;//list of currently active event-level patches
   KVUniqueNameList fActiveParticlePatches;//list of currently active particle-level patches
   Bool_t fEventPatches;//=kTRUE if there are event-level patches
   Bool_t fParticlePatches;//=kTRUE if there are particle-level patches

public:
   KVDataPatchList();
   KVDataPatchList(const Char_t* name, const Char_t* title = "");
   virtual ~KVDataPatchList();

   void InitializePatchList(TString dataset, TString datatype, Int_t runnumber,
                            TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist);
   void Apply(KVReconstructedEvent*);

   void Print(Option_t* /*option*/ = "") const;
   Bool_t HasActivePatches() const
   {
      return (fEventPatches || fParticlePatches);
   }

   ClassDef(KVDataPatchList, 1) //List of corrections to be applied to reconstructed calibrated data
};

#endif
