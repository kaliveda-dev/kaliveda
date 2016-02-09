//Created by KVClassFactory on Fri Apr 26 14:01:32 2013
//Author: John Frankland,,,

#ifndef __KVASGROUP_H
#define __KVASGROUP_H

#include "KVGroup.h"
#include "KVTelescope.h"

class KVASGroup : public KVGroup, public KVPosition {

protected:
   UInt_t fNumberOfLayers;      //number of different layers in group
   UInt_t fLayNumMin;           //minimum layer number (nearest to target)
   UInt_t fLayNumMax;           //maximum layer number (furthest from target)
public:
   KVASGroup();
   void init();
   virtual ~KVASGroup();
   TList* GetTelescopesWithAngles(Float_t theta, Float_t phi) const;
   KVNameValueList* DetectParticle(KVNucleus* part);
   void SetDimensions(KVPosition*, KVPosition*);
   void SetDimensions();
   UInt_t GetNumberOfLayers()
   {
      if (!fNumberOfLayers)
         CountLayers();
      return fNumberOfLayers;
   };
   void Sort();
   void CountLayers();
   UInt_t GetLayerNearestTarget() const;
   UInt_t GetLayerFurthestTarget() const;
   TList* GetTelescopesInLayer(UInt_t nlayer);
   UInt_t GetNumberOfDetectorLayers();
   TList* GetDetectorsInLayer(UInt_t lay);
   UInt_t GetDetectorLayer(KVDetector* det);
   TList* GetAlignedDetectors(KVDetector*, UChar_t dir = kBackwards);

   void Add(KVBase*);
   Bool_t Contains(KVBase* name) const;
   const KVSeqCollection* GetTelescopes() const
   {
      return GetStructures();
   }

   ClassDef(KVASGroup, 1) //Group in axially-symmetric multidetector
};

#endif
