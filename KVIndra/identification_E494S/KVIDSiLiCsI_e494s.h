//Created by KVClassFactory on Fri Nov  2 15:30:21 2012
//Author: Guilain ADEMARD

#ifndef __KVIDSILICSI_E494S_H
#define __KVIDSILICSI_E494S_H

#include "KVIDSiLiCsI.h"
#include "KVRTGIDManager.h"
#include "KVINDRADetector.h"
#include "KVCsI.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVIDSiLiCsI_e494s : public KVIDSiLiCsI, public KVRTGIDManager {
protected:

   KVINDRADetector* fSiLi; //!
   KVCsI*           fCsI;  //!
   Double_t fThresholdX; //!
   Double_t fThresholdY; //!

   Double_t GetThesholdFromVar(const Char_t* var);

public:
   KVIDSiLiCsI_e494s();
   virtual ~KVIDSiLiCsI_e494s();

   virtual void Initialize();

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   virtual Bool_t Identify(KVIdentificationResult* IDR, Double_t x = -1., Double_t y = -1.);

   virtual Bool_t SetIdentificationParameters(const KVMultiDetArray*);
   virtual void RemoveIdentificationParameters();

   ClassDef(KVIDSiLiCsI_e494s, 1) //E503/E494S experiment INDRA identification using SiLi-CsI matrices
};

#endif
