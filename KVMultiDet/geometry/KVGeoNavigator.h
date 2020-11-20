//Created by KVClassFactory on Mon Apr 22 14:53:13 2013
//Author: John Frankland,,,

#ifndef __KVGEONAVIGATOR_H
#define __KVGEONAVIGATOR_H

#include "KVBase.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "KVDetector.h"
#include <KVNameValueList.h>
#include <TGeoMatrix.h>
class KVNucleus;
class TGeoManager;
class TGeoVolume;
class TGeoNode;
class TEnv;

/**
 \class KVGeoNavigator
 \ingroup Geometry
 \brief Base class for propagation of particles through array geometry

 This is a base class for propagation of charged particles (KVNucleus) in events (KVEvent)
 through any TGeoManager ROOT geometry. Classes derived from this one must
 override the method
~~~~~~~{.cpp}
     ParticleEntersNewVolume(KVNucleus*)
~~~~~~~
 in order to do something useful every time that a particle of the event
 enters a new volume (absorber, detector, etc.) of the geometry.

 Then to use your derived class do something like:

~~~~~{.cpp}
  MyGeoNavigator nav( gGeoManager );
  while( nevents-- ) {
       nav.PropagateEvent( event );
  }
~~~~~

 \sa KVRangeTableGeoNavigator, KVGeoImport
 */

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
   Int_t fTrackID;//! track counter
   Bool_t fTracking;//! set to true when tracking particles
protected:
   KVNameValueList fStrucNameFmt;//list of user-defined formats for structure names
   KVString fDetNameFmt;//user-defined format for detector names
   TEnv* fDetStrucNameCorrespList;//list(s) of correspondance for renaming structures/detectors
   void FormatStructureName(const Char_t* type, Int_t number, KVString& name);
   void FormatDetectorName(const Char_t* basename, KVString& name);

public:
   /** \class KVGeoDetectorPath
      \brief Link physical path to node in geometry with detector
      */
   class KVGeoDetectorPath : public TNamed {
      KVDetector* fDetector;
   public:
      KVGeoDetectorPath() : TNamed(), fDetector(nullptr) {}
      KVGeoDetectorPath(const Char_t* path, KVDetector* det) :
         TNamed(path, ""), fDetector(det) {}
      virtual ~KVGeoDetectorPath() {}
      KVDetector* GetDetector() const
      {
         return fDetector;
      }
      ClassDef(KVGeoDetectorPath, 1) //Link physical path to node in geometry with detector
   };
protected:
   KVUniqueNameList fDetectorPaths;//correspondance between physical node and detector objects
   KVDetector* GetDetectorFromPath(const Char_t* p)
   {
      // Fast look-up of detector from full path to physical node
      // This can only be used AFTER a KVGeoImport of the geometry
      KVGeoDetectorPath* gdp = (KVGeoDetectorPath*)fDetectorPaths.FindObject(p);
      return (KVDetector*)(gdp ? gdp->GetDetector() : nullptr);
   }

public:
   KVGeoNavigator(TGeoManager*);
   virtual ~KVGeoNavigator();

   void ResetTrackID(Int_t id = 0)
   {
      fTrackID = id;
   }
   Int_t GetTrackID() const
   {
      return fTrackID;
   }
   void IncrementTrackID()
   {
      ++fTrackID;
   }
   void SetTracking(Bool_t on = kTRUE)
   {
      fTracking = on;
   }
   Bool_t IsTracking() const
   {
      return fTracking;
   }

   void SetStructureNameFormat(const Char_t* type, const Char_t* fmt);
   void SetDetectorNameFormat(const Char_t* fmt)
   {
      // The default base names for detectors are taken from the node name by stripping off
      // the **DET_** prefix. In order to ensure that all detectors have unique names,
      // by default we prefix the names of the parent structures to the basename in
      // order to generate the full name of the detector:
      //~~~~~~~
      //    [struc1-name]_[struc2-name]_..._[detector-basename]
      //~~~~~~~
      // However, this format can be changed by calling method
      //~~~~~{.cpp}
      //    SetDetectorNameFormat("[format]")
      //~~~~~
      // where format can contain any of the following tokens:
      //~~~~~~
      //    $det:name$             - will be replaced by the detector basename
      //    $struc:[type]:name$    - will be replaced by the name of the parent structure of given type
      //    $struc:[type]:type$    - will be replaced by the type of the parent structure of given type
      //    $struc:[type]:number$  - will be replaced by the number of the parent structure of given type
      //~~~~~~
      // plus additional formatting information as for SetStructureNameFormat()
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
   virtual void AddPointToCurrentTrack(Double_t, Double_t, Double_t) {}
   void DrawTracks(KVNumberList* = nullptr);

   void AbsorbDetectorPaths(KVGeoNavigator* GN)
   {
      // Add all contents of GN->fDetectorPaths to our list.
      //
      // Remove ownership of these paths from GN - our dtor will delete them

      fDetectorPaths.AddAll(&GN->fDetectorPaths);
      GN->fDetectorPaths.SetOwner(kFALSE);
   }
   void PrintDetectorPaths()
   {
      TIter it(&fDetectorPaths);
      KVGeoDetectorPath* gdp;
      while ((gdp = (KVGeoDetectorPath*)it())) {
         std::cout << gdp->GetDetector()->GetName() << " : " << gdp->GetName() << std::endl;
      }
   }

   ClassDef(KVGeoNavigator, 0) //Propagate particles of an event through a TGeo geometry
};

#endif
