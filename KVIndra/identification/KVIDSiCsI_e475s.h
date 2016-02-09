/*
$Id: KVIDSiCsI_e475s.h,v 1.1 2009/04/15 09:44:19 ebonnet Exp $
$Revision: 1.1 $
$Date: 2009/04/15 09:44:19 $
*/

//Created by KVClassFactory on Wed Apr 15 11:41:21 2009
//Author: eric bonnet,,,

#ifndef __KVIDSICSI_E475S_H
#define __KVIDSICSI_E475S_H

#include "KVIDSiCsI.h"
#include "KVIDZAGrid.h"
#include "KVIDGraph.h"

class KVReconstructedNucleus;

class KVIDSiCsI_e475s : public KVIDSiCsI {

private:
   KVIDZAGrid* fidgrid;//!

public:
   KVIDSiCsI_e475s();
   virtual ~KVIDSiCsI_e475s();

   virtual void Initialize(void);

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");
   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual void CalculateParticleEnergy(KVReconstructedNucleus* nuc);

   ClassDef(KVIDSiCsI_e475s, 1) //derivation of KVIDSiCsI class for E475s experiment
};

#endif
