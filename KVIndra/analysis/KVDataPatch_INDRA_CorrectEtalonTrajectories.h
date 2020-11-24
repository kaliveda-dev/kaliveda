//Created by KVClassFactory on Fri Feb  7 13:42:32 2020
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_INDRA_CORRECTETALONTRAJECTORIES_H
#define __KVDATAPATCH_INDRA_CORRECTETALONTRAJECTORIES_H

#include "KVDataPatch.h"

/**
\class KVDataPatch_INDRA_CorrectEtalonTrajectories
\brief Correct angular distributions for CsI & ChIo-CsI identified particles in etalon telescopes
\ingroup INDRADataPatch

    Apply the following correction to INDRA data:
       - all particles identified in etalon modules by either CSI_R_L or CI_CSI
       identification were treated as if they passed through the etalon detectors.
       - we check the coherency of this with the energy of the particle and
       correct when necessary

*/

class KVDataPatch_INDRA_CorrectEtalonTrajectories : public KVDataPatch {
   Bool_t fRandomAngles;

public:
   KVDataPatch_INDRA_CorrectEtalonTrajectories();
   virtual ~KVDataPatch_INDRA_CorrectEtalonTrajectories() {}

   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t,
                             TString, Int_t, const TList*)
   {
      return ((dataset == "INDRA_camp1" || dataset == "INDRA_camp2") && datatype == "root");
   }
   virtual Bool_t IsEventPatch()
   {
      return kTRUE;
   }
   virtual Bool_t IsParticlePatch()
   {
      return kTRUE;
   }
   void ApplyToEvent(KVReconstructedEvent*);
   void ApplyToParticle(KVReconstructedNucleus*);

   virtual void PrintPatchInfo() const;

   ClassDef(KVDataPatch_INDRA_CorrectEtalonTrajectories, 1) //Correct angular distributions for CsI & ChIo-CsI identified particles in etalon telescopes
};

#endif
