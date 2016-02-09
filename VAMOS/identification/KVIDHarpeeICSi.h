//Created by KVClassFactory on Thu Dec 12 16:37:34 2013
//Author: Guilain ADEMARD

#ifndef __KVIDHARPEEICSI_H
#define __KVIDHARPEEICSI_H

#include "KVVAMOSIDTelescope.h"

class KVIDHarpeeICSi : public KVVAMOSIDTelescope {

public:
   KVIDHarpeeICSi();
   virtual ~KVIDHarpeeICSi();

   ClassDef(KVIDHarpeeICSi, 1) //DeltaE-E identification telescope in VAMOS with Harpee's IC-Si detectors
};

#endif
