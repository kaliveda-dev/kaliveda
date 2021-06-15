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

##Description
Based on the original Fortran code VEDALOSS written by Roland Dayras (CEA/SPhN Saclay) and
Enrico de Filippo (INFN LNS Catania), KVedaLoss provides range tables for the stopping of charged ions
in a large variety of pre-defined solid and gaseous media. It can be used to calculated energy losses,
residual energies, ranges, and the corresponding inverse calculations.

The code interpolates the range tables of Northcliffe and Schilling (NCS) with those of Hubert, Bimbot and Gauvin (HBG)
in order to calculate ranges based on data valid in the energy range \f$0.1\leq E/A\leq 250\f$ MeV/nucleon.
For \f$E/A<2.5\f$ MeV/nucleon ranges are taken from NCS (which include the contribution from nuclear stopping),
while at higher energies ranges are taken from HBG. It should be noted that the ranges of HBG were calculated
from 2.5 MeV/nucleon upwards taking the NCS range at 2.5 MeV/nucleon as starting point.

## Method
For each material, the interpolated range for each ion from \f$Z=1\f$ to \f$Z=100\f$ is parametrised as
\f[
\log R = \sum_{i=0}^{5}a_{i}\left(\log\epsilon\right)^{i}
\f]
where \f$R\f$ is the range [\f$g/cm^{2}\f$] of the ion and \f$\epsilon\f$ its incident energy [MeV/nucleon].
The six parameters \f$a_{i}\f$ for each atomic number \f$Z\f$ (for a reference isotope mass  number \f$A\f$)
were found by fitting the range data taken from NS and HBG. Such fits can be realized using the KVedaLossRangeFitter
class.

See <a href="http://indra.in2p3.fr/kaliveda/KVedaLossDoc/KVedaLoss.html">here</a> for comparisons of the fitted
ranges with the original values from NCS and HBG.

## Adding new absorber definitions
KVedaLoss provides methods to add new absorber definitions using interpolations of the NCS and HBG range tables
provided by KVRangeYanez. See below for methods to add new absorbers composed of single elements, compounds, or mixtures.

For full explanations and examples of how to use this facility, see \ref Stopping

## Energy limits
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
