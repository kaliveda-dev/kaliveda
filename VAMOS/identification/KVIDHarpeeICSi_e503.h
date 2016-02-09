//Created by KVClassFactory on Fri Dec 13 11:56:00 2013
//Author: Guilain ADEMARD

#ifndef __KVIDHARPEEICSI_E503_H
#define __KVIDHARPEEICSI_E503_H

// C++
#include <cassert>

// KaliVeda (Standard)
#include "KVFunctionCal.h"
#include "KVRList.h"

// KaliVeda (VAMOS)
#include "KVHarpeeIC.h"
#include "KVIDHarpeeICSi.h"
#include "KVVAMOSCodes.h"

class KVIDHarpeeICSi_e503 : public KVIDHarpeeICSi {
#if __cplusplus < 201103L
   KVList* records_;
#else
   std::shared_ptr<KVList> records_;
#endif
   KVIDZAGrid* FindGridForSegment(Int_t num);

   Bool_t kInitialised_;

   enum VIDSubCode {
      kSegmentMisfire = -1
   };

public:

   KVIDHarpeeICSi_e503();
   virtual ~KVIDHarpeeICSi_e503();

   virtual Bool_t   Init();
   virtual Double_t GetIDMapY(Option_t* opt = "");

   virtual Bool_t Identify(
      KVIdentificationResult* idr,
      Double_t x = -1,
      Double_t y = -1
   );

   virtual Bool_t  SetIDCorrectionParameters(const KVRList* const records);
   virtual const KVList* GetIDCorrectionParameters() const;

   ClassDef(KVIDHarpeeICSi_e503, 1)
};

#endif

