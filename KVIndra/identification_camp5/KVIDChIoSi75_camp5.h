//Created by KVClassFactory on Mon Jun 22 16:47:20 2009
//Author: John Frankland,,,

#ifndef __KVIDCHIOSI75_CAMP5_H
#define __KVIDCHIOSI75_CAMP5_H

#include "KVIDChIoSi75.h"

class KVIDChIoSi75_camp5 : public KVIDChIoSi75 {
   KVIDZAGrid* fGGgrid;//!
   KVIDZAGrid* fPGgrid;//!

public:
   KVIDChIoSi75_camp5();
   virtual ~KVIDChIoSi75_camp5();

   virtual void Initialize(void);
   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDChIoSi75_camp5, 1) //ChIo-Si(75) identification for INDRA 5th campaign data
};

#endif
