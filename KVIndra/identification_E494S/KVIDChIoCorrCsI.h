/*
$Id: KVIDChIoCorrCsI.h,v 1.2 2009/04/06 15:21:31 franklan Exp $
$Revision: 1.2 $
$Date: 2009/04/06 15:21:31 $
*/

//Created by KVClassFactory on Mon Mar 30 16:44:34 2009
//Author: John Frankland,,,

#ifndef __KVIDCHIOCORRCSI_H
#define __KVIDCHIOCORRCSI_H

#include "KVIDZAGrid.h"
#include "KVChIo.h"
#include "KVIDChIoCsI.h"
#include "KVCsI.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVIDChIoCorrCsI : public KVIDChIoCsI {
protected:

   KVIDZAGrid* fGrid;
   KVChIo*     fChIo;
   KVCsI*      fCsI;

public:

   KVIDChIoCorrCsI();
   virtual ~KVIDChIoCorrCsI();

   virtual void Initialize(void);

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");

   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   ClassDef(KVIDChIoCorrCsI, 2) //ChIo-CsI identification with grids for E503
};

#endif
