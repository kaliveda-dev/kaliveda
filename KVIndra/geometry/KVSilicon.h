#ifndef KVSILICON_H
#define KVSILICON_H

#include "KVINDRADetector.h"
#include "KVUnits.h"

class KVSilicon: public KVINDRADetector {

   void init();

public:

   KVSilicon();
   KVSilicon(Float_t thick /* um */);
   virtual ~ KVSilicon();

   void SetACQParams();

   Double_t GetPHD(Double_t dE, UInt_t Z);

   void SetMoultonPHDParameters(Double_t a1, Double_t a2, Double_t b1, Double_t b2);

   virtual Short_t GetCalcACQParam(KVACQParam*, Double_t) const;
   virtual TF1* GetELossFunction(Int_t Z, Int_t A);

   void SetThickness(Double_t thick /* um */)
   {
      // Sets thickness of active layer in microns

      KVINDRADetector::SetThickness(thick * KVUnits::um);
   }
   virtual Double_t GetThickness() const /* um */
   {
      // Returns thickness of active layer in microns
      return GetActiveLayer()->GetThickness() / KVUnits::um;
   }
   void DeduceACQParameters(KVEvent*, KVNumberList&);
   Double_t GetDeltaE(Int_t Z, Int_t A, Double_t Einc);

   ClassDef(KVSilicon, 9)       //INDRA forward-rings silicon detector
};

class KVSi75: public KVSilicon {
public:

   KVSi75();
   KVSi75(Float_t thick);
   virtual ~ KVSi75();
   const Char_t* GetArrayName();

   ClassDef(KVSi75, 1)         //80um silicon detector for INDRA etalon telescopes
};

class KVSiLi: public KVSilicon {
public:

   KVSiLi();
   KVSiLi(Float_t thick);
   virtual ~ KVSiLi();
   const Char_t* GetArrayName();

   ClassDef(KVSiLi, 2)         //2mm + 40um dead zone Si(Li) detector for INDRA etalon telescopes
};
#endif
