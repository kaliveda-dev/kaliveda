//Created by KVClassFactory on Mon Mar 18 10:44:28 2013
//Author: Guilain ADEMARD

#ifndef __KVIVRECONEVENT_H
#define __KVIVRECONEVENT_H

#include "KVINDRAReconEvent.h"
#include "KVVAMOSReconEvent.h"

class KVIVReconEvent : public KVINDRAReconEvent {

protected:

   KVVAMOSReconEvent* fVAMOSev; //->VAMOS event

public:

   KVIVReconEvent(Int_t mult_indra = 50, const char* cl_name_indra = "KVINDRAReconNuc", Int_t mult_vamos = 5, const char* cl_name_vamos = "KVVAMOSReconNuc");

   void init();
   virtual ~KVIVReconEvent();

   virtual void     Clear(Option_t* opt = "");
   virtual Int_t    GetTotalMult(Option_t* opt = "");
   virtual void     Print(Option_t* option = "") const;


   void AcceptECodesINDRA(UChar_t  code)
   {
      AcceptECodes(code);
   }
   void AcceptECodesVAMOS(UChar_t  code)
   {
      fVAMOSev->AcceptECodes(code);
   }
   void AcceptFPCodesVAMOS(UInt_t code)
   {
      fVAMOSev->AcceptFPCodes(code);
   }
   void AcceptIDCodesINDRA(UShort_t code)
   {
      AcceptIDCodes(code);
   }
   void AcceptIDCodesVAMOS(UShort_t code)
   {
      fVAMOSev->AcceptIDCodes(code);
   }
   void AcceptTCodesVAMOS(UShort_t code)
   {
      fVAMOSev->AcceptTCodes(code);
   }

   Bool_t CheckINDRACodes(KVINDRACodeMask& code)
   {
      //returns kTRUE if "code" is compatible with INDRA event's code mask
      //if no code mask set for event, returns kTRUE in all cases
      return CheckCodes(code);
   }

   Bool_t CheckVAMOSCodes(KVVAMOSCodes& code)
   {
      //returns kTRUE if "code" is compatible with VAMOS event's code mask.
      //If no code mask set for event, returns kTRUE in all cases
      return fVAMOSev->CheckCodes(code);
   }

   void  CalibrateVAMOSEvent()
   {
      fVAMOSev->CalibrateEvent();
   }

   KVVAMOSReconNuc* GetNextNucleus(Option_t* opt = "")
   {
      return fVAMOSev->GetNextNucleus(opt);
   }

   KVVAMOSReconNuc* GetNucleus(Int_t n_nuc) const
   {
      return fVAMOSev->GetNucleus(n_nuc);
   }
   KVVAMOSReconEvent* GetVAMOSEvent() const
   {
      return fVAMOSev;
   }

   Int_t GetINDRAMult(Option_t* opt = "")
   {
      return KVINDRAReconEvent::GetMult(opt);
   }

   Int_t GetVAMOSMult(Option_t* opt = "")
   {
      return fVAMOSev->GetMult(opt);
   }
   void  IdentifyVAMOSEvent_A()
   {
      fVAMOSev->IdentifyEvent_A();
   }

   void  IdentifyVAMOSEvent_Z()
   {
      fVAMOSev->IdentifyEvent_Z();
   }

   void ResetGetNextNucleus()
   {
      fVAMOSev->ResetGetNextNucleus();
   }

   void  IdentAndCalibVAMOSEvent()
   {
      fVAMOSev->IdentAndCalibEvent();
   }

   void  ReconstructVAMOSEvent(KVMultiDetArray* mda, KVDetectorEvent* kvde)
   {
      fVAMOSev->ReconstructEvent(mda, kvde);
   }

   virtual void SetNumber(UInt_t num)
   {
      KVINDRAReconEvent::SetNumber(num);
      fVAMOSev->SetNumber(num);
   }

   virtual void SetPartSeedCond(const Char_t* cond)
   {
      KVINDRAReconEvent::SetPartSeedCond(cond);
      fVAMOSev->SetPartSeedCond(cond);
   }

   ClassDef(KVIVReconEvent, 1) //Event reconstructed from energy losses in INDRA array and VAMOS spectrometer
};

#endif
