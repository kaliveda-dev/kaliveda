//Created by KVClassFactory on Thu Jun 17 16:10:54 2010
//Author: bonnet

#ifndef __KVCOUPLE_H
#define __KVCOUPLE_H

#include "Rtypes.h"
#include "TMath.h"

/**
  \class KVCouple
  \brief Handles binary decomposition of an integer
  \ingroup Analysis
 */
class KVCouple {

protected:
   std::vector<Int_t> tz1;
   std::vector<Int_t> tz2;
   Int_t current_idx;
   Int_t zlim;
   Int_t nbre;
   Int_t zfra;
   Int_t zmax;
   Bool_t init;

public:
   KVCouple();

   KVCouple(Int_t zf, Int_t zsup);
   KVCouple(Int_t zf, Int_t zsup, Int_t div);

   void ResetVars()
   {
      current_idx = 0;
      zlim = 0;
      nbre = 0;
      zfra = 0;
      zmax = 0;
      init = kFALSE;
   }

   virtual ~KVCouple()
   {
      ResetVars();
      init = kFALSE;
   }

   Int_t GetZ1(Int_t idx)
   {
      return tz1[idx];
   }
   Int_t GetZ2(Int_t idx)
   {
      return tz2[idx];
   }
   Int_t GetNbreCouples()
   {
      return nbre;
   }


   ClassDef(KVCouple, 1) //Handle binary decomposition of an integer
};

#endif
