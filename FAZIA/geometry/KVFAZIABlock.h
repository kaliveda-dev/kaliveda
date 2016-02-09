//Created by KVClassFactory on Thu May 30 10:13:17 2013
//Author: Eric Bonnet

#ifndef __KVFAZIABLOCK_H
#define __KVFAZIABLOCK_H

#include "TGeoVolume.h"

class KVFAZIABlock : public TGeoVolumeAssembly {

   Double_t fTotSidWBlind;
public:
   KVFAZIABlock();
   virtual ~KVFAZIABlock();

   Double_t GetTotalSideWithBlindage() const
   {
      // Total length of one side of block including blindage
      return fTotSidWBlind;
   }

   ClassDef(KVFAZIABlock, 1) //Block of 16 telescopes FAZIA
};

#endif
