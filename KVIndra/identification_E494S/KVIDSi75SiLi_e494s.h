//Created by KVClassFactory on Fri Nov  2 14:11:36 2012
//Author: Guilain ADEMARD

#ifndef __KVIDSI75SILI_E494S_H
#define __KVIDSI75SILI_E494S_H

#include "KVIDSi75SiLi.h"
#include "KVRTGIDManager.h"
#include "KVINDRADetector.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVIDSi75SiLi_e494s : public KVIDSi75SiLi, public KVRTGIDManager {

protected:

   KVINDRADetector* fSi75; //!
   KVINDRADetector* fSiLi; //!
   Double_t fThresholdX; //!
   Double_t fThresholdY; //!

   Double_t GetThesholdFromVar(const Char_t* var);
   Double_t GetIDMapXY(KVINDRADetector* det, Option_t* opt = "");

public:
   KVIDSi75SiLi_e494s();
   virtual ~KVIDSi75SiLi_e494s();

   virtual void Initialize();

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   virtual Bool_t Identify(KVIdentificationResult* IDR, Double_t x = -1., Double_t y = -1.);

   virtual Bool_t SetIdentificationParameters(const KVMultiDetArray*);
   virtual void RemoveIdentificationParameters();

   ClassDef(KVIDSi75SiLi_e494s, 1) //E503/E494S experiment INDRA identification using Si75-SiLi matrices
};

#endif
