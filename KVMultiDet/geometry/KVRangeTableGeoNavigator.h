#ifndef __KVRANGETABLEGEONAVIGATOR_H
#define __KVRANGETABLEGEONAVIGATOR_H

#include "TVirtualGeoTrack.h"
#include "KVGeoNavigator.h"
#include "KVIonRangeTable.h"

/**
 \class KVRangeTableGeoNavigator
 \ingroup Geometry
 \brief Propagate particles through array geometry calculating energy losses
 */

class KVRangeTableGeoNavigator : public KVGeoNavigator {

   KVIonRangeTable* fRangeTable;
   Double_t fCutOffEnergy;//cut-off KE in MeV below which we stop propagation
   TVirtualGeoTrack* fCurrentTrack;//! current track of nucleus being propagated
   Double_t fTrackTime;//! track "clock"

   void InitialiseTrack(KVNucleus* part, TVector3* TheOrigin);
   void AddPointToCurrentTrack(Double_t x, Double_t y, Double_t z)
   {
      // Add point to current track & increase track time counter by 1 unit
      fCurrentTrack->AddPoint(x, y, z, fTrackTime);
      fTrackTime += 1;
   }

public:
   KVRangeTableGeoNavigator(TGeoManager* g, KVIonRangeTable* r)
      : KVGeoNavigator(g), fRangeTable(r), fCutOffEnergy(1.e-3), fCurrentTrack(nullptr),
        fTrackTime(0.)
   {}
   virtual ~KVRangeTableGeoNavigator() {}
   void SetCutOffKEForPropagation(Double_t e)
   {
      fCutOffEnergy = e;
   }
   Double_t GetCutOffKEForPropagation() const
   {
      return fCutOffEnergy;
   }

   virtual void ParticleEntersNewVolume(KVNucleus*);
   virtual void PropagateParticle(KVNucleus*, TVector3* TheOrigin = 0);

   Bool_t CheckIonForRangeTable(Int_t Z, Int_t A)
   {
      // Return kTRUE if current range tables can handle ion
      return fRangeTable->CheckIon(Z, A);
   }

   ClassDef(KVRangeTableGeoNavigator, 0) //Propagate particles through a geometry and calculate their energy losses
};

#endif
