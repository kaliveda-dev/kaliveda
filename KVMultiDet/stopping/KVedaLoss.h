//Created by KVClassFactory on Wed Feb  2 15:49:27 2011
//Author: frankland,,,,

#ifndef __KVEDALOSS_H
#define __KVEDALOSS_H

#include "KVIonRangeTable.h"
#include "KVHashList.h"

class KVedaLossMaterial;
class TGeoMaterial;

class KVedaLoss : public KVIonRangeTable {
   static KVHashList* fMaterials;// static list of all known materials

   Bool_t init_materials() const;
   Bool_t CheckMaterialsList() const
   {
      if (!fMaterials) return init_materials();
      return kTRUE;
   };
   KVIonRangeTableMaterial* GetMaterialWithNameOrType(const Char_t* material);

public:
   KVedaLoss();
   virtual ~KVedaLoss();


   void Print(Option_t* = "") const;
   TObjArray* GetListOfMaterials();

   static void SetIgnoreEnergyLimits(Bool_t yes = kTRUE);

   ClassDef(KVedaLoss, 1) //C++ implementation of VEDALOSS stopping power calculation
};

#endif
