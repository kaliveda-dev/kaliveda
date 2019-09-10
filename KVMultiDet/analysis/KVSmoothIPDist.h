//Created by KVClassFactory on Tue Jul 23 15:24:27 2019
//Author: John Frankland,,,

#ifndef __KVSMOOTHIPDIST_H
#define __KVSMOOTHIPDIST_H

#include "KVBase.h"
#include "TF1.h"

class KVSmoothIPDist : public KVBase {
   TH1*  fHisto;//! last fitted histogram
   TF1 fIPdist;// impact parameter distribution
   TF1 fSigmaR;// total reaction cross section
   TF1 fCentrality;//

public:
   KVSmoothIPDist();
   KVSmoothIPDist(TH1*);
   virtual ~KVSmoothIPDist() {}

   void FitIPDist(TH1*);

   Double_t GetCrossSection() const;
   Double_t GetB0() const
   {
      return fIPdist.GetParameter(1);
   }
   Double_t GetDeltaB() const
   {
      return fIPdist.GetParameter(2);
   }
   Double_t GetCrossSectionPerEvent() const;
   void SetB0(Double_t x)
   {
      fIPdist.SetParameter(1, x);
      fCentrality.SetParameter(0, x);
   }
   void SetDeltaB(Double_t x)
   {
      fIPdist.SetParameter(2, x);
      fSigmaR.SetParameter(0, x);
      fCentrality.SetParameter(1, x);
   }
   void SetDeltaB_WithConstantCrossSection(Double_t deltab, Double_t sigmaR = 0);
   const TF1& GetIPDist()
   {
      return fIPdist;
   }
   const TF1& GetSigmaR()
   {
      return fSigmaR;
   }
   const TF1& GetCentrality()
   {
      return fCentrality;
   }
   Double_t Calculate_b(Double_t centrality) const;

   void NormalizeIPDistToCrossSection()
   {
      // Normalizing to total cross section,
      // IP distribution will be probability distribution

      fIPdist.SetParameter(0, 1. / GetCrossSection());
   }

   ClassDef(KVSmoothIPDist, 1) //Triangular IP distribution with smooth fall-off
};

#endif
