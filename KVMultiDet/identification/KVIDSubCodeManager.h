//$Id: KVIDSubCodeManager.h,v 1.3 2007/03/26 10:18:06 franklan Exp $

#ifndef __KVIDSubCodeManager
#define __KVIDSubCodeManager

#include "Rtypes.h"
#include "TString.h"
#include "Binary_t.h"

typedef Binary16_t KVIDSubCode;

class KVIDSubCodeManager {
   UChar_t fNbits;              //number of bits used to store ID routine code de retour
   UChar_t fMSB;                //number of the Most Significant Bit used to store code de retour (LSB = bit 0)

   // OBSOLETE
   inline virtual Int_t GetIDSubCode(KVIDSubCode& concat) const;
   virtual const Char_t* GetIDSubCodeString(KVIDSubCode& concat) const;

public:

   KVIDSubCodeManager(UChar_t nbits = 16, UChar_t msb = 15);
   virtual ~ KVIDSubCodeManager()
   {
   };

   inline void SetNbits(UChar_t n)
   {
      fNbits = n;
   };
   inline void SetMSB(UChar_t n)
   {
      fMSB = n;
   };

   inline void SetIDSubCode(KVIDSubCode& concat, UShort_t subcod) const;

   inline void PrintSCM(Option_t* opt = "") const;

   inline virtual const Char_t* SubCodeNotSet() const;

   ClassDef(KVIDSubCodeManager, 1)      //Handles the concatenation of identification routine codes
};

inline void KVIDSubCodeManager::SetIDSubCode(KVIDSubCode& concat,
      UShort_t subcod) const
{
   //Set the relevant bits of the binary number 'concat' to the value of 'subcod'
   concat.WriteSubvalue(subcod, fMSB, fNbits);
}

inline Int_t KVIDSubCodeManager::GetIDSubCode(KVIDSubCode& concat) const
{
   //Returns value of subcode written in relevant bits of 'concat'
   //If all bits of subcode = '1' it means no subcode has been set: we return -1
   Int_t sbv = (Int_t) concat.Subvalue(fMSB, fNbits);
   //compare with max value of fNBits bits
   if (sbv == ((1 << fNbits) - 1))
      return -1;
   return sbv;
}

inline void KVIDSubCodeManager::PrintSCM(Option_t*) const
{
   std::cout << "KVIDSubCodeManager: fNbits=" << (int) fNbits << " fMSB=" <<
             (int) fMSB << std::endl;
}

inline const Char_t* KVIDSubCodeManager::SubCodeNotSet() const
{
   return Form("----");
}
#endif
