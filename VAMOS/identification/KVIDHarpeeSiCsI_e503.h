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

public:

   KVIDHarpeeSiCsI_e503();
   virtual ~KVIDHarpeeSiCsI_e503();
   virtual void Copy(TObject&) const;

   virtual Bool_t Init();

   // The identification function relies on the base class identification
   // routine (KVIDHarpeeSiCsI::Identify) to perform the Z identification, using
   // a KVIDZAGrid. It then estimates the A value using the
   // SiliconEnergyMinimiser class (by minimising the difference between
   // simulated and real silicon energies).
   virtual Bool_t Identify(
      KVIdentificationResult* idr,
      Double_t x = -1,
      Double_t y = -1
   );

   // This function adds all the identification correction parameter records
   // specified in 'records' to the private member KVRList 'records_';
   virtual Bool_t  SetIDCorrectionParameters(const KVRList* const records);
   virtual const KVList* GetIDCorrectionParameters() const;

   Int_t GetAMinimiser() {
      return Amin_;   // Mass from minimisation
   }
   Int_t GetBaseQualityCode() {
      return ICode_;   //Quality code from KVIDZAGrid
   }
   Int_t GetQualityCode() {
      return MCode_;   //Global returned quality code
   }

private:

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
      kMCode0, //(Z,A) identification from KVIDZAGrid + A from minimiser are OK and equal
      kMCode1, //(Z,A) identification from KVIDZAGrid + A from minimiser are OK but different
      kMCode2, //Z from KVIDZAGrid is OK but A not; A from minimiser is OK
      kMCode3, //(Z,A) from KVIDZAGrid are OK; A from minimiser is not OK
      kMCode4,  //mass not found either from KVIDZAGrid or minimiser
      kMCode5, //Z from KVIDZAGrid not OK (base_id_result_::Zident=kFALSE)
   };

   Int_t Amin_; //Mass obtained from CsI energy minimiser
   Int_t ICode_; //IDquality code from basic KVIDZAGrid identification
   Int_t MCode_; //KVIDHarpeeSiCsI_e503 specific IDquality code

   ClassDef(KVIDHarpeeSiCsI_e503, 1) // KVIDHarpeeSiCsI_e503
};

#endif

