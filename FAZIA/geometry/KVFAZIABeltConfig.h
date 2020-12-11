//Created by KVClassFactory on Wed Mar 18 15:02:46 2020
//Author: John Frankland,,,

#ifndef __KVFAZIABELTCONFIG3X2_H
#define __KVFAZIABELTCONFIG3X2_H

#include "TGeoVolume.h"
/**
  \class KVFAZIABeltConfig
  \ingroup FAZIAGeo
  \brief A \f$N x M\fM rectangular array of FAZIA blocks
 */
class KVFAZIABeltConfig : public TGeoVolumeAssembly {

   Double_t fDist;  // distance of centre of belt from target
   Double_t fInterBlock; //spacing between blocks

   void BuildBelt(int nblocsX, int nblocsY, Double_t distance_correction);

public:
   KVFAZIABeltConfig(int nblocsX, int nblocsY, Double_t dist = 100.0, Double_t inter_block = 0.5, double distance_correction = 5)
      : TGeoVolumeAssembly("STRUCT_BELT"),
        fDist(dist), fInterBlock(inter_block)
   {
      // Create a belt at the given position
      BuildBelt(nblocsX, nblocsY, distance_correction);
   }
   virtual ~KVFAZIABeltConfig() {}

   ClassDef(KVFAZIABeltConfig, 1) //blocks of FAZIA in a belt of NxM blocks
};

#endif
