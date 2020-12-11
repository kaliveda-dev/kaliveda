//$Id: KVINDRACodes.h,v 1.4 2009/04/06 15:05:55 franklan Exp $
//Author: $Author: franklan $

#ifndef __KVINDRACODES__
#define __KVINDRACODES__

#include "TObject.h"
#include "KVINDRACodeMask.h"

/**
  \class KVINDRACodes
  \ingroup INDRAIdent
  \brief Identification code structure for INDRA data (obsolete)

Describes the identification and calibration status of reconstructed particles.
Based on the codes figuring in veda_6.incl of the old VEDA programme.
~~~~{.cpp}
GetVedaIDCode()  -  returns ID code in VEDA format (0 = gamma, 2 = id CsI/Phoswich, etc.)
GetVedaECode()   -  returns energy calibration code in VEDA format (1 =  calibration sans probleme, etc.)

GetIDStatus()    -  returns a string describing the identification status
                    e.g. if GetVedaIDCode()=0 then GetIDStatus()="gamma"
GetEStatus()     -  returns a string describing the calibration status
                    e.g. if GetVedaECode()=0 then GetEStatus()="Aucun appel a une routine de calibration effectuee"
~~~~
WARNING: the methods GetIDCode() and GetECode() return the new bit-mask status codes defined in
         class KVINDRACodeMask.

If you prefer to use these masks, the best way to test the status of the particles is by
using KVINDRACodeMask::TestIDCode and KVINDRACodeMask::TestECode

e.g. to test if a particle was identified using CsI R-L you have three choices
(supposing the particle is referenced by KVINDRAReconNuc* part):
~~~~{.cpp}
   EITHER   if( part->GetCodes().GetVedaIDCode() == 2 ) { ... }
   OR       if( part->GetCodes().TestIDCode( kIDCode2 ) ) { ... }
   OR       if( part->GetCodes().TestIDCode( kIDCode_CsI ) ) { ... ]
~~~~
Obviously, the third method is to be preferred (it is immediately clear what the instruction means)!!
*/
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
