//Created by KVClassFactory on Sun Jun 21 10:34:54 2015
//Author: Dijon Aurore

#ifndef __KVCHARGESTATEDISTRIB_H
#define __KVCHARGESTATEDISTRIB_H

#include "KVBase.h"
#include "KVNucleus.h"

class TF1;
class KVChargeStateDistrib : public KVBase {

protected:
   KVNucleus* fNuc;
   TF1*       fDistrib;

public:
   KVChargeStateDistrib();
   KVChargeStateDistrib(KVNucleus* nuc);
   virtual ~KVChargeStateDistrib();
   void Copy(TObject& obj) const;

   Double_t GetQmean0();
   Double_t GetQmean();
   Double_t GetQsig();
   TF1* GetDistribution(KVNucleus* nuc = NULL);
   Int_t GetRandomQ(KVNucleus* nuc = NULL);

   void SetNucleus(KVNucleus* nuc)
   {
      fNuc = nuc;
   }
   KVNucleus* GetNucleus() const
   {
      return fNuc;
   }


   ClassDef(KVChargeStateDistrib, 1) //Calculate charge state distribution
};

#endif
