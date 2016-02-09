//Created by KVClassFactory on Fri Jan 23 15:43:30 2015
//Author: ademard

#ifndef __KVIDQA_H
#define __KVIDQA_H

#include "KVVAMOSIDTelescope.h"
#include "KVIDQAGrid.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVVAMOSReconNuc;

class KVIDQA : public KVVAMOSIDTelescope {

   KVIDQAGrid* fQAGrid; //!

public:
   KVIDQA();
   virtual ~KVIDQA();

   virtual const Char_t* GetArrayName();

   virtual Double_t GetIDMapX(Option_t* opt = "")
   {
      UNUSED(opt);
      return 0.;
   }
   virtual Double_t GetIDMapY(Option_t* opt = "")
   {
      UNUSED(opt);
      return 0.;
   }

   virtual Bool_t Identify(KVIdentificationResult* IDR, Double_t x = -1., Double_t y = -1.);
   virtual Bool_t Identify(KVIdentificationResult* IDR, const Char_t* tof_name, Double_t realAoQ, Double_t realA);
   virtual void Initialize();

   Double_t GetRealQ() const
   {
      return (fQAGrid ? fQAGrid->GetRealQ() : 0.);
   }


   ClassDef(KVIDQA, 1) //ID telescope used to identify Q and A with VAMOS
};

#endif
