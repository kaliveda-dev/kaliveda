/*
$Id: KVIDChIoSi5.h,v 1.1 2007/12/10 13:02:13 franklan Exp $
$Revision: 1.1 $
$Date: 2007/12/10 13:02:13 $
*/

//Created by KVClassFactory on Mon Dec 10 12:58:26 2007
//Author: franklan

#ifndef __KVIDCHIOSI5_H
#define __KVIDCHIOSI5_H

#include "KVIDChIoSi.h"

class KVIDChIoSi5 : public KVIDChIoSi {

public:
   KVIDChIoSi5();
   virtual ~KVIDChIoSi5();

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDChIoSi5, 1) //INDRA 5th campaign ChIo-Si identification
};

#endif
