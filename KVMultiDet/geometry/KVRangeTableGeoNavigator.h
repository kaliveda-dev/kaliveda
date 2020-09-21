#ifndef __KVRANGETABLEGEONAVIGATOR_H
#define __KVRANGETABLEGEONAVIGATOR_H

#include "TVirtualGeoTrack.h"
#include "KVGeoNavigator.h"
#include "KVIonRangeTable.h"

/**
 \class KVRangeTableGeoNavigator
 \ingroup Geometry
 \brief Propagate particles through array geometry calculating energy losses

 Given a valid ROOT geometry, we propagate the particles of an event
 and, every time a particle traverses a volume made of a TGeoMaterial
 with a name corresponding to a material known by this range table,
 we calculate the energy loss of the particle,
 and store it in the particle's list of parameters in the form
~~~~~~~~
   "DE:[detector name]" = [energy lost in volume]
~~~~~~~~

 In the case of multilayer detectors, `[detector name]` is replaced by `[detector name]/[layer name]`.

 #### Example of use ####

 In this case the geometry is made up of 3-layer Ionisation Chamber detectors
 (window + gas + window), with an **ACTIVE** gas layer, and single-layer
 Silicon detectors placed behind the IC.

~~~~{.cpp}
 KVEvent* evt = new KVEvent();
 KVNucleus* nuc = evt->AddParticle();
 Double_t theta = gRandom->Uniform(0,30);
 Double_t phi = gRandom->Uniform(0,360);
 nuc->SetZAandE(1,1,200); nuc->SetTheta(theta); nuc->SetPhi(phi);

 KVRangeTableGeoNavigator rtgn(gGeoManager, KVMaterial::GetRangeTable())
 rtgn.PropagateEvent(evt);
 evt->Print();

 KVEvent with 1 particles :
 ------------------------------------
 KVNameValueList::EventParameters : Parameters associated with an event (0x3739f10)
 KVNucleus Z=1 A=1 E*=0
 KVParticle mass=938.783 Theta=14.017 Phi=44.7553 KE=199.73
 KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x37b3510)
 <DE:CHIO_3/WINDOW_1=0.00151169>
 <DE:CHIO_3/ACTIVE_GAS_1=0.00123736>
 <DE:CHIO_3/WINDOW_2=0.00151169>
 <DE:SI_1_5=0.265778>
~~~~
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
