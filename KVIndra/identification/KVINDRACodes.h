//$Id: KVINDRACodes.h,v 1.4 2009/04/06 15:05:55 franklan Exp $
//Author: $Author: franklan $

#ifndef __KVINDRACODES__
#define __KVINDRACODES__

#include "TObject.h"
#include "KVINDRACodeMask.h"

class KVINDRACodes: public KVINDRACodeMask {

   friend class KVINDRAReconNuc;

private:
   static Char_t fCodeGenIdent[14][120];
   static Char_t fCodeGenCalib[8][120];
   static UChar_t fIDCodes[14];
   static UChar_t fECodes[8];
   static UShort_t fIDBits[16];
   static UChar_t fEBits[16];

public:

   enum {
      kIsoRes = BIT(14)         //isotopic resolution achieved i.e. mass was measured
   };

   KVINDRACodes();
   virtual ~ KVINDRACodes() {}
   const Char_t* GetIDStatus();
   const Char_t* GetEStatus();

   static UShort_t VedaIDCodeToBitmask(UChar_t veda_id_code);
   static UChar_t VedaECodeToBitmask(UChar_t veda_e_code);

   UChar_t GetCodeIndex(UShort_t);
   UChar_t GetVedaIDCode();
   UChar_t GetVedaECode();
   void SetVedaIDCode(UChar_t);
   void SetVedaECode(UChar_t);
   UShort_t GetIDCode();
   UChar_t GetECode();
   void SetIDCode(UShort_t);
   void SetECode(UChar_t);


   void Clear(Option_t* opt = "");

   ClassDef(KVINDRACodes, 3)    //Particle identification and calibration status (Veda6)
};

#endif
