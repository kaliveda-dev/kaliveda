//Created by KVClassFactory on Thu Jun 13 08:24:29 2013
//Author: Guilain ADEMARD

#ifndef __KVVAMOSRECONEVENT_H
#define __KVVAMOSRECONEVENT_H

#include "KVReconstructedEvent.h"
#include "KVVAMOSCodes.h"

class KVVAMOSReconNuc;
class KVDetectorEvent;

class KVVAMOSReconEvent : public KVReconstructedEvent {

private:

   KVVAMOSCodes* fCodeMask; //!codes accepted for "good" VAMOS nucleus (IsOK)
   inline KVVAMOSCodes* GetCodeMask()
   {
      if (!fCodeMask)
         fCodeMask = new KVVAMOSCodes;
      return fCodeMask;
   };


public:

   KVVAMOSReconEvent(Int_t mult = 5, const char* classname = "KVVAMOSReconNuc");
   void init();
   virtual ~KVVAMOSReconEvent();



   void     AcceptECodes(UChar_t  code);
   void     AcceptFPCodes(UInt_t   code);
   void     AcceptIDCodes(UShort_t code);
   void     AcceptTCodes(UShort_t code);
   KVVAMOSReconNuc*    GetNextNucleus(Option_t* opt = "");
   KVVAMOSReconNuc*    GetNucleus(Int_t n_nuc) const;
   virtual void     IdentifyEvent_A();
   virtual void     IdentifyEvent_Z();
   virtual void     IdentAndCalibEvent();
   virtual Bool_t   IsOK();
   virtual void     Print(Option_t* option = "") const;
   virtual void     ReconstructEvent(KVMultiDetArray*, KVDetectorEvent* kvde);


   inline Bool_t CheckCodes(KVVAMOSCodes& code)
   {
      //returns kTRUE if "code" is compatible with VAMOS event's code mask.
      //If no code mask set for event, returns kTRUE in all cases
      if (!fCodeMask)
         return kTRUE;
      return ((*fCodeMask) & code);
   }

   inline void ResetGetNextNucleus()
   {
      ResetGetNextParticle();
   }


   ClassDef(KVVAMOSReconEvent, 1) //Event reconstructed from energy losses in VAMOS spectrometer
};

#endif
