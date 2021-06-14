/***************************************************************************
                          kvmaterial.cpp  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Riostream.h"
#include "KVMaterial.h"
#include "KVNucleus.h"
#include "TMath.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TEnv.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "KVIonRangeTable.h"

#include <KVValueRange.h>
#include <TGraph.h>

using namespace std;

ClassImp(KVMaterial);

KVIonRangeTable* KVMaterial::fIonRangeTable = 0x0;

//___________________________________________________________________________________
void KVMaterial::init()
{
   // Default initialisations.
   //
   // No properties are set for the material (except standard temperature (19°C) and pressure (1 atm)).
   //
   // Default range table is generated if not already done. By default it is the VEDALOSS table implemented in KVedaLoss.
   // You can change this by changing the value of environment variable `KVMaterial.IonRangeTable` or by calling
   // static method ChangeRangeTable() before creating any materials.

   fELoss = 0;
   SetName("");
   SetTitle("");
   fAmasr = 0;
   fPressure = 1. * KVUnits::atm;
   fTemp = 19.0;
   // create default range table singleton if not already done
   GetRangeTable();
   fAbsorberVolume = nullptr;
}

//
KVMaterial::KVMaterial()
{
   //default ctor
   init();
}

//__________________________________________________________________________________
KVMaterial::KVMaterial(const Char_t* type, const Double_t thick)
{
   // Create material with given type and linear thickness in cm.

   init();
   SetMaterial(type);
   SetThickness(thick);
}

KVMaterial::KVMaterial(Double_t area_density, const Char_t* type)
{
   // Create material with given area density in \f$g/cm^{2}\f$ and given type

   init();
   SetMaterial(type);
   SetAreaDensity(area_density);
}

KVMaterial::KVMaterial(const Char_t* gas, const Double_t thick, const Double_t pressure, const Double_t temperature)
{
   // Create gaseous material with given type, linear thickness in cm, pressure in Torr,
   // and temperature in degrees C (default value 19°C).
   //
   // __Examples__
   //~~~~{.cpp}
   //KVMaterial("CF4", 15*KVUnits::cm, 1*KVUnits::atm);   // 15cm of CF4 gas at 1atm and 19°C
   //
   //KVMaterial("C3F8", 50*KVUnits::mm, 30*KVUnits::mbar, 25);  //  50mm of C3F8 at 30mbar and 25°C
   //~~~~

   init();
   SetMaterial(gas);
   fPressure = pressure;
   fTemp = temperature;
   SetThickness(thick);
}

//
KVMaterial::KVMaterial(const KVMaterial& obj) : KVBase()
{
   //Copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVMaterial&) obj).Copy(*this);
#endif
}

KVIonRangeTable* KVMaterial::GetRangeTable()
{
   // Static method
   // \returns pointer to currently used range table
   if (!fIonRangeTable) {
      fIonRangeTable = KVIonRangeTable::GetRangeTable(gEnv->GetValue("KVMaterial.IonRangeTable", "VEDALOSS"));
   }
   return fIonRangeTable;
}

KVIonRangeTable* KVMaterial::ChangeRangeTable(const Char_t* name)
{
   // Changes the default range table used for energy loss calculations.
   //
   // The name must correspond to a Plugin defined for class KVIonRangeTable - see list given by
   //
   //~~~~{.cpp}
   //KVBase::GetListOfPlugins("KVIonRangeTable")
   //~~~~

   if (fIonRangeTable) delete fIonRangeTable;
   fIonRangeTable = KVIonRangeTable::GetRangeTable(name);
   if (!fIonRangeTable)
      ::Error("KVMaterial::ChangeRangeTable", "No plugin %s defined for KVIonRangeTable", name);
   return fIonRangeTable;
}

//___________________________________________________________________________________
void KVMaterial::SetMaterial(const Char_t* mat_type)
{
   // Intialise material of a given type, which must exist in the currently used range table.
   //
   //   The list of available material types depends on the underlying range table: this list can be obtained or visualised like so:
   //   ~~~~{.cpp}
   //   KVMaterial::GetRangeTable()->Print(); // print infos on range table

   //   KVMaterial::GetRangeTable()->GetListOfMaterials()->ls(); // retrieve pointer to list

   //   OBJ: TObjArray  TObjArray   An array of objects : 0
   //    OBJ: TNamed Silicon  Si : 0 at: 0x55a63a979390
   //    OBJ: TNamed Mylar Myl : 0 at: 0x55a63d6a95f0
   //    OBJ: TNamed Plastic  NE102 : 0 at: 0x55a63d64ea90
   //    OBJ: TNamed Nickel   Ni : 0 at: 0x55a63d6afb90
   //    OBJ: TNamed Octofluoropropane C3F8 : 0 at: 0x55a63a9f8e00
   //   etc. etc.
   //   ~~~~
   //
   // For materials which are elements of the periodic table you can specify
   // the isotope such as \c "64Ni", \c "13C", \c "natSn", etc. etc.

   init();
   //are we dealing with an isotope ?
   Char_t type[10];
   Int_t iso_mass = 0;
   if (sscanf(mat_type, "nat%s", type) != 1) {
      if (sscanf(mat_type, "%d%s", &iso_mass, type) != 2) {
         strcpy(type, mat_type);
      }
   }
   if (iso_mass) SetMass(iso_mass);
   SetType(type);
   if (!fIonRangeTable->IsMaterialKnown(type))
      Warning("SetMaterial",
              "Called for material %s which is unknown in current range table %s. Energy loss & range calculations impossible.",
              type, fIonRangeTable->GetName());
   else {
      SetType(fIonRangeTable->GetMaterialName(type));
      SetName(type);
   }
}

//___________________________________________________________________________________
KVMaterial::~KVMaterial()
{
   //Destructor
}

void KVMaterial::SetMass(Int_t a)
{
   //Define a specific isotopic mass for the material, e.g. for isotopically pure targets.
   //
   //For detectors, this changes the mass of the material composing the active layer (see KVDetector).

   if (GetActiveLayer()) {
      GetActiveLayer()->SetMass(a);
      return;
   }
   fAmasr = a;
}

//___________________________________________________________________________________
Double_t KVMaterial::GetMass() const
{
   //Returns atomic mass of material.
   //
   //For detectors, this is the mass of the material composing the active layer (see KVDetector).

   if (GetActiveLayer())
      return GetActiveLayer()->GetMass();
   return (fAmasr ? fAmasr : fIonRangeTable->GetAtomicMass(GetType()));
}

//___________________________________________________________________________________
Bool_t KVMaterial::IsIsotopic() const
{
   //Returns kTRUE if a specific isotope has been chosen for the material
   //using SetMass(), e.g.
   //   - for \f${}^{119}Sn\f$ this method returns kTRUE
   //   - for \f${}^{nat}Sn\f$ this method returns kFALSE
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).
   //\sa IsNat()
   if (GetActiveLayer())
      return GetActiveLayer()->IsIsotopic();
   return (fAmasr != 0);
}

//___________________________________________________________________________________
Bool_t KVMaterial::IsNat() const
{
   //Returns kFALSE if a specific isotope has been chosen for the material
   //using SetMass() e.g.
   //   - for \f${}^{119}Sn\f$ this method returns kFALSE
   //   - for \f${}^{nat}Sn\f$ this method returns kTRUE
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).
   //\sa IsIsotopic()
   if (GetActiveLayer())
      return GetActiveLayer()->IsNat();
   return (!IsIsotopic());
}

//___________________________________________________________________________________

Bool_t KVMaterial::IsGas() const
{
   // Returns kTRUE for gaseous material.
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer())
      return GetActiveLayer()->IsGas();
   return fIonRangeTable->IsMaterialGas(GetType());
}

//___________________________________________________________________________________

Double_t KVMaterial::GetZ() const
{
   //Returns atomic number of material.
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).
   if (GetActiveLayer())
      return GetActiveLayer()->GetZ();
   return fIonRangeTable->GetZ(GetType());
}

//___________________________________________________________________________________

Double_t KVMaterial::GetDensity() const
{
   // Returns density of material in \f$g/cm^{3}\f$.
   //
   //~~~~{.cpp}
   //auto dens = mat.GetDensity()/(KVUnits::kg/KVUnits::litre); // in kg/litre
   //~~~~
   //
   // For a gas, density is calculated from current pressure & temperature according to ideal gas law
   // \f[
   // \rho = \frac{pM}{RT}
   // \f]
   // with \f$M\f$ the mass of one mole of the gas, and \f$R\f$ the ideal gas constant.
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer())
      return GetActiveLayer()->GetDensity();
   fIonRangeTable->SetTemperatureAndPressure(GetType(), fTemp, fPressure);
   return fIonRangeTable->GetDensity(GetType());
}

//___________________________________________________________________________________

void KVMaterial::SetThickness(Double_t t)
{
   // Set the linear thickness of the material in cm, e.g.
   //~~~~{.cpp}
   //SetThickness( 30.*KVUnits::um );  set thickness to 30 microns
   //~~~~
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer()) {
      GetActiveLayer()->SetThickness(t);
      return;
   }
   // recalculate area density
   if (GetDensity() != 0)
      fThick = t * GetDensity();
   else
      fThick = t;

}

//___________________________________________________________________________________

Double_t KVMaterial::GetThickness() const
{
   // Returns the linear thickness of the material in cm.
   // Use KVUnits to translate from one unit to another, e.g.
   //~~~~{.cpp}
   //auto micro_thick = mat.GetThickness()/KVUnits::um; thickness in microns
   //~~~~
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer())
      return GetActiveLayer()->GetThickness();
   if (GetDensity() != 0)
      return fThick / GetDensity();
   else
      return fThick;
}

//___________________________________________________________________________________

void KVMaterial::SetAreaDensity(Double_t dens /* g/cm**2 */)
{
   // Set area density in \f$g/cm^{2}\f$.
   //
   // For solids, area density can only be changed by changing the thickness of the material.
   //
   // For gases, the density depends on temperature and pressure - see GetDensity(). This method
   // leaves temperature and pressure unchanged, therefore for gases also this
   // method will effectively modify the linear dimension of the gas cell.
   //
   //~~~~{.cpp}
   //mat.SetAreaDensity(500*KVUnits::ug); // set density in microgram/cm2
   //~~~~
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer())
      GetActiveLayer()->SetAreaDensity(dens);
   fThick = dens;
}

//___________________________________________________________________________________

Double_t KVMaterial::GetAreaDensity() const
{
   // Return area density of material in \f$g/cm^{2}\f$
   //
   //~~~~{.cpp}
   //auto dens_mgcm2 = mat.GetAreaDensity()/KVUnits::mg; // in mg/cm2
   //~~~~
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer()) return GetActiveLayer()->GetAreaDensity();
   return fThick;
}

//___________________________________________________________________________________

void KVMaterial::SetPressure(Double_t p)
{
   // Set the pressure of a gaseous material (in torr).
   // The linear dimension (thickness) is kept constant, the area density changes.
   //
   //~~~~{.cpp}
   //mat.SetPressure(50*KVUnits::mbar); // set pressure to 50mbar
   //~~~~
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (!IsGas()) return;
   if (GetActiveLayer()) {
      GetActiveLayer()->SetPressure(p);
      return;
   }
   // get current linear dimension of absorber
   Double_t e = GetThickness();
   fPressure = p;
   // change area density to keep linear dimension constant
   SetThickness(e);
}


//___________________________________________________________________________________

Double_t KVMaterial::GetPressure() const
{
   // Returns the pressure of a gas (in torr).
   // If the material is not a gas - see IsGas() - value is zero.
   //
   //~~~~{.cpp}
   //auto press_mbar = mat.GetPressure()/KVUnits::mbar; // pressure in mbar
   //~~~~
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (!IsGas()) return 0.0;
   if (GetActiveLayer())
      return GetActiveLayer()->GetPressure();
   return fPressure;
}

//___________________________________________________________________________________

void KVMaterial::SetTemperature(Double_t t)
{
   // Set temperature of material in degrees celsius.
   //
   // This only has an effect on gaseous materials, where the resulting change in density
   // changes the area density of the absorber (for fixed linear dimension).
   // \sa GetDensity()
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (!IsGas()) return;
   if (GetActiveLayer()) {
      GetActiveLayer()->SetTemperature(t);
      return;
   }
   // get current linear dimension of absorber
   Double_t e = GetThickness();
   fTemp = t;
   // change area density to keep linear dimension constant
   SetThickness(e);
}


//___________________________________________________________________________________

Double_t KVMaterial::GetTemperature() const
{
   //Returns temperature of material in degrees celsius (only gaseous materials).
   //
   //For detectors, the material in question is that of the active layer (see KVDetector).

   if (GetActiveLayer())
      return GetActiveLayer()->GetTemperature();
   return fTemp;
}

//___________________________________________________________________________________

Double_t KVMaterial::GetEffectiveThickness(TVector3& norm, TVector3& direction)
{
   //\param[in] norm vector normal to the material, oriented from the origin towards the material
   //\param[in] direction direction of motion of an ion
   //\returns effective linear thickness of absorber (in cm) as 'seen' in given direction, taking into
   // account the arbitrary orientation of the normal to the material's surface

   TVector3 n = norm.Unit();
   TVector3 d = direction.Unit();
   //absolute value of scalar product, in case direction is opposite to normal
   Double_t prod = TMath::Abs(n * d);
   return GetThickness() / TMath::Max(prod, 1.e-03);
}

//___________________________________________________________________________________

Double_t KVMaterial::GetEffectiveAreaDensity(TVector3& norm, TVector3& direction)
{
   //\param[in] norm vector normal to the material, oriented from the origin towards the material
   //\param[in] direction direction of motion of an ion
   //\returns effective area density of absorber in \f$g/cm^{2}\f$ as 'seen' in the given direction, taking into
   // account the arbitrary orientation of the normal to the material's surface

   TVector3 n = norm.Unit();
   TVector3 d = direction.Unit();
   //absolute value of scalar product, in case direction is opposite to normal
   Double_t prod = TMath::Abs(n * d);
   return GetAreaDensity() / TMath::Max(prod, 1.e-03);
}

//___________________________________________________________________________________
void KVMaterial::Print(Option_t*) const
{
   //Show information on this material
   cout << "KVMaterial: " << GetName() << " (" << GetType() << ")" << endl;
   if (fIonRangeTable->IsMaterialGas(GetType()))
      cout << " Pressure " << GetPressure() << " torr" << endl;
   cout << " Thickness " << KVMaterial::GetThickness() << " cm" << endl;
   cout << " Area density " << KVMaterial::GetAreaDensity() << " g/cm**2" << endl;
   cout << "-----------------------------------------------" << endl;
   cout << " Z = " << GetZ() << " atomic mass = " << GetMass() << endl;
   cout << " Density = " << GetDensity() << " g/cm**3" << endl;
   cout << "-----------------------------------------------" << endl;
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetELostByParticle(KVNucleus* kvn, TVector3* norm)
{
   //\param[in] kvp pointer to a KVNucleus object describing a charged ion
   //\param[in] norm [optional] vector normal to the material, oriented from the origin towards the material
   //
   //\returns The energy loss \f$\Delta E\f$ in MeV of a charged particle impinging on the absorber
   //
   //If the unit normal vector is given, the effective thickness of the material 'seen' by the particle
   //depending on the orientation of its direction of motion with respect to the absorber is used for the calculation,
   //rather than the nominal thickness corresponding to ions impinging perpendicularly.

   Double_t thickness;
   if (norm) {
      TVector3 p = kvn->GetMomentum();
      thickness = GetEffectiveThickness((*norm), p);
   }
   else
      thickness = GetThickness();
   Double_t E_loss =
      fIonRangeTable->GetLinearDeltaEOfIon(GetType(), kvn->GetZ(), kvn->GetA(), kvn->GetKE(),
                                           thickness, fAmasr, fTemp, fPressure);
   return E_loss;
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetParticleEIncFromERes(KVNucleus* kvn, TVector3* norm)
{
   // \param[in] kvn KVNucleus describing properties of incident ion (\f$Z,A\f$ and with kinetic energy \f$E_{res}\f$)
   // \param[in] norm [optional] vector normal to the material, oriented from the origin towards the material.
   // \returns incident energy of ion \f$E_{inc}\f$ [MeV] deduced from residual energy \f$E_{res}=E_{inc}-\Delta E\f$.
   //
   //If \a norm is given, the effective thickness of the material 'seen' by the particle
   //depending on its direction of motion is used for the calculation.

   Double_t thickness;
   if (norm) {
      TVector3 p = kvn->GetMomentum();
      thickness = GetEffectiveThickness((*norm), p);
   }
   else
      thickness = GetThickness();
   Double_t E_inc = fIonRangeTable->
                    GetLinearEIncFromEResOfIon(GetType(), kvn->GetZ(), kvn->GetA(), kvn->GetKE(),
                          thickness, fAmasr, fTemp, fPressure);
   return E_inc;
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetDeltaE(Int_t Z, Int_t A, Double_t Einc)
{
   // \param[in] Z atomic number of incident ion
   // \param[in] A mass number of incident ion
   // \param[in] Einc kinetic energy of incident ion
   // \returns Energy lost \f$\Delta E\f$ [MeV] by an ion \f$Z, A\f$ impinging on the absorber with kinetic energy \f$E_{inc}\f$ [MeV]

   if (Z < 1) return 0.;
   Double_t E_loss =
      fIonRangeTable->GetLinearDeltaEOfIon(GetType(), Z, A, Einc, GetThickness(), fAmasr, fTemp, fPressure);

   return TMath::Max(E_loss, 0.);
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetRange(Int_t Z, Int_t A, Double_t Einc)
{
   // \param[in] Z atomic number of incident ion
   // \param[in] A mass number of incident ion
   // \param[in] Einc kinetic energy of incident ion
   // \returns range in [\f$g/cm^2\f$] in absorber for incident nucleus \f$Z,A\f$ with kinetic energy \f$E_{inc}\f$ [MeV]
   //
   // Different units can be used with KVUnits:
   //~~~~{.cpp}
   // KVMaterial si("Si");
   // si.GetRange(2,4,13)/KVUnits::mg;
   //(long double) 25.190011L     // range in silicon in mg/cm2 of 13 MeV 4He particles
   //~~~~

   if (Z < 1) return 0.;
   Double_t R =
      fIonRangeTable->GetRangeOfIon(GetType(), Z, A, Einc, fAmasr, fTemp, fPressure);
   return R;
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetLinearRange(Int_t Z, Int_t A, Double_t Einc)
{
   // \param[in] Z atomic number of incident ion
   // \param[in] A mass number of incident ion
   // \param[in] Einc kinetic energy of incident ion
   // \returns linear range in [cm] in absorber for incident nucleus \f$Z,A\f$
   // with kinetic energy \f$E_{inc}\f$ [MeV]
   //
   // Different units can be used with KVUnits:
   //~~~~{.cpp}
   // KVMaterial si("Si");
   // si.GetLinearRange(2,4,13)/KVUnits::um;
   //(long double) 108.11164L   // range in silicon in microns of 13 MeV 4He particles
   //~~~~

   if (Z < 1) return 0.;
   Double_t R =
      fIonRangeTable->GetLinearRangeOfIon(GetType(), Z, A, Einc, fAmasr, fTemp, fPressure);
   return R;
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetDeltaEFromERes(Int_t Z, Int_t A, Double_t Eres)
{
   // \param[in] Z atomic number of incident ion
   // \param[in] A mass number of incident ion
   // \param[in] Eres residual kinetic energy of ion after impinging on the absorber
   // \returns Incident kinetic energy \f$E_{inc}\f$ of an ion \f$Z, A\f$ with given \f$E_{res}=E_{inc}-\Delta E\f$ residual energy
   //
   //\b Example of use:
   //~~~~{.cpp}
   //KVMaterial si("Si", 300*KVUnits::um);  // 300um silicon absorber
   //KVNucleus alpha("4He",30);             // 30MeV/u alpha particle
   //
   //si.DetectParticle(&alpha);             // particle is slowed by silicon
   //
   //si.GetEnergyLoss();
   //(double) 4.1371801                     // energy lost by particle in silicon
   //
   //alpha.GetEnergy();
   //(double) 115.86282                     // residual energy of alpha after silicon
   //
   //si.GetDeltaEFromERes(2,4,115.86282);
   //(double) 4.1371801                     // energy loss calculated from residual energy
   //~~~~

   if (Z < 1) return 0.;
   Double_t E_loss = fIonRangeTable->
                     GetLinearDeltaEFromEResOfIon(
                        GetType(), Z, A, Eres, GetThickness(), fAmasr, fTemp, fPressure);
   return E_loss;
}

Double_t KVMaterial::GetEResFromDeltaE(Int_t Z, Int_t A, Double_t dE, enum SolType type)
{
   //   \returns Calculated residual kinetic energy \f$E_{res}\f$ [MeV] of a nucleus \f$Z,A\f$ after the absorber from
   //    the energy loss \f$\Delta E\f$ in the absorber. If \a dE is given, it is used instead of the current energy loss.
   //
   //   \param[in] Z,A atomic and mass number of nucleus
   //   \param[in] dE [optional] energy loss of nucleus in absorber
   //   \param[in] type
   //   \parblock
   //   Determine the type of solution to use. Possible values are:
   //    \arg SolType::kEmax [default]: solution corresponding to the highest incident energy is returned.
   //   This is the solution found for \f$E_{inc}\f$ above that of the maximum of the \f$\Delta E(E_{inc})\f$ curve.
   //    \arg SolType::kEmin : low energy solution (\f$E_{inc}\f$ below maximum of the \f$\Delta E(E_{inc})\f$ curve).
   //   \endparblock
   //
   //\b Example of use:
   //~~~~{.cpp}
   //KVMaterial si("Si", 300*KVUnits::um);  // 300um silicon absorber
   //KVNucleus alpha("4He",30);             // 30MeV/u alpha particle
   //
   //si.DetectParticle(&alpha);             // particle is slowed by silicon
   //
   //si.GetEnergyLoss();
   //(double) 4.1371801                     // energy lost by particle in silicon
   //
   //alpha.GetEnergy();
   //(double) 115.86282                     // residual energy of alpha after silicon
   //
   //si.GetEResFromDeltaE(2,4);
   //(double) 115.86282                     // residual energy calculated from energy loss
   //~~~~
   //
   //   \note If the energy loss in the absorber is greater than the maximum theoretical \f$\Delta E\f$ - given by GetMaxDeltaE() - then we return
   //   the residual energy corresponding to the maximum - given by GetEIncOfMaxDeltaE().
   //
   //   \note For detectors (see KVDetector), \a dE is the energy loss \f$\Delta E\f$ only in the \b active layer, not the total
   //   energy lost by the particle crossing the detector; because for detectors with inactive layers \f$E_{inc}\geq \Delta E + E_{res}\f$.

   Double_t EINC = GetIncidentEnergy(Z, A, dE, type);
   return GetERes(Z, A, EINC);
}

//__________________________________________________________________________________________

Double_t KVMaterial::GetIncidentEnergy(Int_t Z, Int_t A, Double_t delta_e, enum SolType type)
{
   //\returns Calculated incident energy [MeV] of nucleus \f$Z,A\f$ corresponding to energy loss \f$\Delta E\f$
   //in this absorber. If \a delta_e is given, it is used instead of the current energy loss in this absorber.
   //
   //\param[in] Z,A atomic and mass number of nucleus
   //\param[in] delta_e [optional] energy loss of nucleus in absorber \f$\Delta E\f$ [MeV]
   //\param[in] type
   //\parblock
   //Determine the type of solution to use. Possible values are:
   // \arg SolType::kEmax [default]: solution corresponding to the highest incident energy is returned.
   //This is the solution found for \f$E_{inc}\f$ above that of the maximum of the \f$\Delta E(E_{inc})\f$ curve.
   // \arg SolType::kEmin : low energy solution (\f$E_{inc}\f$ below maximum of the \f$\Delta E(E_{inc})\f$ curve).
   //\endparblock
   //
   //\b Example of use:
   //~~~~{.cpp}
   //KVMaterial si("Si", 300*KVUnits::um);  // 300um silicon absorber
   //KVNucleus alpha("4He",30);             // 30MeV/u alpha particle
   //
   //si.DetectParticle(&alpha);             // particle is slowed by silicon
   //
   //si.GetEnergyLoss();
   //(double) 4.1371801                     // energy lost by particle in silicon
   //
   //si.GetIncidentEnergy(2,4);
   //(double) 120.00000                     // incident energy of alpha calculated from energy loss
   //~~~~
   //
   //\note If the energy loss in the absorber is greater than the maximum theoretical \f$\Delta E\f$ - given by GetMaxDeltaE() - then we return
   //the incident energy corresponding to the maximum - given by GetEIncOfMaxDeltaE().

   if (Z < 1) return 0.;

   Double_t DE = (delta_e > 0 ? delta_e : GetEnergyLoss());

   return fIonRangeTable->GetLinearEIncFromDeltaEOfIon(GetType(), Z, A, DE, GetThickness(),
          (enum KVIonRangeTable::SolType)type, fAmasr, fTemp, fPressure);
}

//______________________________________________________________________________________//

Double_t KVMaterial::GetERes(Int_t Z, Int_t A, Double_t Einc)
{
   // \param[in] Z atomic number of incident ion
   // \param[in] A mass number of incident ion
   // \param[in] Einc kinetic energy of incident ion
   // \returns Residual energy \f$E_{res}=E_{inc}-\Delta E\f$ in MeV of an ion \f$Z, A\f$ after impinging on the absorber with kinetic energy \f$E_{inc}\f$
   //
   //\b Example of use:
   //~~~~{.cpp}
   //KVMaterial si("Si", 300*KVUnits::um);  // 300um silicon absorber
   //KVNucleus alpha("4He",30);             // 30MeV/u alpha particle
   //
   //si.DetectParticle(&alpha);             // particle is slowed by silicon
   //
   //si.GetEnergyLoss();
   //(double) 4.1371801                     // energy lost by particle in silicon
   //
   //alpha.GetEnergy();
   //(double) 115.86282                     // residual energy of alpha after silicon
   //
   //si.GetERes(2,4,120.0);
   //(double) 115.86282                     // residual energy calculated from incident energy
   //~~~~

   if (Z < 1) return 0.;
   if (IsGas() && GetPressure() == 0)
      return Einc;

   Double_t E_res =
      fIonRangeTable->GetEResOfIon(GetType(), Z, A, Einc, fThick, fAmasr, fTemp, fPressure);

   return E_res;
}

//___________________________________________________________________________________________________

void KVMaterial::DetectParticle(KVNucleus* kvp, TVector3* norm)
{
   //\param[in] kvp pointer to a KVNucleus object describing a charged ion
   //\param[in] norm [optional] vector normal to the material, oriented from the origin towards the material
   //
   //The energy loss \f$\Delta E\f$ of a charged particle traversing the absorber is calculated,
   //and the particle is slowed down by a corresponding amount
   //(the kinetic energy of the KVNucleus object passed as argument will be reduced by \f$\Delta E\f$, possibly to zero).
   //
   //If the unit normal vector is given, the effective thickness of the material 'seen' by the particle
   //depending on the orientation of its direction of motion with respect to the absorber is used for the calculation,
   //rather than the nominal thickness corresponding to ions impinging perpendicularly.

   kvp->SetIsDetected();//set flag to say that particle has been slowed down
   //If this is the first absorber that the particle crosses, we set a "reminder" of its
   //initial energy
   if (!kvp->GetPInitial())
      kvp->SetE0();

#ifdef DBG_TRGT
   cout << "detectparticle in material " << GetType() << " of thickness "
        << GetThickness() << endl;
#endif
   Double_t el = GetELostByParticle(kvp, norm);
   // set particle residual energy
   Double_t Eres = kvp->GetKE() - el;
   kvp->SetKE(Eres);
   // add to total of energy losses in absorber
   fELoss += el;
}

//__________________________________________________________________________________________

void KVMaterial::Clear(Option_t*)
{
   // Reset absorber - set stored energy lost by particles in absorber to zero
   fELoss = 0.0;
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVMaterial::Copy(TObject& obj) const
#else
void KVMaterial::Copy(TObject& obj)
#endif
{
   // Make a copy of this material object
   KVBase::Copy(obj);
   ((KVMaterial&) obj).SetMaterial(GetType());
   ((KVMaterial&) obj).SetMass(GetMass());
   ((KVMaterial&) obj).SetPressure(GetPressure());
   ((KVMaterial&) obj).SetTemperature(GetTemperature());
   ((KVMaterial&) obj).SetThickness(GetThickness());
}

//__________________________________________________________________________________________

Double_t KVMaterial::GetIncidentEnergyFromERes(Int_t Z, Int_t A, Double_t Eres)
{
   // \param[in] Z,A atomic & mass numbers of incident ion
   // \param[in] Eres residual energy of ion after absorber
   // \returns incident energy of ion \f$E_{inc}\f$ [MeV] deduced from residual energy \f$E_{res}=E_{inc}-\Delta E\f$.
   if (Z < 1) return 0.;

   return fIonRangeTable->
          GetLinearEIncFromEResOfIon(GetType(), Z, A, Eres, GetThickness(), fAmasr, fTemp, fPressure);
}

//__________________________________________________________________________________________

Double_t KVMaterial::GetEIncOfMaxDeltaE(Int_t Z, Int_t A)
{
   //\param[in] Z,A atomic and mass number of impinging ion
   //\returns incident energy in MeV for which the \f$\Delta E\f$-\f$E\f$ curve has a maximum

   if (Z < 1) return 0.;

   return fIonRangeTable->
          GetLinearEIncOfMaxDeltaEOfIon(GetType(), Z, A, GetThickness(), fAmasr, fTemp, fPressure);
}

//__________________________________________________________________________________________

Double_t KVMaterial::GetMaxDeltaE(Int_t Z, Int_t A)
{
   //\returns The maximum possible energy loss \f$\Delta E\f$ of a nucleus in the absorber
   //\param[in] Z,A atomic and mass number of the nucleus
   //
   //\sa GetEIncOfMaxDeltaE()
   //
   //\note For detectors, this is the maximum energy loss in the active layer.

   if (GetActiveLayer()) return GetActiveLayer()->GetMaxDeltaE(Z, A);

   if (Z < 1) return 0.;

   return fIonRangeTable->
          GetLinearMaxDeltaEOfIon(GetType(), Z, A, GetThickness(), fAmasr, fTemp, fPressure);
}

//__________________________________________________________________________________________

TGeoMedium* KVMaterial::GetGeoMedium(const Char_t* med_name)
{
   // By default, return pointer to TGeoMedium corresponding to this KVMaterial.
   //
   // \param[in] med_name [optional] if it corresponds to the name of an already existing
   // medium, we return a pointer to this medium, or a nullptr if it does not exist.
   //
   // `med_name = "Vacuum"` is a special case: if the "Vacuum" does not exist, we create it.
   //
   // Instance of geometry manager class TGeoManager must be created before calling this
   // method, otherwise nullptr will be returned.
   //
   // If the required TGeoMedium is not already available in the TGeoManager, we create
   // a new TGeoMedium corresponding to the properties of this KVMaterial.
   // The name of the TGeoMedium (and associated TGeoMaterial) is the name of the KVMaterial.
   //
   // \note For detectors, the material in question is that of the active layer (see KVDetector).

   if (!gGeoManager) return nullptr;

   if (strcmp(med_name, "")) {
      TGeoMedium* gmed = gGeoManager->GetMedium(med_name);
      if (gmed) return gmed;
      else if (!strcmp(med_name, "Vacuum")) {
         // create material
         TGeoMaterial* gmat = new TGeoMaterial("Vacuum", 0, 0, 0);
         gmat->SetTitle("Vacuum");
         gmed = new TGeoMedium("Vacuum", 0, gmat);
         gmed->SetTitle("Vacuum");
         return gmed;
      }
      return nullptr;
   }

   // if object is a KVDetector, we return medium corresponding to the active layer
   if (GetActiveLayer()) return GetActiveLayer()->GetGeoMedium();

   // for gaseous materials, the TGeoMedium/Material name is of the form
   //      gasname_pressure
   // e.g. C3F8_37.5 for C3F8 gas at 37.5 torr
   // each gas with different pressure has to have a separate TGeoMaterial/Medium
   TString medName;
   if (IsGas()) medName.Form("%s_%f", GetName(), GetPressure());
   else medName = GetName();

   TGeoMedium* gmed = gGeoManager->GetMedium(medName);

   if (gmed) return gmed;

   TGeoMaterial* gmat = gGeoManager->GetMaterial(medName);

   if (!gmat) {
      // create material
      gmat = GetRangeTable()->GetTGeoMaterial(GetName());
      gmat->SetPressure(GetPressure());
      gmat->SetTemperature(GetTemperature());
      gmat->SetTransparency(0);
      gmat->SetName(medName);
      gmat->SetTitle(GetName());
   }

   // create medium
   static Int_t numed = 1; // static counter variable used to number media
   gmed = new TGeoMedium(medName, numed, gmat);
   numed += 1;

   return gmed;
}

TGraph* KVMaterial::GetGraphOfDeltaEVsE(const KVNucleus& nuc, Int_t npts, Double_t Emin, Double_t Emax)
{
   // \param[in] nuc definition of charged particle
   // \param[in] npts number of points to use
   // \param[in] Emin minimum incident energy \f$E\f$
   // \param[in] Emax maximum incident energy \f$E\f$
   // \returns TGraph with the \f$\Delta E\f$-\f$E\f$ curve for this material for a given charged particle

   auto g = new TGraph;
   KVValueRange<Double_t> R(Emin, Emax);
   for (int i = 0; i < npts; ++i) {
      auto E = R.ValueIofN(i, npts);
      g->SetPoint(i, E, GetDeltaE(nuc.GetZ(), nuc.GetA(), E));
   }
   return g;
}

Double_t KVMaterial::GetEmaxValid(Int_t Z, Int_t A)
{
   //\param[in] Z,A atomic & mass numbers of ion
   // \returns maximum incident energy [MeV] for which range tables are valid
   // for this material and ion with \f$Z,A\f$.
   //
   //\note For detectors, the limit of validity for the material composing the active layer is returned (see KVDetector).
   if (GetActiveLayer()) return GetActiveLayer()->GetEmaxValid(Z, A);
   return fIonRangeTable->GetEmaxValid(GetType(), Z, A);
}

Double_t KVMaterial::GetPunchThroughEnergy(Int_t Z, Int_t A)
{
   //\param[in] Z,A atomic & mass numbers of ion
   // \returns incident energy \f$E_{inc}\f$ [MeV] for which ion \f$Z,A\f$ has a range equal to the
   // thickness of this absorber (see GetRange(), GetLinearRange()).

   return fIonRangeTable->GetLinearPunchThroughEnergy(GetType(), Z, A, GetThickness(), fAmasr, fTemp, fPressure);
}
