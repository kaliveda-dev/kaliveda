//Created by KVClassFactory on Fri Apr 26 12:45:15 2013
//Author: John Frankland,,,

#ifndef __KVDETECTORNODE_H
#define __KVDETECTORNODE_H

#include "KVBase.h"
#include "KVSeqCollection.h"

class KVDetector;
class KVGeoDNTrajectory;

/**
  \class KVGeoDetectorNode
  \ingroup Geometry
  \brief Information on relative positions of detectors & particle trajectories

 Each detector of the multidetector geometry has an associated node which can
 be accessed using

~~~~~~~~~~~{.cpp}
    KVDetector* det;                           // pointer to a detector
    KVGeoDetectorNode* node = det->GetNode();  // access detector's node
~~~~~~~~~~~

 Note that the relation is reciprocal, i.e. one can retrieve the detector
 associated to a node:

~~~~~~~~~~~{.cpp}
    KVDetector* d = node->GetDetector();       // in this case, d==det !
~~~~~~~~~~~

 The nodes are set up when the multidetector geometry is imported from a ROOT
 geometry using KVGeoImport. All possible trajectories from the target through
 the detectors of the array are explored. As each detector/node is traversed,
 we store information on the detectors which are immediately in front of or
 behind it, and which trajectories (see KVGeoDNTrajectory class) pass through
 which node. This information can then be obtained from the KVGeoDetectorNode
 associated to each detector.

 ###FORWARDS/BACKWARDS, IN FRONT/BEHIND###
 The convention is that movement towards the target is FORWARDS;
    + a detector D1 is IN FRONT of detector D2 if D1 is closer to the target.

 Conversely, movement away from the target is BACKWARDS;
    + a detector D1 is BEHIND detector D2 if D1 is further from the target.
  */

class KVGeoDetectorNode : public KVBase {
   KVDetector*      fDetector;//!associated detector
   KVSeqCollection* fInFront;//list of detectors in front
   KVSeqCollection* fBehind;//list of detectors behind
   KVSeqCollection* fTraj;//list of trajectories passing through this node
   KVSeqCollection* fTrajF;//list of trajectories passing through this node going forwards
   KVSeqCollection* fTrajB;//list of trajectories passing through this node going backwards
   Int_t fNTraj;//number of trajectories passing through this node
   Int_t fNTrajForwards;//number of trajectories going forwards from this node
   Int_t fNTrajBackwards;//number of trajectories going backwards from this node

   void init();

   void CalculateForwardsTrajectories();
   void CalculateBackwardsTrajectories();

public:
   KVGeoDetectorNode();
   KVGeoDetectorNode(const Char_t* name);
   virtual ~KVGeoDetectorNode();

   void SetDetector(KVDetector*);
   KVDetector* GetDetector() const;
   const Char_t* GetName() const;

   void AddInFront(KVDetector*);
   void AddBehind(KVDetector*);
   Bool_t IsInFrontOf(KVDetector*);
   Bool_t IsBehind(KVDetector*);
   KVSeqCollection* GetDetectorsInFront() const
   {
      return fInFront;
   }
   KVSeqCollection* GetDetectorsBehind() const
   {
      return fBehind;
   }
   KVSeqCollection* GetTrajectories() const
   {
      return fTraj;
   }
   KVSeqCollection* GetForwardTrajectories() const;
   KVSeqCollection* GetBackwardTrajectories() const;
   Int_t GetNDetsInFront() const;
   Int_t GetNDetsBehind() const;
   Int_t GetNTraj() const;
   Int_t GetNTrajForwards() const;
   Int_t GetNTrajBackwards() const;

   void BuildTrajectoriesForwards(TSeqCollection*);
   void AddTrajectory(KVGeoDNTrajectory*);

   void RehashLists();

   KVGeoDNTrajectory* FindTrajectory(const char* title) const;
   KVGeoDNTrajectory* FindTrajectory(UInt_t number) const;

   const Char_t* GetFullPathToNode() const
   {
      // Return full path to this node in ROOT geometry, i.e. "/TOP/A_1/B_2/MY_DETECTOR"
      // This information is set by KVGeoImport on creation
      return GetTitle();
   }

   void ls(Option_t* option = "") const;

   ClassDef(KVGeoDetectorNode, 2) //Information on relative positions of detectors & particle trajectories
};

#endif
