//Created by KVClassFactory on Mon Mar 16 11:45:14 2015
//Author: ,,,

#ifndef __KVFAZIAIDSISI_H
#define __KVFAZIAIDSISI_H

#include "KVFAZIAIDTelescope.h"
#include "KVIdentificationResult.h"

class KVFAZIADetector;
class KVIDZAGrid;

class KVFAZIAIDSiSi : public KVFAZIAIDTelescope {
private:

   KVIDZAGrid* fSiSiGrid;//! telescope's grid
   KVFAZIADetector* fSi1;//!
   KVFAZIADetector* fSi2;//!

public:
   KVFAZIAIDSiSi();
   virtual ~KVFAZIAIDSiSi();
   virtual UShort_t GetIDCode()
   {
      return kSi1Si2;
   };
   virtual void   Initialize();
   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVFAZIAIDSiSi, 1) //identification telescope for FAZIA Si-Si idcards
};

#endif
