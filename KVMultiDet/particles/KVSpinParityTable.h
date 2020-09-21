//Created by KVClassFactory on Wed Feb 23 16:10:14 2011
//Author: bonnet

#ifndef __KVSpinParityTABLE_H
#define __KVSpinParityTABLE_H

#include "KVNuclDataTable.h"
#include "KVSpinParity.h"

/**
\class KVSpinParityTable
\brief Table of spin parity values for nuclear levels
\ingroup NucProp
 */

class KVSpinParityTable : public KVNuclDataTable {

protected:

   void init();

public:
   KVSpinParityTable();
   virtual ~KVSpinParityTable();

   virtual void Initialize();

   KVSpinParity* GetSpinParity(Int_t zz, Int_t aa) const;
   Bool_t  IsMultiple(Int_t zz, Int_t aa) const;


   ClassDef(KVSpinParityTable, 1) //Store available values on spin and parity of GS
};

#endif
