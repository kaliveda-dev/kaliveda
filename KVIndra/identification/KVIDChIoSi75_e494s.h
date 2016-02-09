//Created by KVClassFactory on Fri Nov  2 12:13:31 2012
//Author: Guilain ADEMARD

#ifndef __KVIDCHIOSI75_E494S_H
#define __KVIDCHIOSI75_E494S_H

#include "KVIDChIoSi75.h"

class KVIDChIoSi75_e494s : public KVIDChIoSi75 {

public:
   KVIDChIoSi75_e494s();
   virtual ~KVIDChIoSi75_e494s();

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDChIoSi75_e494s, 1) //E503/E494S experiment INDRA identification using ChIo-Si75 matrices
};

#endif
