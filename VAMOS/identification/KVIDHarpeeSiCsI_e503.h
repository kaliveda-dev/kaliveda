#ifndef __KVIDHARPEESICSI_E503_H__
#define __KVIDHARPEESICSI_E503_H__

// C++
#include <cassert>

// KaliVeda (Standard)
#include "KVRList.h"

// KaliVeda (VAMOS)
#include "KVHarpeeSi.h"
#include "KVIDHarpeeSiCsI.h"
#include "KVVAMOSCodes.h"
#include "SiliconEnergyMinimiser.h"

class KVIDHarpeeSiCsI_e503 : public KVIDHarpeeSiCsI {

#if __cplusplus < 201103L
   KVList* records_;
   KVIdentificationResult* base_id_result_;
   SiliconEnergyMinimiser* minimiser_;
#else
   std::shared_ptr<KVList> records_;
   std::shared_ptr<KVIdentificationResult> base_id_result_;
   std::shared_ptr<SiliconEnergyMinimiser> minimiser_;
#endif

   Bool_t kInitialised_;

   enum VIDSubCode {
      kIdentified,
      kBaseIdentZFailed,
      kEstimateAFailed,
   };

public:

   KVIDHarpeeSiCsI_e503();
   virtual ~KVIDHarpeeSiCsI_e503();
   virtual void Copy(TObject&) const;

   virtual Bool_t Init();
   virtual Bool_t Identify(
      KVIdentificationResult* idr,
      Double_t x = -1,
      Double_t y = -1
   );

   virtual Bool_t  SetIDCorrectionParameters(const KVRList* const records);
   virtual const KVList* GetIDCorrectionParameters() const;

   ClassDef(KVIDHarpeeSiCsI_e503, 1) // KVIDHarpeeSiCsI_e503
};

#endif

