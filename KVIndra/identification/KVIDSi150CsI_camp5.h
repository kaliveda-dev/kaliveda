//Created by KVClassFactory on Wed Jun 10 16:45:12 2009
//Author: John Frankland,,,

#ifndef __KVIDSI150CSI_CAMP5_H
#define __KVIDSI150CSI_CAMP5_H

#include "KVIDSiCsI.h"
#include "KVSilicon.h"
#include "KVCsI.h"
#include "KVIDZAGrid.h"

class KVIDSi150CsI_camp5 : public KVIDSiCsI {
   KVIDZAGrid* fZAGrid;//! identification grid with isotopes (small Z)
   KVIDZAGrid* fZGrid;//! identification grid with 1 line per Z (large Z)
   KVSilicon* fSi;//! silicon detector
   KVACQParam* fSIPG;//! silicon detector PG acquisition parameter
   KVCsI* fCsI;//! csi detector

public:
   KVIDSi150CsI_camp5();
   virtual ~KVIDSi150CsI_camp5();

   virtual void Initialize(void);
   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");
   Double_t GetPedestalY(Option_t* opt = "");

   ClassDef(KVIDSi150CsI_camp5, 1) //Si(150)-CsI identifications for 5th campaign INDRA data
};

#endif
