/*
$Id: KVIDSi75SiLi_camp5.h,v 1.6 2009/04/06 15:22:02 franklan Exp $
$Revision: 1.6 $
$Date: 2009/04/06 15:22:02 $
*/

//Created by KVClassFactory on Mon Oct 29 16:45:49 2007
//Author: franklan

#ifndef __KVIDSI75SILI_CAMP5_H
#define __KVIDSI75SILI_CAMP5_H

#include "KVIDSi75SiLi.h"
#include "KVIDZAGrid.h"

class KVIDSi75SiLi_camp5 : public KVIDSi75SiLi {
   KVIDZAGrid* fGGgrid;//! grid used for Si75(GG)-SiLi(PG) charge & mass identification
   KVIDZAGrid* fPGgrid;//! grid used for Si75(PG)-SiLi(PG) charge & mass identification
   KVIDZAGrid* fPGZgrid;//! grid used for Si75(PG)-SiLi(PG) charge identification
   KVDetector* fSi75;//!
   KVDetector* fSiLi;//!

public:

   KVIDSi75SiLi_camp5();
   virtual ~KVIDSi75SiLi_camp5();

   virtual void Initialize(void);
   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDSi75SiLi_camp5, 1) //Identification in Si75-SiLi telescopes for INDRA_camp5 dataset
};

#endif
