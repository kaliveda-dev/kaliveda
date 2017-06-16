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

   Int_t GetAMinimiser()
   {
      return Amin_;   //A from minimiser
   }

   Int_t GetBaseQualityCode()
   {
      return ICode_;   //Quality code from KVIDZAGrid
   }

   Double_t GetBasePID()
   {
      return base_id_result_->PID;
   }

   Int_t GetQualityCode()
   {
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
      //ID was OK
      kMCode0, //Z found with grid, A from minimiser found
      //ID wasn't OK
      kMCode1, //Z not found from grid (base_id_result_::Zident=kFALSE)
      kMCode2, //Z found from grid, A from minimiser not found
   };

   Int_t Amin_;  //Mass from minimiser
   Int_t ICode_; //IDquality code from basic KVIDZAGrid identification
   Int_t MCode_; //KVIDHarpeeSiCsI_e503 specific IDquality code

   ClassDef(KVIDHarpeeSiCsI_e503, 1) // KVIDHarpeeSiCsI_e503
};

#endif

