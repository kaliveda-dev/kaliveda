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
#include "TF1.h"
#include "TVector3.h"
#include "Riostream.h"
class KVIonRangeTable;

class KVNucleus;
class TGeoMedium;
class TGeoVolume;

class KVMaterial: public KVBase {

protected:
   static KVIonRangeTable* fIonRangeTable; //  pointer to class used to calculate charged particle ranges & energy losses

   TVector3 fNormToMat;//!dummy vector for calculating normal to absorber

   TGeoVolume* fAbsorberVolume;//!pointer to corresponding volume in ROOT geometry

public:

private:
   Int_t fAmasr;                // isotopic mass of element
   Double_t fThick;              // area density of absorber in g/cm**2
   Double_t fPressure;        // gas pressure in torr
   Double_t fTemp;            // gas temperature in degrees celsius
   Double_t fELoss;             //total of energy lost by all particles traversing absorber

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

   static KVIonRangeTable* GetRangeTable();

   void init();
   virtual ~ KVMaterial();
   void SetMass(Double_t a);
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

   virtual void DetectParticle(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetELostByParticle(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetParticleEIncFromERes(KVNucleus*, TVector3* norm = 0);

   virtual void Print(Option_t* option = "") const;
   virtual Double_t GetEnergyLoss()
   {
      return fELoss;
   };
   virtual void SetEnergyLoss(Double_t e)
   {
      fELoss = e;
   };

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   virtual void Clear(Option_t* opt = "");

   virtual Double_t GetEmaxValid(Int_t Z, Int_t A);
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
      //to facilitate polymorphism with KVDetector class
      return 0;
   };

   virtual void SetPressure(Double_t);
   virtual void SetTemperature(Double_t);

   virtual Double_t GetPressure() const;
   virtual Double_t GetTemperature() const;

   Bool_t IsIsotopic() const;
   Bool_t IsNat() const;

   Bool_t IsGas() const;

   virtual const TVector3& GetNormal()
   {
      // Return vector normal to surface of absorber. For a KVMaterial, this is (0,0,1) as
      // a basic absorber has no orientation. Rederived in child classes KVTarget and KVDetector.
      return fNormToMat;
   };

   virtual TGeoMedium* GetGeoMedium(const Char_t* /*med_name*/ = "");
   virtual void SetAbsGeoVolume(TGeoVolume* v)
   {
      fAbsorberVolume = v;
   };
   virtual TGeoVolume* GetAbsGeoVolume() const
   {
      // Returns pointer to volume representing this absorber in the ROOT geometry.
      return fAbsorberVolume;
   };

   ClassDef(KVMaterial, 6)      // Class describing physical materials used to construct detectors & targets
};

#endif
