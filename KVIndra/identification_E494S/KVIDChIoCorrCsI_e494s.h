//Created by KVClassFactory on Thu Sep 20 17:24:51 2012
//Author: Ademard Guilain

#ifndef __KVIDCHIOCORRCSI_E494S_H
#define __KVIDCHIOCORRCSI_E494S_H

#include "KVIDChIoCorrCsI.h"

class KVIDChIoCorrCsI_e494s : public KVIDChIoCorrCsI {

public:
   KVIDChIoCorrCsI_e494s();
   virtual ~KVIDChIoCorrCsI_e494s();

   virtual Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDChIoCorrCsI_e494s, 1) //ChIo-CsI identification with grids for E494S
};

#endif
