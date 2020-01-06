//Created by KVClassFactory on Fri Feb 21 10:48:23 2014
//Author: Rivet Marie-France

#ifndef __KVSI75_E494S_H
#define __KVSI75_E494S_H

#include "KVSilicon.h"

class KVSi75_e494s : public KVSi75 {

public:
   KVSi75_e494s();
   KVSi75_e494s(Float_t thick);
   virtual ~KVSi75_e494s();
   Double_t GetVoltsFromCanalPG(Double_t chan = 0.0);
   Int_t GetCanalPGFromVolts(Float_t volts);
   Double_t GetCanalPGFromVoltsDouble(Float_t volts);
//   void Copy(TObject& obj) const;

   ClassDef(KVSi75_e494s, 1) //Calib Etalons
};

class KVSiLi_e494s : public KVSiLi {

public:
   KVSiLi_e494s();
   KVSiLi_e494s(Float_t thick);
   virtual ~KVSiLi_e494s();
   Double_t GetVoltsFromCanalPG(Double_t chan = 0.0);
   Int_t GetCanalPGFromVolts(Float_t volts);
   Double_t GetCanalPGFromVoltsDouble(Float_t volts);
//   void Copy(TObject& obj) const;

   ClassDef(KVSiLi_e494s, 1) //Calib Etalons
};

#endif
