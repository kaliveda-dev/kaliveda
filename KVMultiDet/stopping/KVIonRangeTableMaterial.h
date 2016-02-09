//Created by KVClassFactory on Fri Sep 28 11:34:14 2012
//Author: John Frankland,,,

#ifndef __KVIONRANGETABLEMATERIAL_H
#define __KVIONRANGETABLEMATERIAL_H

#include "KVBase.h"
#include <TString.h>
#include <KVList.h>
#include "KVIonRangeTable.h"

class TGeoMaterial;
class TF1;

#define RTT  62.36367e+03  // cm^3.Torr.K^-1.mol^-1
#define ZERO_KELVIN  273.15

class KVIonRangeTableMaterial : public KVBase {
protected:
   const KVIonRangeTable* fTable;//parent range table
   TString fState;               // state of material = "solid", "liquid", "gas", "unknown"
   KVList*   fComposition;      // composition of compound/mixture
   Bool_t fCompound;
   Bool_t fMixture;
   Double_t fDens;              // density of material in g/cm**3
   Double_t fZmat;              // effective atomic number of material
   Double_t fAmat;              // effective mass number of material
   Double_t fMoleWt;            // mass of one mole of substance in grammes

   TF1* fDeltaE; // function parameterising energy loss in material
   TF1* fEres; // function parameterising residual energy after crossing material
   TF1* fRange; // function parameterising range of charged particles in material
   TF1* fStopping; // function parameterising stopping power of charged particles in material

public:
   KVIonRangeTableMaterial();
   KVIonRangeTableMaterial(const KVIonRangeTable*, const Char_t* name, const Char_t* symbol, const Char_t* state,
                           Double_t density = -1, Double_t Z = -1, Double_t A = -1);
   KVIonRangeTableMaterial(const KVIonRangeTableMaterial&) ;
   virtual ~KVIonRangeTableMaterial();

   virtual void Initialize();
   const KVIonRangeTable* GetTable() const;

   void ls(Option_t* = "") const;
   void Print(Option_t* = "") const;
   virtual Float_t GetEmaxValid(Int_t /* Z */, Int_t /* A */) const
   {
      return 1.e+4;
   };
   virtual Float_t GetEminValid(Int_t /* Z */, Int_t /* A */) const
   {
      return 0.0;
   };

   void Copy(TObject&) const;
   const Char_t* GetSymbol() const
   {
      return GetTitle();
   };
   Double_t GetMass() const
   {
      return fAmat;
   };
   Double_t GetZ() const
   {
      return fZmat;
   };
   Double_t GetMoleWt() const
   {
      // mass of 1 mole of substance in grammes
      return fMoleWt;
   };
   Double_t GetDensity() const
   {
      // return density of material in g/cm**3.
      // for gaseous materials, you must call SetGasDensity(T,P) first in order
      // to define the temperature and pressure of the gas.
      return fDens;
   };
   void SetDensity(Double_t d)
   {
      // set density of material in g/cm**3.
      // no effect for gaseous materials.
      fDens = d;
   };
   void SetTemperatureAndPressure(Double_t T, Double_t P)
   {
      // for a gaseous material, calculate density in g/cm**3 according to given
      // conditions of temperature (T, in degrees celsius) and pressure (P, in Torr)
      if (IsGas() && P == 0) fDens = 0;
      if (IsGas() && T > 0 && P > 0) fDens = (fMoleWt * P) / ((T + ZERO_KELVIN) * RTT);
   };
   void SetState(const Char_t* st)
   {
      // Set state of material: gas, liquid, solid, ...
      fState = st;
   };
   Bool_t IsGas() const
   {
      // returns kTRUE if material is in gaseous state
      return (fState == "gas");
   };

   Bool_t IsCompound() const
   {
      // returns kTRUE if material is a compound, kFALSE if it is an element or a mixture
      return (fCompound);
   };
   Bool_t IsMixture() const
   {
      // returns kTRUE if material is a mixture, kFALSE if it is an element or a compound
      return (fMixture);
   };

   KVList* GetComposition() const
   {
      // return list of elements in compound material
      return fComposition;
   };

   void AddCompoundElement(Int_t Z, Int_t A, Int_t Natoms);
   void AddMixtureElement(Int_t Z, Int_t A, Int_t Natoms, Double_t Proportion);

   TGeoMaterial* GetTGeoMaterial() const;

   virtual TF1* GetRangeFunction(Int_t Z, Int_t A, Double_t isoAmat = 0) = 0;
   virtual TF1* GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0) = 0;
   virtual TF1* GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0) = 0;
   virtual TF1* GetStoppingFunction(Int_t Z, Int_t A, Double_t isoAmat = 0) = 0;

   void PrintRangeTable(Int_t Z, Int_t A, Double_t isoAmat = 0, Double_t units = KVUnits::cm, Double_t T = -1, Double_t P = -1);
   void PrintComposition(std::ostream&) const;

   virtual Double_t GetRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat = 0.);
   virtual Double_t GetLinearRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat = 0, Double_t T = -1., Double_t P = -1.);

   virtual Double_t GetDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetLinearDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.);

   virtual Double_t GetEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetLinearEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.);

   virtual Double_t GetEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetLinearEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.);

   virtual Double_t GetEIncFromDeltaEOfIon(Int_t Z, Int_t A, Double_t DeltaE, Double_t e, enum KVIonRangeTable::SolType type = KVIonRangeTable::kEmax, Double_t isoAmat = 0.);
   virtual Double_t GetLinearEIncFromDeltaEOfIon(Int_t Z, Int_t A, Double_t DeltaE, Double_t e, enum KVIonRangeTable::SolType type = KVIonRangeTable::kEmax, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.);

   Double_t GetDeltaEFromEResOfIon(Int_t Z, Int_t A, Double_t ERes, Double_t e, Double_t isoAmat = 0.)
   {
      // Calculates energy loss (in MeV) of an ion (Z,A) from residual energy ERes (MeV) after thickness e (in g/cm**2).
      // Give Amat to change default (isotopic) mass of material,
      return GetEIncFromEResOfIon(Z, A, ERes, e, isoAmat) - ERes;
   }

   Double_t GetLinearDeltaEFromEResOfIon(Int_t Z, Int_t A, Double_t ERes, Double_t e,
                                         Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.)
   {
      // Calculates energy loss (in MeV) of an ion (Z,A) from residual energy ERes (MeV) after thickness e (in cm).
      // Give Amat to change default (isotopic) mass of material,
      // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
      return GetLinearEIncFromEResOfIon(Z, A, ERes, e, isoAmat, T, P) - ERes;
   }

   virtual Double_t GetPunchThroughEnergy(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0.) ;
   virtual Double_t GetLinearPunchThroughEnergy(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.) ;

   virtual Double_t GetMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetEIncOfMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetLinearMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.);
   virtual Double_t GetLinearEIncOfMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0., Double_t T = -1., Double_t P = -1.);

   ClassDef(KVIonRangeTableMaterial, 1) //Material for use in energy loss & range calculations
};

#endif
