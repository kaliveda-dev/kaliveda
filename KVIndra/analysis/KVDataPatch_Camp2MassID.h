//Created by KVClassFactory on Sat Jun  9 20:07:03 2018
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_CAMP2MASSID_H
#define __KVDATAPATCH_CAMP2MASSID_H

#include "KVDataPatch.h"

/**
\class KVDataPatch_Camp2MassID
\brief Correct mass identification in 2nd campaign data
\ingroup INDRADataPatch

Correct bad attribution of isotopic resolution & mass in 2nd campaign data:
    - all particles identified in Phoswich detectors (Ring 1) were labelled as being isotopically identified.
    - calculated masses were not consistent: several A for some Z. We give single KVNucleus::kVedaMass A to all

*/

class KVDataPatch_Camp2MassID : public KVDataPatch {
public:
   KVDataPatch_Camp2MassID();
   virtual ~KVDataPatch_Camp2MassID();
   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t,
                             TString, Int_t, const TList*)
   {
      return (dataset == "INDRA_camp2" && datatype == "root");
   }
   virtual Bool_t IsEventPatch()
   {
      return kFALSE;
   }
   virtual Bool_t IsParticlePatch()
   {
      return kTRUE;
   }
   void ApplyToEvent(KVReconstructedEvent*) {}
   void ApplyToParticle(KVReconstructedNucleus*);

   virtual void PrintPatchInfo() const;

   ClassDef(KVDataPatch_Camp2MassID, 1) //Patch to correct 2nd campaign A-identification status
};

#endif
