//Created by KVClassFactory on Mon Jul  8 15:40:35 2013
//Author: Guilain ADEMARD

#ifndef __KVVAMOSRECONGEONAVIGATOR_H
#define __KVVAMOSRECONGEONAVIGATOR_H

#include "KVGeoNavigator.h"
#include "KVIonRangeTable.h"

class KVVAMOSReconNuc;
class KVIonRangeTable;

enum ECalib {
   kNoCalib    = 0,                         // no calibration
   kECalib     = BIT(0),                    // E calib. for active absorbers
   kFullECalib = kECalib | BIT(1),          // E calib. for all absorbers
   kTCalib     = BIT(2),                    // T calib. for active absorbers
   kFullTCalib = kTCalib | BIT(3),          // T calib. for all absorbers
   kETCalib    = kECalib | kTCalib,         // T and E calib. for active absorbers
   kFullCalib  = kFullECalib | kFullTCalib  // T and E calib for all absorbers
};


class KVVAMOSReconGeoNavigator : public KVGeoNavigator {

protected:

   Bool_t fDoNothing;// true if nothing has to be done when nucleus enters in new volume
   ECalib fCalib; // option to select infos to be stored
   TVector3 fOrigine;// Origine of the initial propagation i.e. coord. intersection point of the trajectory and the focal plane
   TVector3 fFOrigine;// Origine of the forward propagation i.e. coord. of a point before the first detector at the focal plane
   Double_t fE; // Energy of the propagated nucleus
   Double_t fStartPath; // path of the current start point from the Focal Plane reference frame
   Double_t fTOF; // total time of flight
   KVIonRangeTable* fRangeTable;

   Double_t CalculateLinearDeltaT(Double_t vi, Double_t vf, Double_t step) const;

public:

   KVVAMOSReconGeoNavigator(TGeoManager*, KVIonRangeTable*);
   virtual ~KVVAMOSReconGeoNavigator();

   void ParticleEntersNewVolume(KVNucleus* nuc);

   void PropagateNucleus(KVVAMOSReconNuc*, ECalib cal = kNoCalib);

   ClassDef(KVVAMOSReconGeoNavigator, 1) //Propagate nuclei through the geometry of VAMOS for their reconstruction/calibrate/identification
};

#endif
