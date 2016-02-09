//Created by KVClassFactory on Fri Feb 14 09:48:39 2014
//Author: John Frankland,,,

#ifndef __KVINDRAFAZIA_H
#define __KVINDRAFAZIA_H

#include "KVINDRA.h"

class KVINDRAFAZIA : public KVINDRA {
protected:

   TString fFGeoType;  //type of FAZIA geometry (="compact",...)
   Double_t fFDist;    //distance of FAZIA detectors from target (in cm)
   Double_t fFThetaMin;//minimum polar angle for compact geometry (in degrees)

   void BuildFAZIACompact();

public:
   KVINDRAFAZIA();
   virtual ~KVINDRAFAZIA();
   virtual void Build(Int_t run = -1);

   ClassDef(KVINDRAFAZIA, 1) //INDRA-FAZIA setups
};

#endif
