//Created by KVClassFactory on Wed Feb  2 16:13:08 2011
//Author: frankland,,,,

#ifndef __KVEDALOSSMATERIAL_H
#define __KVEDALOSSMATERIAL_H

#include "KVIonRangeTableMaterial.h"
#include <Riostream.h>
#include "TObjArray.h"
#include <vector>

class TGeoMaterial;
class KVedaLoss;

// maximum atomic number included in range tables
#define ZMAX_VEDALOSS 100
#define VERY_BIG_ENERGY  1.e+6

/**
  \class KVedaLossMaterial
  \ingroup Stopping
  \brief Description of material in the KVedaLoss range table

  \sa KVedaLoss
 */
class KVedaLossMaterial : public KVIonRangeTableMaterial {

private:
   static KVedaLoss* fgTable;
   // internal variables used by RangeFunc/DeltaEFunc
   Double_t RF_Z;
   Double_t RF_A;
   std::vector<Double_t>* par;
   Double_t ran, adm, dleps, adn, riso, eps, DLEP, drande;
   Double_t thickness; // in g/cm**2
   TObjArray fInvRange; //KVedaLossInverseRangeFunction objects

protected:
   std::vector<Double_t> fEmin;        //Z-dependent minimum energy/nucleon for calculation to be valid
   std::vector<Double_t> fEmax;        //Z-dependent maximum energy/nucleon for calculation to be valid
   std::vector< std::vector<Double_t> > fCoeff;   //parameters for range tables

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
      return (CheckIon(Z) ? A * fEmax[Z - 1] : 0.0);
   };
   Float_t GetEminValid(Int_t Z, Int_t A) const
   {
      return (CheckIon(Z) ? A * fEmin[Z - 1] : 0.0);
   };

   virtual TF1* GetRangeFunction(Int_t Z, Int_t A, Double_t isoAmat = 0);
   virtual TF1* GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0);
   virtual TF1* GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0);
   virtual TF1* GetStoppingFunction(Int_t Z, Int_t A, Double_t isoAmat = 0);

   virtual Double_t GetRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat = 0.);
   virtual Double_t GetDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetPunchThroughEnergy(Int_t Z, Int_t A, Double_t e, Double_t isoAmat = 0.);
   virtual Double_t GetEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat = 0.);

   static void SetNoLimits(Bool_t on = kTRUE)
   {
      // Normally all range, \f$dE\f$, \f$E_{res}\f$ functions are limited to range \f$0\leq E\leq E_{max}\f$,
      // where \f$E_{max}\f$ is nominal maximum energy for which range tables are valid
      // (usually 400MeV/u for \f$Z<3\f$, 250MeV/u for \f$Z>3\f$).
      //
      // If higher energies are required, call static method KVedaLoss::SetIgnoreEnergyLimits() **BEFORE ANY MATERIALS ARE CREATED**
      // in order to recalculate the \f$E_{max}\f$ limits in such a way that:
      //    -  range function is always monotonically increasing function of \f$E_{inc}\f$;
      //    -  stopping power is concave (i.e. no minimum of stopping power followed by an increase)
      //
      // Then, at the most, the new limit will be 1 GeV/nucleon, or
      // at the least, it will remain at the nominal (400 or 250 MeV/nucleon) level.
      fNoLimits = on;
   };

   void GetParameters(Int_t Zion, Int_t& Aion, std::vector<Double_t>& rangepar);
   static Bool_t CheckIon(Int_t Z)
   {
      return (Z > 0 && Z <= ZMAX_VEDALOSS);
   }

   ClassDef(KVedaLossMaterial, 4) //Description of material properties used by KVedaLoss range calculation
};

#endif
