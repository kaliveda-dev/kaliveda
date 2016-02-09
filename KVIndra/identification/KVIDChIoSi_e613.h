//Created by KVClassFactory on Wed Mar 12 14:49:28 2014
//Author: bonnet

#ifndef __KVIDCHIOSI_E613_H
#define __KVIDCHIOSI_E613_H

#include "KVIDChIoSi.h"

class KVIDChIoSi_e613 : public KVIDChIoSi {

protected:

   KVIDGChIoSi* FromFitChIoSiGrid;//!

public:
   KVIDChIoSi_e613();
   virtual ~KVIDChIoSi_e613();

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");
   void Initialize();
   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   ClassDef(KVIDChIoSi_e613, 1) //modification of the sequence of identification for e613
};

#endif
