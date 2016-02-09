//Created by KVClassFactory on Thu Feb 24 11:24:52 2011
//Author: bonnet

#ifndef __KVNDTMANAGER_H
#define __KVNDTMANAGER_H

#include "KVList.h"

class KVNuclDataTable;
class KVNuclData;
class TObjArray;

class KVNDTManager : public KVList {

protected:
   void init();
   TObjArray* Arange;
   TObjArray* Zrange;


public:
   KVNDTManager();
   virtual ~KVNDTManager();

   KVNuclDataTable* GetTable(const Char_t* name) const;

   Bool_t IsInTable(Int_t zz, Int_t aa, const Char_t* name) const;
   Double_t GetValue(Int_t zz, Int_t aa, const Char_t* name) const;
   void SetValue(Int_t zz, Int_t aa, const Char_t* name, Double_t val);
   KVNuclData* GetData(Int_t zz, Int_t aa, const Char_t* name) const;
   Bool_t IsMeasured(Int_t zz, Int_t aa, const Char_t* name) const;
   const Char_t* GetUnit(Int_t zz, Int_t aa, const Char_t* name) const;
   void PrintTables() const;

   ClassDef(KVNDTManager, 1) //Allow to navigate between different tables of nuclear data
};

//................  global variable
R__EXTERN KVNDTManager* gNDTManager;

#endif
