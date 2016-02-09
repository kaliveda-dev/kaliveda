//Created by KVClassFactory on Fri Feb 11 10:36:24 2011
//Author: bonnet

#ifndef __KVMASSEXCESSTABLE_H
#define __KVMASSEXCESSTABLE_H

#include "KVNuclDataTable.h"
#include "KVMassExcess.h"

class KVMassExcessTable : public KVNuclDataTable {

protected:

   void init();

public:
   KVMassExcessTable();
   virtual ~KVMassExcessTable();

   virtual void Initialize();

   KVMassExcess* GetMassExcess(Int_t zz, Int_t aa) const;

   ClassDef(KVMassExcessTable, 1) //manage mass excess data for nuclei
};

#endif
