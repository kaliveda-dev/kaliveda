/*
$Id: KVIDChIoSi_e475s.h,v 1.1 2008/02/19 15:01:11 ebonnet Exp $
$Revision: 1.1 $
$Date: 2008/02/19 15:01:11 $
*/

//Created by KVClassFactory on Tue Feb 19 15:56:56 2008
//Author: eric bonnet,,,

#ifndef __KVIDCHIOSI_E475S_H
#define __KVIDCHIOSI_E475S_H

#include "KVIDChIoSi.h"

class KVIDChIoSi_e475s : public KVIDChIoSi {

public:
   KVIDChIoSi_e475s();
   virtual ~KVIDChIoSi_e475s();

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDChIoSi_e475s, 1) //derivation of KVIDChIoSi class for E475s experiment
};

#endif
