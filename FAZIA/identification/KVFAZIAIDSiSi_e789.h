//Created by KVClassFactory on Mon Mar 16 11:45:14 2015
//Author: ,,,

#ifndef __KVFAZIAIDSiSi_e789_H
#define __KVFAZIAIDSiSi_e789_H

#include "KVFAZIAIDSiSi.h"
#include "KVFAZIADetector.h"
#include "KVIDZAGrid.h"

class KVFAZIAIDSiSi_e789 : public KVFAZIAIDSiSi {

protected:
   KVIDZAGrid* fSiSiGrid;//! telescope's grid
   KVIDZAGrid* fSiSiGridQL1;//! telescope's grid low range

public:
   KVFAZIAIDSiSi_e789();
   virtual ~KVFAZIAIDSiSi_e789() {}

   void Initialize();
   Bool_t Identify(KVIdentificationResult* idr, Double_t /*x*/ = -1., Double_t /*y*/ = -1.);

   ClassDef(KVFAZIAIDSiSi_e789, 1) //identification telescope for E789 FAZIA Si-Si idcards
};

#endif
