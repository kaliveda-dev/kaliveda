//Created by KVClassFactory on Tue Apr 23 13:04:21 2013
//Author: John Frankland,,,

#ifndef __KVGEOIMPORT_H
#define __KVGEOIMPORT_H

#include "KVGeoNavigator.h"
#include "KVGeoDNTrajectory.h"
class KVMultiDetArray;
class KVIonRangeTable;
class KVIonRangeTableMaterial;
class KVDetector;
class KVGroup;

/**
  \class KVGeoImport
  \ingroup Geometry
  \brief Import detector array described by ROOT geometry and set up corresponding KVMultiDetArray object

 To use:
~~~~~~~~{.cpp}
 KVGeoImport geo(gGeoManager, KVMaterial::GetRangeTable(), new KVMultiDetArray);
 gImp.ImportGeometry();
~~~~~~~~

 ### RULES FOR VALID DETECTOR GEOMETRY ###

 1.) All detector volumes & nodes must have names which begin with **DET_**

 2.) They must be made of materials which are known by the range table fRangeTable.

 3.) For multi-layer detectors, the *active* layer volume/node must have a name beginning with **ACTIVE_**

 4.) The "thickness" of the detector or any layer inside a multilayer detector
     will be taken as the size of the volume's shape along its Z-axis
     (so make sure that you define your detector volumes in this way).

 5.) It is assumed that the natural length units of the geometry are centimetres.

 6.) The name of the KVDetector object created and added to the array will be taken
     from the unique full path of the node corresponding to the geometrical positioning
     of the detector, see KVGeoNavigator::ExtractDetectorNameFromPath()

  ### STRUCTURE ELEMENTS ###
 A structure element  is any association of detectors which may occur several times
 in a geometry in different places, with the same internal structure.

 #### Example: a wall of detectors in a rectangular array. ####

 The detectors in the wall may all be represented by a single volume (if they
 are identical) called **DET_SI**, for example, which is placed *N* times in *N*
 different positions on nodes called, by default, **DET_SI_1**, **DET_SI_2**, etc.

 Once you have defined the wall volume, call it **STRUCT_SIWALL** (or whatever you
 want: only the **STRUCT_** part is mandatory).
 Now you can place the same wall structure in different positions of your
 geometry on nodes called **STRUCT_SIWALL_1**, **STRUCT_SIWALL_2**, etc.

 This will give a full path to the node of **DET_SI_1** in **SIWALL_2** like this:
~~~~~~
    /TOP_1/STRUCT_SIWALL_2/DET_SI_1
~~~~~~
 which will be interpreted by KVGeoNavigator::ExtractDetectorNameFromPath() as a
 detector with unique name: **SIWALL_2_SI_1** (the **STRUCT_** and **DET_** parts are stripped off).

 */
class KVGeoImport : public KVGeoNavigator {
   KVMultiDetArray* fArray;
   KVIonRangeTable* fRangeTable;
   KVDetector* fLastDetector;
   Bool_t fCreateArray;
   TString fDetectorPlugin;
   KVGeoDNTrajectory fCurrentTrajectory;
   TVector3* fOrigin;

   KVDetector* GetCurrentDetector();
   KVDetector* BuildDetector(TString det_name, TGeoVolume* det_vol);
   void AddLayer(KVDetector*, TGeoVolume*);
   KVNameValueList fAcceptedDetectorNames;
   Bool_t fCheckDetVolNames;

public:
   KVGeoImport(TGeoManager*, KVIonRangeTable*, KVMultiDetArray*, Bool_t create = kTRUE);
   virtual ~KVGeoImport();

   virtual void ParticleEntersNewVolume(KVNucleus*);

   void ImportGeometry(Double_t dTheta = 0.1/*degrees*/, Double_t dPhi = 1.0/*degrees*/,
                       Double_t ThetaMin = 0.0/*degrees*/, Double_t PhiMin = 0.0/*degrees*/,
                       Double_t ThetaMax = 180.0/*degrees*/, Double_t PhiMax = 360.0/*degrees*/);

   void SetLastDetector(KVDetector*);

   void SetDetectorPlugin(const TString& name)
   {
      fDetectorPlugin = name;
   }
   void PropagateParticle(KVNucleus*, TVector3* TheOrigin = nullptr);

   void SetOrigin(double x, double y, double z)
   {
      // Call this method if needed before calling ImportGeometry in order to shoot particles
      // from an arbitrary point (x,y,z) instead of (0,0,0).
      // This can be useful if the detector array is "misaligned" so that its telescopes are
      // not pointing towards the theoretical origin (i.e. target position).
      fOrigin = new TVector3(x, y, z);
   }

   void AddAcceptedDetectorName(const char* name);

   ClassDef(KVGeoImport, 0) //Import a ROOT geometry into a KVMultiDetArray object
};

#endif
