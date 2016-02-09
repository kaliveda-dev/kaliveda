//Created by KVClassFactory on Mon Apr 22 14:53:13 2013
//Author: John Frankland,,,

#ifndef __KVGEONAVIGATOR_H
#define __KVGEONAVIGATOR_H

#include "KVBase.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include <KVNameValueList.h>
#include <TGeoMatrix.h>
class KVNucleus;
class KVEvent;
class TGeoManager;
class TGeoVolume;
class TGeoNode;
class TEnv;

class KVGeoNavigator : public KVBase {
private:
   TGeoManager* fGeometry;//geometry to navigate
   TGeoVolume* fCurrentVolume;//current volume
   TGeoNode* fCurrentNode;//current node
   TGeoNode* fCurrentDetectorNode;//node for current detector
   TGeoHMatrix fCurrentMatrix;//current global transformation matrix
   TString fCurrentPath;//current full path to physical node
   TClonesArray fCurrentStructures;//list of current structures deduced from path
   Int_t fCurStrucNumber;//number of current parent structures
   TGeoNode* fMotherNode;//mother node of current node
   Double_t fStepSize;//distance to travel in volume
   TVector3 fEntryPoint;//position of particle on entering volume
   TVector3 fExitPoint;//position of particle on exiting volume
   Bool_t fStopPropagation;//flag set by user when particle propagation should stop
protected:
   KVNameValueList fStrucNameFmt;//list of user-defined formats for structure names
   KVString fDetNameFmt;//user-defined format for detector names
   TEnv* fDetStrucNameCorrespList;//list(s) of correspondance for renaming structures/detectors
   void FormatStructureName(const Char_t* type, Int_t number, KVString& name);
   void FormatDetectorName(const Char_t* basename, KVString& name);

public:
   KVGeoNavigator(TGeoManager*);
   virtual ~KVGeoNavigator();

   void SetStructureNameFormat(const Char_t* type, const Char_t* fmt);
   void SetDetectorNameFormat(const Char_t* fmt)
   {
      fDetNameFmt = fmt;
   }
   const Char_t* GetDetectorNameFormat() const
   {
      return fDetNameFmt.Data();
   }
   void SetNameCorrespondanceList(const Char_t*);
   void SetNameCorrespondanceList(const TEnv*);
   Bool_t GetNameCorrespondance(const Char_t*, TString&);

   void PropagateEvent(KVEvent*, TVector3* TheOrigin = 0);
   virtual void PropagateParticle(KVNucleus*, TVector3* TheOrigin = 0);
   virtual void ParticleEntersNewVolume(KVNucleus*);

   TGeoManager* GetGeometry() const
   {
      return fGeometry;
   }
   TGeoVolume* GetCurrentVolume() const
   {
      return fCurrentVolume;
   }
   TGeoNode* GetCurrentNode() const
   {
      return fCurrentNode;
   }
   const TGeoHMatrix* GetCurrentMatrix() const;
   Double_t GetStepSize() const
   {
      return fStepSize;
   }
   const TVector3& GetEntryPoint() const
   {
      return fEntryPoint;
   }
   const TVector3& GetExitPoint() const
   {
      return fExitPoint;
   }
   TGeoVolume* GetCurrentDetectorNameAndVolume(KVString&, Bool_t&);
   TGeoNode* GetCurrentDetectorNode() const;
   TString GetCurrentPath() const
   {
      return fCurrentPath;
   }

   Bool_t StopPropagation() const
   {
      return fStopPropagation;
   }
   void SetStopPropagation(Bool_t stop = kTRUE)
   {
      fStopPropagation = stop;
   }

   void ExtractDetectorNameFromPath(KVString&);
   const TClonesArray& CurrentStructures() const
   {
      return fCurrentStructures;
   }

   ClassDef(KVGeoNavigator, 0) //Propagate particles of an event through a TGeo geometry
};

#endif
