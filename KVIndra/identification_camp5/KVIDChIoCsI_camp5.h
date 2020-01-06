/*
$Id: KVIDChIoCsI_camp5.h,v 1.2 2009/04/06 15:21:31 franklan Exp $
$Revision: 1.2 $
$Date: 2009/04/06 15:21:31 $
*/

//Created by KVClassFactory on Mon Mar 30 16:44:34 2009
//Author: John Frankland,,,

#ifndef __KVIDCHIOCSI_CAMP5_H
#define __KVIDCHIOCSI_CAMP5_H

#include "KVIDChIoCsI.h"
#include "KVIDZAGrid.h"

class KVIDChIoCsI_camp5 : public KVIDChIoCsI {

   KVIDZAGrid* fGGgrid;//! grid used for ChIo(GG)-CsI(R) charge identification
   KVIDZAGrid* fPGgrid;//! grid used for ChIo(PG)-CsI(R) charge identification
   Double_t fCsIRPedestal;//!CsI Rapide pedestal for current run
   Double_t fCsILPedestal;//!CsI Lente pedestal for current run

public:
   KVIDChIoCsI_camp5();
   virtual ~KVIDChIoCsI_camp5();

   virtual void Initialize(void);
   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");
   Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   ClassDef(KVIDChIoCsI_camp5, 1) //ChIo-CsI id with grids for INDRA_camp5
};

#endif
