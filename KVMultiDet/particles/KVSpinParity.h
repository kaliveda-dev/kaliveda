//Created by KVClassFactory on Thu Jan 27 17:23:54 2011
//Author: Eric Bonnet,,,

#ifndef __KVSpinParity_H
#define __KVSpinParity_H

#include "KVNuclData.h"

/**
\class KVSpinParity
\brief Spin parity assignment of nuclear levels
\ingroup NucProp
*/

class KVSpinParity : public KVNuclData {

protected:
   /*
   TObject's bits can be used as flags, bits 0 - 13 and 24-31 are
   reserved as  global bits while bits 14 - 23 can be used in different
   bit 14 used by KVNuclData class
   */
   enum EStatusBits {
      kMultiple = BIT(15)  //several possible values (1) / just one (0)
   };


   void init();

public:
   KVSpinParity();
   KVSpinParity(const Char_t* name);
   virtual ~KVSpinParity();

   void SetMultiple(Bool_t multiple)
   {
      SetBit(kMultiple, multiple);
   }
   Bool_t IsMultiple() const
   {
      return TestBit(kMultiple);
   }

   ClassDef(KVSpinParity, 1) //Simple class for storing life time information of nucleus
};

#endif
