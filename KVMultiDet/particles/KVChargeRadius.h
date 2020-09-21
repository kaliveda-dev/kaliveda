//Created by KVClassFactory on Wed Jan 30 12:13:12 2013
//Author: bonnet

#ifndef __KVCHARGERADIUS_H
#define __KVCHARGERADIUS_H

#include "KVNuclData.h"

/**
\class KVChargeRadius
\brief Simple class for storing charge radius information of nuclei
\ingroup NucProp
*/

class KVChargeRadius : public KVNuclData {
   void init();

protected:
   Double_t fError;

public:
   KVChargeRadius();
   KVChargeRadius(const Char_t* name);
   KVChargeRadius(const KVChargeRadius&);
   ROOT_COPY_ASSIGN_OP(KVChargeRadius)
   void Copy(TObject& object) const;
   virtual ~KVChargeRadius();

   void SetError(Double_t err)
   {
      fError = err;
   }
   Double_t GetError() const
   {
      return fError;
   }

   ClassDef(KVChargeRadius, 1) ////Simple class for store charge radius information of nucleus
};

#endif
