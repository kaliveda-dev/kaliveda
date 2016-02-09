/*
$Id: KVIDChIoSiCorr.h,v 1.1 2007/12/10 13:02:13 franklan Exp $
$Revision: 1.1 $
$Date: 2007/12/10 13:02:13 $
*/

//Created by KVClassFactory on Mon Dec 10 12:58:26 2007
//Author: franklan

#ifndef __KVIDCHIOSICORR_H
#define __KVIDCHIOSICORR_H

#include "KVIDChIoSi.h"
#include "KVIDGChIoSi_e494s.h"
#include "KVINDRADetector.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVIdentificationResult;

class KVIDChIoSiCorr : public KVIDChIoSi {

   Double_t GetIDMapXY(KVINDRADetector* det, Option_t* opt);

public:

   KVIDChIoSiCorr() {};
   virtual ~KVIDChIoSiCorr() {};

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");

//    virtual  Bool_t Identify(KVIdentificationResult *IDR, Double_t x=-1., Double_t y=-1.);

   ClassDef(KVIDChIoSiCorr, 2) //INDRA E503/E494s ChIo-Si identification
};

#endif
