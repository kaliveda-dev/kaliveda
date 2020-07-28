#ifndef KVGROUP_H
#define KVGROUP_H

#define KVGROUP_ADD_UNKNOWN_TELESCOPE "Attempt to add undefined telescope to group"

#include "KVGeoStrucElement.h"
#include "KVList.h"
#include "KVGeoDNTrajectory.h"
#include "KVNameValueList.h"
class KVDetector;
class KVNucleus;

/**
 \class KVGroup
 \ingroup Geometry
 \brief Group of detectors which can be treated independently of all others in array
 */

class KVGroup : public KVGeoStrucElement {

protected:
   enum {
      kIsRemoving = BIT(14)     //flag set during call to RemoveTelescope
   };
   KVList* fReconstructedNuclei;        //!Particles reconstructed in this group
   KVHashList fTrajectories;           //! Trajectories passing through group
   KVHashList fReconTraj;              //! list of all possible trajectories for reconstructed particles
   KVNameValueList fReconTrajMap;      //! map names of duplicate trajectories for reconstructed particles

public:
   enum {
      kForwards,
      kBackwards
   };
   KVGroup();
   void init();
   virtual ~ KVGroup();
   virtual void SetNumber(UInt_t num)
   {
      // Setting number for group also sets name to "Group_n"
      SetName(Form("Group_%u", num));
      KVGeoStrucElement::SetNumber(num);
   }
   KVGeoStrucElement* GetArray() const
   {
      // Groups have a single parent structure, the multidetector array they belong to
      return (KVGeoStrucElement*)GetParents()->First();
   }

   virtual UInt_t GetNumberOfDetectorLayers();

   void Reset(Option_t* opt = "");

   virtual TList* GetDetectorsInLayer(UInt_t lay);
   virtual TList* GetAlignedDetectors(KVDetector*, UChar_t dir = kBackwards);

   inline UInt_t GetHits()
   {
      if (fReconstructedNuclei)
         return fReconstructedNuclei->GetSize();
      else
         return 0;
   };
   void ClearHitDetectors();
   KVList* GetParticles()
   {
      return fReconstructedNuclei;
   }
   void AddHit(KVNucleus* kvd);
   void RemoveHit(KVNucleus* kvd);

   Bool_t IsRemoving()
   {
      return TestBit(kIsRemoving);
   }
   virtual void Sort() {}
   virtual void CountLayers() {}
   const TCollection* GetTrajectories() const
   {
      return &fTrajectories;
   }
   void AddTrajectory(KVGeoDNTrajectory* t)
   {
      fTrajectories.Add(t);
   }
   void AddTrajectories(const TCollection* c)
   {
      fTrajectories.AddAll(c);
   }

   Int_t CalculateReconstructionTrajectories();
   const TSeqCollection* GetReconTrajectories() const
   {
      // Get list of all possible trajectories for particle reconstruction in array
      return &fReconTraj;
   }
   const KVGeoDNTrajectory* FindReconTraj(const KVString& path)
   {
      // Return reconstruction trajectory with given path i.e. "DET_1/DET_2/DET_3"
      return (KVGeoDNTrajectory*)fReconTraj.FindObjectByTitle(path);
   }

   const KVGeoDNTrajectory* GetTrajectoryForReconstruction(const KVGeoDNTrajectory* t, const KVGeoDetectorNode* n) const
   {
      if (t && n) {
         TString mapped_name = fReconTrajMap.GetStringValue(Form("%s_%s", t->GetTrajectoryName(), n->GetName()));
         const KVGeoDNTrajectory* tr = (const KVGeoDNTrajectory*)fReconTraj.FindObject(mapped_name);
         return tr;
      }
      return nullptr;
   }
   void PrintData() const;

   ClassDef(KVGroup, 1)//Group of detectors having similar angular positions.
};
#endif
