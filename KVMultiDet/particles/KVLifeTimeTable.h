//Created by KVClassFactory on Wed Feb  9 13:04:29 2011
//Author: bonnet

#ifndef __KVLIFETIMETABLE_H
#define __KVLIFETIMETABLE_H

#include "KVNuclDataTable.h"
#include "KVNameValueList.h"
#include "KVLifeTime.h"

class KVLifeTimeTable : public KVNuclDataTable {

protected:

   virtual void init();
   Double_t ToMeV(TString unit);
   Double_t ToSecond(TString unit);

   Int_t kNu_t;            //!number of possible time units
   Double_t conversion_t[15][15];   //matrice kNu_t*kNu_t to convert a life time from one unit to an other one

   Int_t kNu_e;            //!number of possible energy units (resonnance)
   Double_t conversion_e[15][15];   // matrice kNu_e*kNu_e to convert
   // an energy resonnance from one unit to an other one
   KVString list_t, list_vt;
   KVString list_e, list_ve;

   KVNameValueList lu_t;
   KVNameValueList lu_e;

public:
   KVLifeTimeTable();
   virtual ~KVLifeTimeTable();

   virtual void Initialize();

   KVLifeTime* GetLifeTime(Int_t zz, Int_t aa) const;
   Bool_t  IsAResonance(Int_t zz, Int_t aa) const;


   ClassDef(KVLifeTimeTable, 1) //Store life time information of nuclei

};

#endif
