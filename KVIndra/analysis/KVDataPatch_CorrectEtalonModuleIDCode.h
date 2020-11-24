//Created by KVClassFactory on Thu Jan 30 15:16:45 2014
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_CORRECTETALONMODULEIDCODE_H
#define __KVDATAPATCH_CORRECTETALONMODULEIDCODE_H

#include "KVDataPatch.h"
class KVINDRAReconEvent;
class KVDataPatch_CorrectEtalonModuleIDCode : public KVDataPatch {
   KVINDRAReconEvent* fEvent;//pointer to current event

   /**
   \class KVDataPatch_CorrectEtalonModuleIDCode
   \brief Patch for correcting bad id-code & calibration for particles stopping in CsI member of etalon modules [data written with v1.8.11]
   \ingroup INDRADataPatch

   Correct bad attribution of IDCode & calibration of particles stopping in CsI detectors behind etalon telescopes.
   Gammas (IDcode=0) and light charged particles (IDcode=2) were given
   IDcode=5 due to a bug in v1.8.11 (lp:bug#1271634).

   The status of the correction can be obtained from parameter KVDataPatch_CorrectEtalonModuleIDCode:
   ='correction applied' or 'correction failed'

   */
public:
   KVDataPatch_CorrectEtalonModuleIDCode();
   virtual ~KVDataPatch_CorrectEtalonModuleIDCode();
   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t runnumber,
                             TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist);
   virtual Bool_t IsEventPatch()
   {
      return kTRUE;
   }
   virtual Bool_t IsParticlePatch()
   {
      return kTRUE;
   }
   void ApplyToEvent(KVReconstructedEvent* e);
   void ApplyToParticle(KVReconstructedNucleus*);

   virtual void PrintPatchInfo() const;

   ClassDef(KVDataPatch_CorrectEtalonModuleIDCode, 1) //Patch for correcting bad id-code & calibration for particles stopping in CsI member of etalon modules [data written with v1.8.11]
};

#endif
