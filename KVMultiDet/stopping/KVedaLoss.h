//Created by KVClassFactory on Wed Feb  2 15:49:27 2011
//Author: frankland,,,,

#ifndef __KVEDALOSS_H
#define __KVEDALOSS_H

#include "KVIonRangeTable.h"
#include "KVHashList.h"

class KVedaLossMaterial;
class TGeoMaterial;

/**
  \class KVedaLoss
  \ingroup Stopping
  \brief C++ implementation of VEDALOSS stopping power calculation

See documentation <a href="KVedaLossDoc/KVedaLoss.html">here</a>.

### Energy limits
Normally all range, \f$dE\f$, \f$E_{res}\f$ functions are limited to range \f$0\leq E\leq E_{max}\f$,
where \f$E_{max}\f$ is nominal maximum energy for which range tables are valid
(usually 400MeV/u for \f$Z<3\f$, 250MeV/u for \f$Z>3\f$).

If higher energies are required, call static method KVedaLoss::SetIgnoreEnergyLimits() **BEFORE ANY MATERIALS ARE CREATED**
in order to recalculate the \f$E_{max}\f$ limits in such a way that:
   -  range function is always monotonically increasing function of \f$E_{inc}\f$;
   -  stopping power is concave (i.e. no minimum of stopping power followed by an increase)

Then, at the most, the new limit will be 1 GeV/nucleon, or
at the least, it will remain at the nominal (400 or 250 MeV/nucleon) level.
 */
class KVedaLoss : public KVIonRangeTable {
   static KVHashList* fMaterials;// static list of all known materials
   TString fLocalMaterialsDirectory;

   Bool_t init_materials() const;
   Bool_t CheckMaterialsList() const
   {
      if (!fMaterials) return init_materials();
      return kTRUE;
   };
   KVIonRangeTableMaterial* GetMaterialWithNameOrType(const Char_t* material) const;
   static Bool_t fgNewRangeInversion;// static flag for using new KVedaLossInverseRangeFunction

   void AddMaterial(KVIonRangeTableMaterial*) const;

public:
   KVedaLoss();
   virtual ~KVedaLoss();


   void Print(Option_t* = "") const;
   TObjArray* GetListOfMaterials();

   static void SetIgnoreEnergyLimits(Bool_t yes = kTRUE);

   static void SetUseNewRangeInversion(Bool_t yes = kTRUE)
   {
      fgNewRangeInversion = yes;
   }
   static Bool_t IsUseNewRangeInversion()
   {
      return fgNewRangeInversion;
   }

   Bool_t CheckIon(Int_t Z, Int_t A) const;
   Bool_t ReadMaterials(const Char_t* path) const;

   KVIonRangeTableMaterial* AddElementalMaterial(Int_t Z, Int_t A = 0) const;
   Bool_t AddRANGEMaterial(const Char_t* name) const;
   KVIonRangeTableMaterial* AddCompoundMaterial(
      const Char_t* /*name*/, const Char_t* /* symbol */,
      Int_t /* nelem */, Int_t* /* z */, Int_t* /* a */, Int_t* /* natoms */, Double_t /* density */ = -1.0) const;
   KVIonRangeTableMaterial* AddMixedMaterial(
      const Char_t* /* name */, const Char_t* /* symbol */,
      Int_t /* nelem */, Int_t* /* z */, Int_t* /* a */, Int_t* /* natoms */, Double_t* /* proportion */,
      Double_t /* density */ = -1.0) const;
   ClassDef(KVedaLoss, 1) //C++ implementation of VEDALOSS stopping power calculation
};

#endif
