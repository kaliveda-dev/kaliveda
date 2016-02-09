//Created by KVClassFactory on Thu Dec 20 14:56:27 2012
//Author: John Frankland,,,

#ifndef __KVEDALOSSRANGEFITTER_H
#define __KVEDALOSSRANGEFITTER_H

#include "KVBase.h"
#include <KVIonRangeTableMaterial.h>
#include "KVedaLossMaterial.h"
#include <TF1.h>
#include <KVedaLoss.h>

class TGraph;

class KVedaLossRangeFitter : public KVBase {
   KVedaLoss VEDALOSS;

protected:
   KVIonRangeTableMaterial* fMaterial;// range table to fit
   KVedaLossMaterial* fClosestVedaMat;// closest known VEDALOSS material
   TF1* fRangeFunction;
   Int_t Aref;
   Double_t range(Double_t*, Double_t*);

public:
   KVedaLossRangeFitter();
   virtual ~KVedaLossRangeFitter();

   void SetMaterial(KVIonRangeTableMaterial* m);
   void SetInitialParameters(Int_t Z);

   TGraph* GenerateRangeTable(Int_t Z);
   TGraph* GenerateVEDALOSSRangeTable(Int_t Z);
   Int_t FitRangeTable(TGraph*);

   void DoFits(TString output_file, Int_t Zmin = 1, Int_t Zmax = 100);
   void PrintFitParameters(Int_t, std::ostream&);
   void PrintFitHeader(std::ostream&);

   ClassDef(KVedaLossRangeFitter, 1) //Fit a range table using the VEDALOSS functional
};

#endif
