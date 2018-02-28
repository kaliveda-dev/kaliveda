//Created by KVClassFactory on Thu Feb  3 10:04:41 2011
//Author: frankland,,,,

#include "KVIonRangeTable.h"
#include "KVIonRangeTableMaterial.h"
#include <TPluginManager.h>
#include <TError.h>
#include "TGeoManager.h"

#define FIND_MAT_AND_EXEC(method,defval) \
   KVIonRangeTableMaterial* M = GetMaterial(mat); \
   if(M) return M->method; \
   return defval
#define CHECK_ION_FIND_MAT_AND_EXEC(method,defval) \
   if(!CheckIon(Z,A)){ \
      if(Z) Warning(#method , "Ion Z=%d out of range table limits", Z); \
      return defval; \
   } \
   KVIonRangeTableMaterial* M = GetMaterial(mat); \
   if(M) return M->method; \
   return defval

ClassImp(KVIonRangeTable)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIonRangeTable</h2>
<h4>Abstract base class for calculation of range & energy loss of charged particles in matter</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIonRangeTable::KVIonRangeTable(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Default constructor
}

KVIonRangeTable::~KVIonRangeTable()
{
   // Destructor
}

KVIonRangeTable* KVIonRangeTable::GetRangeTable(const Char_t* name)
{
   // Generates an instance of the KVIonRangeTable plugin class corresponding to given name.

   TPluginHandler* ph;
   //check and load plugin library
   if (!(ph = LoadPlugin("KVIonRangeTable", name))) {
      ::Error("KVIonRangeTable::GetRangeTable", "No plugin for KVIonRangeTable with name=%s found in .kvrootrc", name);
      return 0;
   }
   KVIonRangeTable* irt = (KVIonRangeTable*) ph->ExecPlugin(0);
   return irt;
}

//________________________________________________________________________________//

Double_t KVIonRangeTable::GetAtomicMass(const Char_t* material)
{
   // Return atomic mass of a material in the range table.
   // "material" can be either the type or the name of the material.
   // Prints a warning and returns 0 if material is unknown.

   KVIonRangeTableMaterial* M = GetMaterial(material);
   if (!M) {
      Warning("GetAtomicMass", "Material %s is unknown. Returned mass = 0.", material);
      return 0.0;
   }
   return M->GetMass();
}

KVIonRangeTableMaterial* KVIonRangeTable::GetMaterial(const Char_t* material)
{
   return GetMaterialWithNameOrType(material);
}

KVIonRangeTableMaterial* KVIonRangeTable::GetMaterial(TGeoMaterial* mat)
{
   return GetMaterialWithPointer(mat);
}

KVIonRangeTableMaterial* KVIonRangeTable::GetMaterialWithPointer(TGeoMaterial* material)
{
   // Returns pointer to material for given TGeoMaterial
   // We try both the name and the title of the TGeoMaterial
   KVIonRangeTableMaterial* mat = GetMaterial(material->GetTitle());
   if (!mat) mat = GetMaterial(material->GetName());
   return mat;
}

//________________________________________________________________________________//

Double_t KVIonRangeTable::GetZ(const Char_t* material)
{
   // Return atomic number of a material in the range table.
   // "material" can be either the type or the name of the material.
   // Prints a warning and returns 0 if material is unknown.

   KVIonRangeTableMaterial* M = GetMaterial(material);
   if (!M) {
      Warning("GetZ", "Material %s is unknown. Returned Z = 0.", material);
      return 0.0;
   }
   return M->GetZ();
}

//________________________________________________________________________________//

Bool_t KVIonRangeTable::IsMaterialKnown(const Char_t* material)
{
   // Returns kTRUE if material of given name or type is in range table.
   KVIonRangeTableMaterial* M = GetMaterial(material);
   return (M != 0x0);
}

Bool_t KVIonRangeTable::IsMaterialKnown(TGeoMaterial* material)
{
   // Returns kTRUE if material corresponding to TGeoMaterial name or type is in range table.
   KVIonRangeTableMaterial* M = GetMaterial(material);
   return (M != 0x0);
}

//________________________________________________________________________________//

Bool_t KVIonRangeTable::IsMaterialGas(const Char_t* mat)
{
   // Returns kTRUE if material of given name or type is gaseous.
   FIND_MAT_AND_EXEC(IsGas(), kFALSE);
}

//________________________________________________________________________________//

const Char_t* KVIonRangeTable::GetMaterialName(const Char_t* mat)
{
   // Return name of material of given type or name if it is in range tables
   FIND_MAT_AND_EXEC(GetName(), "");
}

//________________________________________________________________________________//

Double_t KVIonRangeTable::GetDensity(const Char_t* mat)
{
   // Return density of material (g/cm**3) of given type or name if it is in range tables
   FIND_MAT_AND_EXEC(GetDensity(), 0.0);
}

//________________________________________________________________________________//

void KVIonRangeTable::SetTemperatureAndPressure(const Char_t* material, Double_t temperature, Double_t pressure)
{
   // Set temperature (in degrees celsius) and pressure (in torr) for a given
   // material. This has no effect except for gaseous materials, for which T & P
   // determine the density (in g/cm**3).

   KVIonRangeTableMaterial* M = GetMaterial(material);
   if (M) M->SetTemperatureAndPressure(temperature, pressure);
}

//________________________________________________________________________________//

Double_t KVIonRangeTable::GetRangeOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t E,
                                        Double_t Amat, Double_t, Double_t)
{
   // Returns range (in g/cm**2) of ion (Z,A) with energy E (MeV) in material.
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetRangeOfIon(Z, A, E, Amat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearRangeOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t E,
      Double_t Amat, Double_t T, Double_t P)
{
   // Returns linear range (in cm) of ion (Z,A) with energy E (MeV) in material.
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearRangeOfIon(Z, A, E, Amat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t E, Double_t r,
      Double_t Amat, Double_t, Double_t)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness r (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetDeltaEOfIon(Z, A, E, r, Amat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t E, Double_t d,
      Double_t Amat, Double_t T, Double_t P)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness d (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearDeltaEOfIon(Z, A, E, d, Amat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetEResOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t E, Double_t r,
                                       Double_t Amat, Double_t, Double_t)
{
   // Returns residual energy (in MeV) of ion (Z,A) with incident energy E (MeV) after thickness r (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetEResOfIon(Z, A, E, r, Amat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearEResOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t E, Double_t d,
      Double_t Amat, Double_t T, Double_t P)
{
   // Returns residual energy (in MeV) of ion (Z,A) with incident energy E (MeV) after thickness d (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearEResOfIon(Z, A, E, d, Amat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetEIncFromEResOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat, Double_t, Double_t)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) with residual energy Eres (MeV) after thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   CHECK_ION_FIND_MAT_AND_EXEC(GetEIncFromEResOfIon(Z, A, Eres, e, isoAmat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearEIncFromEResOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t Eres, Double_t e,
      Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) with residual energy Eres (MeV) after thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearEIncFromEResOfIon(Z, A, Eres, e, isoAmat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetEIncFromDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t DeltaE, Double_t e, enum KVIonRangeTable::SolType type, Double_t isoAmat, Double_t, Double_t)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) from energy loss DeltaE (MeV) in thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   CHECK_ION_FIND_MAT_AND_EXEC(GetEIncFromDeltaEOfIon(Z, A, DeltaE, e, type, isoAmat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearEIncFromDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t deltaE, Double_t e, enum KVIonRangeTable::SolType type,
      Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) from energy loss DeltaE (MeV) in thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearEIncFromDeltaEOfIon(Z, A, deltaE, e, type, isoAmat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetDeltaEFromEResOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat, Double_t, Double_t)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) from energy loss DeltaE (MeV) in thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   CHECK_ION_FIND_MAT_AND_EXEC(GetDeltaEFromEResOfIon(Z, A, Eres, e, isoAmat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearDeltaEFromEResOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) from energy loss DeltaE (MeV) in thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearDeltaEFromEResOfIon(Z, A, Eres, e, isoAmat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetPunchThroughEnergy(const Char_t* mat, Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t, Double_t)
{
   // Calculate incident energy (in MeV) for ion (Z,A) for which the range is equal to the
   // given thickness e (in g/cm**2). At this energy the residual energy of the ion is (just) zero,
   // for all energies above this energy the residual energy is > 0.
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   CHECK_ION_FIND_MAT_AND_EXEC(GetPunchThroughEnergy(Z, A, e, isoAmat), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetLinearPunchThroughEnergy(const Char_t* mat, Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculate incident energy (in MeV) for ion (Z,A) for which the range is equal to the
   // given thickness e (in cm). At this energy the residual energy of the ion is (just) zero,
   // for all energies above this energy the residual energy is > 0.
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearPunchThroughEnergy(Z, A, e, isoAmat, T, P), 0.0);
}
//________________________________________________________________________________//

Double_t KVIonRangeTable::GetMaxDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t, Double_t)
{
   // Calculate maximum energy loss (in MeV) of ion (Z,A) in given thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material.
   CHECK_ION_FIND_MAT_AND_EXEC(GetMaxDeltaEOfIon(Z, A, e, isoAmat), 0.0);
}

Double_t KVIonRangeTable::GetEIncOfMaxDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t, Double_t)
{
   // Calculate incident energy (in MeV) corresponding to maximum energy loss of ion (Z,A)
   // in given thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material.

   CHECK_ION_FIND_MAT_AND_EXEC(GetEIncOfMaxDeltaEOfIon(Z, A, e, isoAmat), 0.0);
}

Double_t KVIonRangeTable::GetLinearMaxDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculate maximum energy loss (in MeV) of ion (Z,A) in given thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearMaxDeltaEOfIon(Z, A, e, isoAmat, T, P), 0.0);
}

Double_t KVIonRangeTable::GetLinearEIncOfMaxDeltaEOfIon(const Char_t* mat, Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculate incident energy (in MeV) corresponding to maximum energy loss of ion (Z,A)
   // in given thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   CHECK_ION_FIND_MAT_AND_EXEC(GetLinearEIncOfMaxDeltaEOfIon(Z, A, e, isoAmat, T, P), 0.0);
}

Double_t KVIonRangeTable::GetEmaxValid(const Char_t* mat, Int_t Z, Int_t A)
{
   // Returns maximum energy (in MeV) for which range table is valid
   // for given material and incident ion (Z,A)

   CHECK_ION_FIND_MAT_AND_EXEC(GetEmaxValid(Z, A), 0.0);
}

//______________________________________________________________________________________//

TGeoMaterial* KVIonRangeTable::GetTGeoMaterial(const Char_t* mat)
{
   // Return pointer to TGeoMaterial corresponding to this material,
   // for use in ROOT geometries, VMC, etc.

   FIND_MAT_AND_EXEC(GetTGeoMaterial(), 0x0);
}

void KVIonRangeTable::Print(Option_t*) const
{
   printf("%s::%s\n%s\n", ClassName(), GetName(), GetTitle());
}

