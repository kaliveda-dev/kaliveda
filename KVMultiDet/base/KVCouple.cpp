//Created by KVClassFactory on Thu Jun 17 16:10:54 2010
//Author: bonnet

#include "KVCouple.h"

ClassImp(KVCouple)


KVCouple::KVCouple()
{
   // Default constructor
}

//----------------------------------------------
KVCouple::KVCouple(Int_t zf, Int_t zsup)
{

   //printf("zf=%d zmax=%d\n",zf,zsup);
   zfra = zf;
   zmax = zsup;

   Int_t div = 2;

   zlim = zfra / div;
   Double_t zmoy = zfra / Double_t(div);
   Double_t diff = zmoy - zlim;
   Int_t sup = TMath::Nint(diff * div);
   if (sup >= 1) {
      zlim += 1;
   }
   Int_t zref = 0;
   ((zfra >= zmax) ? zref = zmax : zref = zfra);

   nbre = zref - zlim + 1;
   if (nbre > 0) {
      tz1.reserve(nbre);
      tz2.reserve(nbre);
      Int_t z1 = 0;
      for (Int_t ii = 0; ii < nbre; ii += 1) {
         z1 = zref - ii;
         tz1[ii]  =  z1;
         tz2[ii]  =  zfra - z1;
         //printf("      %d    %d %d\n",ii,z1,zfra-z1);
      }
      init = kTRUE;
   }
   else {
      //return;
      ResetVars();
   }

}

//----------------------------------------------
KVCouple::KVCouple(Int_t zf, Int_t zsup, Int_t div)
{

   zfra = zf;
   zmax = zsup;

   zlim = zfra / div;
   Double_t zmoy = zfra / Double_t(div);
   Double_t diff = zmoy - zlim;
   Int_t sup = TMath::Nint(diff * div);
   if (sup >= 1) {
      zlim += 1;
   }

   if (zfra >= zmax) {
      //cas 1
      nbre = zmax - zlim + 1;
      if (nbre > 0) {

         tz1.reserve(nbre);
         tz2.reserve(nbre);
         Int_t z1 = 0;
         for (Int_t ii = 0; ii < nbre; ii += 1) {
            z1 = zmax - ii;
            tz1[ii]  =  z1;
            tz2[ii]  =  zfra - z1;
         }
         init = kTRUE;
      }
      else {
         //return;
         ResetVars();
      }
   }
   else {
      nbre = zfra - zlim + 1;
      if (nbre > 0) {

         tz1.reserve(nbre);
         tz2.reserve(nbre);
         Int_t z1 = 0;
         for (Int_t ii = 0; ii < nbre; ii += 1) {
            z1 = zfra - ii;
            tz1[ii]  =  z1;
            tz2[ii]  =  zfra - z1;
         }
         init = kTRUE;
      }
      else {
         //return;
         ResetVars();
      }
   }
}
