//Created by KVClassFactory on Tue Sep  4 11:26:12 2012
//Author: Guilain ADEMARD

#ifndef __KVINDRA_VAMOS_H
#define __KVINDRA_VAMOS_H

#include "KVMultiDetArray.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVINDRA;
class KVVAMOS;

class KVINDRA_VAMOS : public KVMultiDetArray {

public:
   KVINDRA_VAMOS();
   virtual ~KVINDRA_VAMOS();

   void init();

   virtual void Build(Int_t run = -1);
   virtual void Clear(Option_t* opt = "");
   virtual KVNameValueList* DetectParticle(KVNucleus* part);
   virtual void DetectEvent(KVEvent* event, KVReconstructedEvent* rec_event, const Char_t* detection_frame = "");
   using KVMultiDetArray::GetDetectorEvent;
   virtual void   GetDetectorEvent(KVDetectorEvent* detev, TSeqCollection* fired_params = 0);
   using KVMultiDetArray::GetGroupWithAngles;
   virtual KVGroup* GetGroupWithAngles(Float_t theta, Float_t phi);

   virtual void SetFilterType(Int_t t);
   virtual void   SetParameters(UShort_t n);
   virtual void SetROOTGeometry(Bool_t on = kTRUE);

   KVVAMOS* GetVAMOS()
   {
      return (KVVAMOS*)GetStructure("VAMOS");
   }
   KVINDRA* GetINDRA()
   {
      return (KVINDRA*)GetStructure("INDRA");
   }

   ClassDef(KVINDRA_VAMOS, 1) //INDRA + VAMOS experimental setup for the e494s and the e503 experiments performed at GANIL
};

#endif
