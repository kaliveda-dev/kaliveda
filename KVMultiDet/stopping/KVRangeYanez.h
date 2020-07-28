//Created by KVClassFactory on Thu Sep 27 14:48:55 2012
//Author: John Frankland,,,

#ifndef __KVRANGEYANEZ_H
#define __KVRANGEYANEZ_H

#include "KVIonRangeTable.h"
#include "KVHashList.h"

class KVRangeYanezMaterial;

/**
\class KVRangeYanez
\brief Interface to Range dE/dx and range library
\ingroup Stopping

This is a modified version of the Range package developed by Ricardo Yanez.
See <a href="http://www.calel.org/range.html">here</a> for details.

Modifications concern:
  - corrected calculation of Northcliffe-Schilling ranges
  - modified stopping extrapolation for gases in Hubert-Bimbot-Gauvin table
*/

class KVRangeYanez : public KVIonRangeTable {
   static KVHashList* fMaterials;// static list of all currently defined materials
   void CheckMaterialsList() const;
   KVIonRangeTableMaterial* GetMaterialWithNameOrType(const Char_t* material) const;
   TString fLocalMaterialsDirectory;
   mutable Bool_t fDoNotSaveMaterials;
   void SaveMaterial(KVIonRangeTableMaterial* mat) const;

public:

   KVRangeYanez();
   KVRangeYanez(const KVRangeYanez&) ;
   ROOT_COPY_ASSIGN_OP(KVRangeYanez)
   virtual ~KVRangeYanez() {}
   void Copy(TObject&) const;

   virtual KVIonRangeTableMaterial* AddElementalMaterial(Int_t z, Int_t a = 0) const;
   virtual KVIonRangeTableMaterial* AddCompoundMaterial(
      const Char_t* name, const Char_t* symbol,
      Int_t nelem,  Int_t* z, Int_t* a, Int_t* natoms, Double_t density = -1.0) const;
   virtual KVIonRangeTableMaterial* AddMixedMaterial(
      const Char_t* name, const Char_t* symbol,
      Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t* weight, Double_t density = -1.0) const;
   KVIonRangeTableMaterial* MakeNaturallyOccuringElementMixture(Int_t z, Int_t& a) const;
   void Print(Option_t* = "") const;
   TObjArray* GetListOfMaterials();
   Bool_t CheckIon(Int_t, Int_t) const
   {
      return kTRUE;
   }
   Bool_t ReadMaterials(const Char_t* filename) const;

   ClassDef(KVRangeYanez, 1) //Interface to Range dE/dx and range library (Ricardo Yanez)
};

#endif
