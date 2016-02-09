//Created by KVClassFactory on Mon Feb 17 13:52:51 2014
//Author: John Frankland,,,

#ifndef __KVFAZIAIDSiPSA_H
#define __KVFAZIAIDSiPSA_H

#include "KVFAZIAIDTelescope.h"
#include "TGraph.h"

class KVFAZIAIDSiPSA : public KVFAZIAIDTelescope {

   TGraph* fRearThreshold;//! Z-dependent threshold extracted from published article

public:
   KVFAZIAIDSiPSA();
   virtual ~KVFAZIAIDSiPSA();
   Bool_t CheckTheoreticalIdentificationThreshold(KVNucleus*, Double_t = 0.);

   ClassDef(KVFAZIAIDSiPSA, 1) //PSA identification in first silicon of FAZIA telescopes
};

#endif
