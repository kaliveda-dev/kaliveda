//Created by KVClassFactory on Thu Jan 30 14:22:15 2014
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_INDRA_CAMP5_PHDCORRECTION_H
#define __KVDATAPATCH_INDRA_CAMP5_PHDCORRECTION_H

#include "KVDataPatch.h"
/**
\class KVDataPatch_INDRA_camp5_PHDcorrection
\brief Patch for correcting Silicon PHD on rings 1-9 [INDRA_camp5 before 1.8.10]
\ingroup INDRADataPatch

Particle-level patch applied to all runs of INDRA 5th campaign 'root' data written with KaliVeda
version <1.8.10 and KVINDRAReconNuc class version < 11.

Patch is applied to all identified & calibrated nuclei with Z>10 on rings 1-9.
The particle energy is recalibrated.

*/

class KVDataPatch_INDRA_camp5_PHDcorrection : public KVDataPatch {

public:
   KVDataPatch_INDRA_camp5_PHDcorrection();
   virtual ~KVDataPatch_INDRA_camp5_PHDcorrection();
   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t runnumber,
                             TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist);
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

   ClassDef(KVDataPatch_INDRA_camp5_PHDcorrection, 1) //Patch for correcting Silicon PHD on rings 1-9 [INDRA_camp5 before 1.8.10]
};

#endif
