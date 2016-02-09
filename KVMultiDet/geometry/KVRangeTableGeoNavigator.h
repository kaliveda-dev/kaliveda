//Created by KVClassFactory on Tue Apr 23 10:27:56 2013
//Author: John Frankland,,,

#ifndef __KVRANGETABLEGEONAVIGATOR_H
#define __KVRANGETABLEGEONAVIGATOR_H

#include "KVGeoNavigator.h"
class KVIonRangeTable;

class KVRangeTableGeoNavigator : public KVGeoNavigator {
   KVIonRangeTable* fRangeTable;
   Double_t fCutOffEnergy;//cut-off KE in MeV below which we stop propagation

public:
   KVRangeTableGeoNavigator(TGeoManager*, KVIonRangeTable*);
   virtual ~KVRangeTableGeoNavigator();

   void SetCutOffKEForPropagation(Double_t e)
   {
      fCutOffEnergy = e;
   }

   virtual void ParticleEntersNewVolume(KVNucleus*);
   virtual void PropagateParticle(KVNucleus*, TVector3* TheOrigin = 0);

   ClassDef(KVRangeTableGeoNavigator, 0) //Propagate particles through a geometry and calculate their energy losses
};

#endif
