//Created by KVClassFactory on Tue Apr 23 13:04:21 2013
//Author: John Frankland,,,

#ifndef __KVGEOIMPORT_H
#define __KVGEOIMPORT_H

#include "KVGeoNavigator.h"
class KVMultiDetArray;
class KVIonRangeTable;
class KVIonRangeTableMaterial;
class KVDetector;
class KVGroup;

class KVGeoImport : public KVGeoNavigator {
   KVMultiDetArray* fArray;
   KVIonRangeTable* fRangeTable;
   KVGroup* fCurrentGroup;
   Int_t fGroupNumber;
   KVDetector* fLastDetector;
   Bool_t fCreateArray;
   TString fDetectorPlugin;

   KVDetector* GetCurrentDetector();
   KVDetector* BuildDetector(TString det_name, TGeoVolume* det_vol);
   void AddLayer(KVDetector*, TGeoVolume*);

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

   ClassDef(KVGeoImport, 0) //Import a ROOT geometry into a KVMultiDetArray object
};

#endif
