//Created by KVClassFactory on Wed Jan 30 12:13:08 2013
//Author: bonnet

#ifndef __KVCHARGERADIUSTABLE_H
#define __KVCHARGERADIUSTABLE_H

#include "KVNuclDataTable.h"
#include "KVChargeRadius.h"

class KVChargeRadiusTable : public KVNuclDataTable {
protected:
   virtual void init();

public:
   KVChargeRadiusTable();
   virtual ~KVChargeRadiusTable();

   virtual void Initialize();

   KVChargeRadius* GetChargeRadius(Int_t zz, Int_t aa) const;

   ClassDef(KVChargeRadiusTable, 1) ////Store charge radius information for nuclei
};

#endif
