//$Id: KVINDRACodeMask.h,v 1.4 2008/02/14 10:30:18 franklan Exp $
//Author: $Author: franklan $

#ifndef __KVINDRACODEMASK__
#define __KVINDRACODEMASK__

#include "TObject.h"

////////////////////////////////////////////////////////////////////////////////////////

/* Global bits for code masks */
enum EVedaCodes {
   kIDCode0 = BIT(0),           //these bits concern KVINDRACodeMask::fIDMask
   kIDCode1 = BIT(1),
   kIDCode2 = BIT(2),
   kIDCode3 = BIT(3),
   kIDCode4 = BIT(4),
   kIDCode5 = BIT(5),
   kIDCode6 = BIT(6),
   kIDCode7 = BIT(7),
   kIDCode8 = BIT(8),
   kIDCode9 = BIT(9),
   kIDCode10 = BIT(10),
   kIDCode11 = BIT(11),
   kIDCode14 = BIT(12),
   kIDCode15 = BIT(13),
   kECode0 = BIT(0),            //these bits concern KVINDRACodeMask::fEMask
   kECode1 = BIT(1),
   kECode2 = BIT(2),
   kECode3 = BIT(3),
   kECode11 = BIT(4),
   kECode15 = BIT(5),
   kECode4 = BIT(6),
   kECode5 = BIT(7)
};

enum EIDCodes {
   kIDCode_Gamma = kIDCode0,
   kIDCode_Neutron = kIDCode1,
   kIDCode_CsI = kIDCode2,
   kIDCode_Phoswich = kIDCode2,
   kIDCode_SiCsI = kIDCode3,
   kIDCode_SiLiCsI = kIDCode3,
   kIDCode_Si75SiLi = kIDCode3,
   kIDCode_ChIoSi = kIDCode4,
   kIDCode_ChIoCsI = kIDCode4,
   kIDCode_ChIoSi75 = kIDCode4,
   kIDCode_Bragg = kIDCode5,
   kIDCode_Zmin = kIDCode5,
   kIDCode_ZminCsI = kIDCode9,
   kIDCode_ArretChIo = kIDCode5,
   kIDCode_ArretSi = kIDCode5,
   kIDCode_MultihitChIo = kIDCode8
};

class KVINDRACodeMask: public TObject {

private:
   UShort_t fIDMask;            //16-bit mask with id codes
   UChar_t fEMask;              //8-bit mask with e-calibration codes

public:

   KVINDRACodeMask()
   {
      //Default ctor - no mask defined
      fIDMask = 0;
      fEMask = 0;
   };
   virtual ~ KVINDRACodeMask()
   {
      //Default dtor
      fIDMask = 0;
      fEMask = 0;
   };

   UShort_t GetIDMask()
   {
      return fIDMask;
   }
   UChar_t GetEMask()
   {
      return fEMask;
   }
   void SetIDMask(UShort_t codes)
   {
      fIDMask = codes;
   }
   Bool_t TestIDCode(UShort_t code);
   Bool_t TestECode(UChar_t code);
   void SetEMask(UChar_t codes)
   {
      fEMask = codes;
   }

   Bool_t operator&(KVINDRACodeMask& msk)
   {
      return (TestIDCode(msk.GetIDMask()) && TestECode(msk.GetEMask()));
   };

   void Clear(Option_t* = "")
   {
      fIDMask = 0;
      fEMask = 0;
      TObject::Clear();
   };

   ClassDef(KVINDRACodeMask, 1);        //Handles setting and testing code masks for particle calibration quality control
};


#endif
