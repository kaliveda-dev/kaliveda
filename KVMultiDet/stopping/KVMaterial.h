/***************************************************************************
                          kvmaterial.h  -  description
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

#ifndef KVMATERIAL_H
#define KVMATERIAL_H

#include "RVersion.h"
#include "KVBase.h"
#include "KVList.h"
#include "KVUnits.h"
#include "TF1.h"
#include "TVector3.h"
#include "Riostream.h"
class KVIonRangeTable;

class KVNucleus;
class TGeoMedium;
class TGeoVolume;
class TGraph;

/**
  \class KVMaterial
  \ingroup Stopping
  \brief  Description of physical materials used to construct detectors & targets; interface to range tables

 KVMaterial is a class for describing the properties of physical materials with which charged nuclear species
 may interact in the course of heavy-ion reactions.
 It is the base class for all detector and target classes. It provides an easy-to-use interface to range tables and
 energy loss calculations for charged particles.

 The list of available material types depends on the underlying range table: this list can be obtained or visualised like so:
 ~~~~{.cpp}
 KVMaterial::GetRangeTable()->Print(); // print infos on range table

 KVMaterial::GetRangeTable()->GetListOfMaterials()->ls(); // retrieve pointer to list

 OBJ: TObjArray   TObjArray   An array of objects : 0
  OBJ: TNamed  Silicon  Si : 0 at: 0x55a63a979390
  OBJ: TNamed  Mylar Myl : 0 at: 0x55a63d6a95f0
  OBJ: TNamed  Plastic  NE102 : 0 at: 0x55a63d64ea90
  OBJ: TNamed  Nickel   Ni : 0 at: 0x55a63d6afb90
  OBJ: TNamed  Octofluoropropane C3F8 : 0 at: 0x55a63a9f8e00
 etc. etc.
 ~~~~

 Materials can be created in a variety of ways, using either the full name or symbolic name from the previous list:

~~~~{.cpp}
KVMaterial si("Silicon", 300*KVUnits::um); // 300 microns of silicon

KVMaterial gas("C3F8", 5*KVUnits::cm, 30*KVUnits::mbar); // 5cm of C3F8 at 30mbar pressure

KVMaterial ni_target(350*KVUnits::ug, "Ni") // Nickel with area density 350ug/cm**2
~~~~

Once defined, energy loss calculations can be easily performed:

~~~~{.cpp}
si.GetDeltaE(2, 4, 50.0); // energy loss of 50MeV alpha particle in 300um of Silicon

ni_target.GetPunchThroughEnergy(6,12); // punch through for 12C ions in 350ug/cm**2 of Nickel
~~~~

Several methods are also provided in order to deduce either incident energy, \f$E_{inc}\f$, energy loss \f$\Delta E\f$,
or residual energy, \f$E_{res}=E_{inc}-\Delta E\f$, from one of the others. All such methods are summarized in the following table:

|                     | calculate \f$E_{inc}\f$     | calculate  \f$\Delta E\f$ | calculate \f$E_{res}\f$ |
|---------------------|-----------------------------|---------------------------|-------------------------|
| from \f$E_{inc}\f$  |            -                |        GetDeltaE()        |       GetERes()         |
^                     |                             | GetELostByParticle()      |                         |
| from \f$\Delta E\f$ | GetIncidentEnergy()         |             -             |   GetEResFromDeltaE()   |
| from \f$E_{res}\f$  | GetIncidentEnergyFromERes() | GetDeltaEFromERes()       |           -             |
^                     | GetParticleEIncFromERes()   |                           |                         |

*/
class KVMaterial: public KVBase {

protected:
   static KVIonRangeTable* fIonRangeTable; //  pointer to class used to calculate charged particle ranges & energy losses

   TGeoVolume* fAbsorberVolume;//!pointer to corresponding volume in ROOT geometry

public:

private:
   Int_t fAmasr;                // isotopic mass of element
   Double_t fThick;              // area density of absorber in g/cm**2
   Double_t fPressure;        // gas pressure in torr
   Double_t fTemp;            // gas temperature in degrees celsius
   mutable Double_t fELoss;             //total of energy lost by all particles traversing absorber

public:
   enum SolType {
      kEmax,
      kEmin
   };
   KVMaterial();
   KVMaterial(const Char_t* type, const Double_t thick = 0.0);
   KVMaterial(const Char_t* gas, const Double_t thick, const Double_t pressure, const Double_t temperature = 19.0);
   KVMaterial(Double_t area_density, const Char_t* type);
   KVMaterial(const KVMaterial&);
   ROOT_COPY_ASSIGN_OP(KVMaterial)

   static KVIonRangeTable* GetRangeTable();
   static KVIonRangeTable* ChangeRangeTable(const Char_t* name);

   void init();
   virtual ~ KVMaterial();
   void SetMass(Int_t a);
   virtual void SetMaterial(const Char_t* type);
   Double_t GetMass() const;
   Double_t GetZ() const;
   Double_t GetDensity() const;
   void SetAreaDensity(Double_t dens /* g/cm**2 */);
   Double_t GetAreaDensity() const;
   virtual void SetThickness(Double_t thick /* cm */);
   virtual Double_t GetThickness() const;
   Double_t GetEffectiveThickness(TVector3& norm, TVector3& direction);
   Double_t GetEffectiveAreaDensity(TVector3& norm, TVector3& direction);

   virtual void DetectParticle(KVNucleus*, TVector3* norm = nullptr);
   virtual Double_t GetELostByParticle(KVNucleus*, TVector3* norm = nullptr);
   virtual Double_t GetParticleEIncFromERes(KVNucleus*, TVector3* norm = nullptr);

   virtual void Print(Option_t* option = "") const;
   virtual Double_t GetEnergyLoss() const
   {
      // Returns total energy loss [MeV] of charged particles in the absorber, which may be defined either by
      // a call to SetEnergyLoss(), either by repeated calls to DetectParticle(). In the latter case,
      // if DetectParticle() is called several times for different charged particles, this method returns the sum of all energy losses
      // for all charged particles.
      //
      // The total energy loss is set to zero by calling method Clear().
      return fELoss;
   }
   virtual void SetEnergyLoss(Double_t e) const
   {
      // Define the total energy loss [MeV] of charged particles in the absorber
      // \sa GetEnergyLoss()
      fELoss = e;
   }

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   virtual void Clear(Option_t* opt = "");

   Double_t GetEmaxValid(Int_t Z, Int_t A);
   virtual Double_t GetIncidentEnergy(Int_t Z, Int_t A, Double_t delta_e =
                                         -1.0, enum SolType type = kEmax);
   virtual Double_t GetIncidentEnergyFromERes(Int_t Z, Int_t A, Double_t Eres);
   virtual Double_t GetDeltaE(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetDeltaEFromERes(Int_t Z, Int_t A, Double_t Eres);
   virtual Double_t GetERes(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetEResFromDeltaE(Int_t Z, Int_t A, Double_t dE =
                                         -1.0, enum SolType type = kEmax);
   virtual Double_t GetEIncOfMaxDeltaE(Int_t Z, Int_t A);
   virtual Double_t GetMaxDeltaE(Int_t Z, Int_t A);

   virtual Double_t GetRange(Int_t Z, Int_t A, Double_t Einc);
   virtual Double_t GetLinearRange(Int_t Z, Int_t A, Double_t Einc);

   virtual Double_t GetPunchThroughEnergy(Int_t Z, Int_t A);

   virtual KVMaterial* GetActiveLayer() const
   {
      // \returns nullptr (see overrides in derived classes)
      return nullptr;
   }

   virtual void SetPressure(Double_t);
   virtual void SetTemperature(Double_t);

   virtual Double_t GetPressure() const;
   virtual Double_t GetTemperature() const;

   Bool_t IsIsotopic() const;
   Bool_t IsNat() const;

   Bool_t IsGas() const;

   virtual TGeoMedium* GetGeoMedium(const Char_t* /*med_name*/ = "");
   virtual void SetAbsGeoVolume(TGeoVolume* v)
   {
      // Link this material to a volume in a ROOT geometry
      fAbsorberVolume = v;
   }
   virtual TGeoVolume* GetAbsGeoVolume() const
   {
      // Returns pointer to volume representing this absorber in a ROOT geometry.
      return fAbsorberVolume;
   }

   virtual TGraph* GetGraphOfDeltaEVsE(const KVNucleus& nuc, Int_t npts, Double_t Emin, Double_t Emax);

   ClassDef(KVMaterial, 6)// Class describing physical materials used to construct detectors & targets
};

#endif
