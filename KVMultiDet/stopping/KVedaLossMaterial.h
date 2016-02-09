//Created by KVClassFactory on Wed Feb  2 16:13:08 2011
//Author: frankland,,,,

#ifndef __KVEDALOSSMATERIAL_H
#define __KVEDALOSSMATERIAL_H

#include "KVIonRangeTableMaterial.h"
#include <Riostream.h>

class TGeoMaterial;

// maximum atomic number included in range tables
#define ZMAX_VEDALOSS 100
#define VERY_BIG_ENERGY  1.e+6

class KVedaLossMaterial : public KVIonRangeTableMaterial {

private:
   // internal variables used by RangeFunc/DeltaEFunc
   Double_t RF_Z;
   Double_t RF_A;
   Double_t* par;
   Double_t ran, adm, dleps, adn, riso, eps, DLEP, drande;
   Double_t thickness; // in g/cm**2

protected:
   Double_t fEmax[ZMAX_VEDALOSS];        //[ZMAX_VEDALOSS] Z-dependent maximum energy/nucleon for calculation to be valid
   Double_t fEmin[ZMAX_VEDALOSS];        //[ZMAX_VEDALOSS] Z-dependent minimum energy/nucleon for calculation to be valid
   Double_t fCoeff[ZMAX_VEDALOSS][14];  //[ZMAX_VEDALOSS][14] parameters for range tables

   static Bool_t fNoLimits;// if kTRUE, ignore max E limit for validity of calculation

   Double_t DeltaEFunc(Double_t*, Double_t*);
   Double_t EResFunc(Double_t*, Double_t*);
   Double_t RangeFunc(Double_t*, Double_t*);
   Double_t StoppingFunc(Double_t*, Double_t*);

public:
   KVedaLossMaterial();
   KVedaLossMaterial(const KVIonRangeTable*, const Char_t* name, const Char_t* type, const Char_t* state,
                     Double_t density, Double_t Z, Double_t A, Double_t MoleWt = 0.0);
   virtual ~KVedaLossMaterial();

   Bool_t ReadRangeTable(FILE* fp);
   Float_t GetEmaxValid(Int_t Z, Int_t A) const
   {
      return ((Z <= ZMAX_VEDALOSS && Z > 0) ? A * fEmax[Z - 1] : 0.0);
   };
   Float_t GetEminValid(Int_t Z, Int_t A) const
   {
      return ((Z <= ZMAX_VEDALOSS && Z > 0) ? A * fEmin[Z - 1] : 0.0);
   };

   virtual TF1* GetRangeFunction(Int_t Z, Int_t A, Double_t isoAmat = 0);
   virtual TF1* GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0);
   virtual TF1* GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0);
   virtual TF1* GetStoppingFunction(Int_t Z, Int_t A, Double_t isoAmat = 0);

   virtual Double_t GetRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat = 0.);
   virtual Double_t GetDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0.);

   static void SetNoLimits(Bool_t on = kTRUE)
   {
      // Normally all range, dE, Eres functions are limited to range 0<=E<=Emax,
      // where Emax is nominal max energy for which range tables are valid
      // (usually 400MeV/u for Z<3, 250MeV/u for Z>3)
      // if higher energies are required, call this static method in order to recalculate the Emax limits
      // in such way that:
      //     range function is always monotonically increasing function of Einc
      //     stopping power is concave (i.e. no minimum of stopping power followed by an increase)
      // at the most, the new limit will be 1 GeV/nucleon.
      // at the least, it will remain at the nominal (400 or 250 MeV/nucleon) level.
      fNoLimits = on;
   };

   void GetParameters(Int_t Zion, Int_t& Aion, Double_t*& rangepar);

   ClassDef(KVedaLossMaterial, 3) //Description of material properties used by KVedaLoss range calculation
};

#endif
