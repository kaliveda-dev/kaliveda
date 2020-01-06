/*
$Id: KVIDChIoCsI_e475s.h,v 1.1 2009/04/17 08:50:02 ebonnet Exp $
$Revision: 1.1 $
$Date: 2009/04/17 08:50:02 $
*/

//Created by KVClassFactory on Thu Apr  9 14:59:44 2009
//Author: eric bonnet,,,

#ifndef __KVIDCHIOCSI_E475S_H
#define __KVIDCHIOCSI_E475S_H

#include "KVIDChIoCsI.h"
#include "KVIDZAGrid.h"

class KVReconstructedNucleus;

class KVIDChIoCsI_e475s : public KVIDChIoCsI {
private:
   KVIDZAGrid* fidgrid;//!

public:
   KVIDChIoCsI_e475s();
   virtual ~KVIDChIoCsI_e475s();

   virtual void Initialize(void);

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");
   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual void CalculateParticleEnergy(KVReconstructedNucleus* nuc);

   ClassDef(KVIDChIoCsI_e475s, 1) //derivation of KVIDChIoCsI class for E475s experiment
};

#endif
