//Created by KVClassFactory on Thu Dec 12 16:37:53 2013
//Author: Guilain ADEMARD

#ifndef __KVIDHARPEESICSI_H
#define __KVIDHARPEESICSI_H

#include <cassert>

#include "KVHarpeeCsI.h"
#include "KVHarpeeSi.h"
#include "KVVAMOSCodes.h"
#include "KVVAMOSIDTelescope.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVIDHarpeeSiCsI : public KVVAMOSIDTelescope {

public:

   KVIDHarpeeSiCsI();
   virtual ~KVIDHarpeeSiCsI();

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDHarpeeSiCsI, 1) //DeltaE-E identification telescope in VAMOS with Harpee's Si-CsI detectors
};

#endif

