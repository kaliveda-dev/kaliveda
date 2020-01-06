/*
$Id: KVIDCsI_e475s.h,v 1.2 2008/07/08 12:41:56 ebonnet Exp $
$Revision: 1.2 $
$Date: 2008/07/08 12:41:56 $
*/

//Created by KVClassFactory on Fri Jul  4 16:36:34 2008
//Author: Eric Bonnet

#ifndef __KVIDCSI_E475S_H
#define __KVIDCSI_E475S_H

#include "KVIDINDRACsI.h"

class KVIDCsI_e475s : public KVIDINDRACsI {

public:
   KVIDCsI_e475s();
   virtual ~KVIDCsI_e475s();

   void CalculateParticleEnergy(KVReconstructedNucleus* nuc);

   ClassDef(KVIDCsI_e475s, 1) //derivation of KVIDCsI class for E475s experiment
};

#endif
