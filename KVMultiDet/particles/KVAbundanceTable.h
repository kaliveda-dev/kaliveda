//Created by KVClassFactory on Wed Feb 23 16:10:14 2011
//Author: bonnet

#ifndef __KVABUNDANCETABLE_H
#define __KVABUNDANCETABLE_H

#include "KVNuclDataTable.h"
#include "KVAbundance.h"

class KVAbundanceTable : public KVNuclDataTable {

protected:

   void init();

public:
   KVAbundanceTable();
   virtual ~KVAbundanceTable();
   /*
   TObjArray* tobj_rangeA;
   TObjArray* tobj_rangeZ;
   */
   virtual void Initialize();

   KVAbundance* GetAbundance(Int_t zz, Int_t aa) const;


   ClassDef(KVAbundanceTable, 1) //Store available values of the relative abundance
};

#endif
