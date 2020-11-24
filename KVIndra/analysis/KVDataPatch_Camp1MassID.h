//Created by KVClassFactory on Wed Jun 19 15:54:49 2019
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_CAMP1MASSID_H
#define __KVDATAPATCH_CAMP1MASSID_H

#include "KVDataPatch.h"

/**
\class KVDataPatch_Camp1MassID
\brief Correct mass identification in 1st campaign data
\ingroup INDRADataPatch

 Correct the following errors in 1st campagn data:
   - ring=1 idcode=2 (phoswich): should have IsAMeasured=kFALSE
   - ring=2-9 idcode=3 (si-csi): should have IsAMeasured=kFALSE
   - give correct VEDA mass to all particles with IsAMeasured=kFALSE
   - etalons have many strange isotopes (with mass id upto 12~13)
       - set IsAMeasured=kFALSE for any unknown nucleus or with lifetime < 1ns
*/

class KVDataPatch_Camp1MassID : public KVDataPatch {
public:
   KVDataPatch_Camp1MassID();
   virtual ~KVDataPatch_Camp1MassID();

   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t,
                             TString, Int_t, const TList*)
   {
      return (dataset == "INDRA_camp1" && datatype == "root");
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

   ClassDef(KVDataPatch_Camp1MassID, 1) //Correct mass identification in 1st campaign data
};

#endif
