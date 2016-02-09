//Created by KVClassFactory on Tue Mar 12 15:08:25 2013
//Author: Guilain ADEMARD

#ifndef __KVVAMOSCODES_H
#define __KVVAMOSCODES_H

#include "KVINDRACodes.h"
#include "KVDataSet.h"

/* Global bits for code masks */
enum FocalPosCodes {
   kFPCode0 = BIT(0),           //these bits concern KVCodes::fFPMask
   kFPCode1 = BIT(1),
   kFPCode2 = BIT(2),
   kFPCode3 = BIT(3),
   kFPCode4 = BIT(4),
   kFPCode5 = BIT(5),
   kFPCode6 = BIT(6),
   kFPCode7 = BIT(7),
   kFPCode8 = BIT(8),
   kFPCode9 = BIT(9),
   kFPCode10 = BIT(10),
   kFPCode11 = BIT(11),
   kFPCode12 = BIT(12),
   kFPCode13 = BIT(13),
   kFPCode14 = BIT(14),
   kFPCode15 = BIT(15),
   kFPCode16 = BIT(16),
   kFPCode17 = BIT(17),
   kFPCode18 = BIT(18),
   kFPCode19 = BIT(19),
   kFPCode20 = BIT(20),
   kFPCode21 = BIT(21),
   kFPCode22 = BIT(22),
   kFPCode23 = BIT(23),
   kFPCode24 = BIT(24),
   kFPCode25 = BIT(25),
   kFPCode26 = BIT(26),
   kFPCode27 = BIT(27),
   kFPCode28 = BIT(28),
   kFPCode29 = BIT(29),
   kFPCode30 = BIT(30),
   kFPCode31 = BIT(31)
};

enum TimeOfFlightCodes {
   kTCode0 = BIT(0),           //these bits concern KVCodes::fTMask
   kTCode1 = BIT(1),
   kTCode2 = BIT(2),
   kTCode3 = BIT(3),
   kTCode4 = BIT(4),
   kTCode5 = BIT(5),
   kTCode6 = BIT(6),
   kTCode7 = BIT(7),
   kTCode8 = BIT(8),
   kTCode9 = BIT(9),
   kTCode10 = BIT(10),
   kTCode11 = BIT(11),
   kTCode12 = BIT(12),
   kTCode13 = BIT(13),
   kTCode14 = BIT(14),
   kTCode15 = BIT(15)
};



class KVVAMOSCodes : public KVINDRACodes {

private:

   static KVDataSet* fDataSet;//Dataset associated to codes
   static UChar_t    fNFPdets;//Number of detectors used for FP reconstruction
   static UChar_t    fNToF;//Number of Times of Flight (ACQ parameters)
   static Char_t**   fFPdetNames;//Names of the FP recon. detectors (array of size = fNFPdet)
   static Char_t**   fToFNames;//Names of the Times of Flight (array of size = fNToF)
   static Char_t**   fCodeGenFPRecon;//Array of explanations for FP reconstruction
   static Char_t**   fCodeGenToF;//Array of explanations for Time of Flight codes

   UInt_t            fFPMask;    //32-bit mask with Focal plan Position code
   UShort_t          fTMask;     //16-bit mask with Time of Flight code

protected:

   static void   BuildFPReconCodes();
   static void   BuildTimeOfFlightCodes();
   static void   DeleteFPReconCodes();
   static void   DeleteTimeOfFlightCodes();
   static Bool_t RefreshCodes();
   static Bool_t RefreshFPCodes();
   static Bool_t RefreshTCodes();

public:

   KVVAMOSCodes();
   virtual ~KVVAMOSCodes();

   UInt_t GetFPMask()
   {
      return fFPMask;
   }

   UShort_t GetTMask()
   {
      return fTMask;
   }

   const Char_t* GetFPStatus();
   const Char_t* GetTStatus();
   UInt_t   GetFPCode();
   UShort_t GetTCode();
   static UShort_t GetTCode(const Char_t*);
   void     SetFPCode(UInt_t);
   void     SetFPCode(Int_t nc1, Int_t nc2, Int_t ni1 = -1, Int_t ni2 = -1, Bool_t ni1x = kTRUE);
   void     SetTCode(UShort_t);
   void     SetTCode(const Char_t*);
   UChar_t  GetFPCodeIndex(UInt_t);
   UChar_t  GetTCodeIndex(UShort_t);
   UChar_t  GetFPCodeIndex();
   UChar_t  GetTCodeIndex();
   void     SetFPCodeFromIndex(Int_t);
   void     SetTCodeFromIndex(Int_t);

   void SetFPMask(UInt_t codes)
   {
      fFPMask = codes;
   }

   void SetTMask(UShort_t codes)
   {
      fTMask  = codes;
   }

   Bool_t TestFPCode(UInt_t code);
   Bool_t TestTCode(UShort_t code);

   Bool_t operator&(KVVAMOSCodes& msk)
   {
      return (TestTCode(msk.GetTMask()) && TestFPCode(msk.GetFPMask()) && TestIDCode(msk.GetIDMask()) && TestECode(msk.GetEMask()));
   }

   virtual void Clear(Option_t* opt = "")
   {
      KVINDRACodes::Clear(opt);
      fFPMask = 0;
      fTMask  = 0;
   }

   static Char_t* GetFPdetName(Short_t i)
   {
      if (RefreshCodes() && (i < fNFPdets)) return fFPdetNames[i];
      return NULL;
   }

   static Char_t* GetToFName(Short_t i)
   {
      if (RefreshCodes() && (i < fNToF)) return fToFNames[i];
      return NULL;
   }

   static void ShowAvailableFPCodes();
   static void ShowAvailableTCodes();

   static UChar_t GetNFPCodes()
   {
      //Returns the number of FP codes.
      return 1 + (fNFPdets * (fNFPdets - 1) * (2 * fNFPdets - 3)) / 2;
   }
   static UChar_t GetNTCodes()
   {
      //Returns the number of T codes.
      return 1 + fNToF;
   }


   ClassDef(KVVAMOSCodes, 1) //Status for calibration, trajectory reconstruction, identification and time of flight in VAMOS
};

#endif
