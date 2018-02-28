//Created by KVClassFactory on Thu Sep 27 14:50:08 2012
//Author: John Frankland,,,

#ifndef __KVRANGEYANEZMATERIAL_H
#define __KVRANGEYANEZMATERIAL_H

#include "KVIonRangeTableMaterial.h"

class KVRangeYanezMaterial : public KVIonRangeTableMaterial {
   Int_t fTableType;//=0 for Northcliffe-Schilling (<12 MeV/u), =1 for Hubert et al (2.5<E/A<500 MeV), =2 for interpolated (0<E/A<500 MeV)

   Int_t fNelem;                             //number of elements in material
   Int_t iabso;                                //value of iabso argument for function calls
   struct Elem {
      Int_t z;
      Int_t a;
      Double_t w;
   } fAbsorb[10];   //list of elements
   Double_t thickness;                  //! in g/cm**2
   Int_t Zp, Ap;                             //!Z,A of incident projectile ion
   Double_t error;                        //!calculated error in MeV

protected:
   void PrepareRangeLibVariables(Int_t Z, Int_t A);
   Double_t DeltaEFunc(Double_t*, Double_t*);
   Double_t RangeFunc(Double_t*, Double_t*);
   Double_t EResFunc(Double_t*, Double_t*);

   void MakeFunctionObjects();

public:
   KVRangeYanezMaterial();
   KVRangeYanezMaterial(const KVRangeYanezMaterial&) ;
   KVRangeYanezMaterial(const KVIonRangeTable* t, const Char_t* name, const Char_t* symbol, const Char_t* state,
                        Double_t density = -1, Int_t Z = -1, Int_t A = -1);
   virtual ~KVRangeYanezMaterial();
   void Copy(TObject&) const;
   void Initialize();

   TF1* GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0);
   TF1* GetRangeFunction(Int_t Z, Int_t A, Double_t isoAmat = 0);
   TF1* GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat = 0);
   TF1* GetStoppingFunction(Int_t, Int_t, Double_t = 0)
   {
      Warning("GetStoppingFunction", "Not available for Yanez Range tables");
      return 0;
   };
   void SetTableType(int type)
   {
      fTableType = type;
   };

   virtual Double_t GetEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat = 0.);

   ClassDef(KVRangeYanezMaterial, 1) //Description of absorber for the Range dE/dx and range library (Ricardo Yanez)
};

#endif
