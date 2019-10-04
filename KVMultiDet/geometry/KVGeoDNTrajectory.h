//Created by KVClassFactory on Tue Feb 25 16:31:36 2014
//Author: John Frankland,,,

#ifndef __KVGEODNTRAJECTORY_H
#define __KVGEODNTRAJECTORY_H

#include "KVBase.h"
#include "KVUniqueNameList.h"
#include "TObjArray.h"
#include "KVGeoDetectorNode.h"

/**
 \class KVGeoDNTrajectory
 \ingroup Geometry
 \brief Path taken by particles through multidetector geometry

 A KVGeoDNTrajectory represents one possible trajectory that particles produced in
 an event may take to travel from the target through the detectors of the multidetector
 array. Each trajectory is made up of a series of KVGeoDetectorNode objects which
 are each associated with a detector in the geometry. As the trajectories are mostly
 used in particle reconstruction & identification, each trajectory starts from the last
 detector in a stack and moves towards the target.

 ###NAME, NUMBER & TITLE###

 `GetName()`
     - The name of each trajectory is `DET1/DET2/DET3/...` made up from the names of the detectors/nodes on the trajectory.

 `GetNumber()`
     - The unique number of the trajectory.

 `GetTitle()`
     - A unique name, made up of the prefix `GDNTraj_` followed by the trajectory's unique number.

 ###ITERATE OVER TRAJECTORY###

 In order to visit each detector/node of the trajectory in order, use the iterators:

 ~~~~~~~~~~~{.cpp}
    KVGeoDNTrajectory* trajectory; // pointer to some valid trajectory

    trajectory->IterateFrom();     // default: start from first node of trajectory
                                   // [i.e. the detector furthest from the target]

    KVGeoDetectorNode* node;
    while( (node = trajectory->GetNextNode()) ){

       KVDetector* detector = node->GetDetector();   // detector associated to node

         ...your code here...

    }
 ~~~~~~~~~~~

 You can also start from any node on the trajectory:

 ~~~~~~~~~~~{.cpp}
    KVGeoDetectorNode* node_on_traj = trajectory->GetNodeAt(1); // start from second node

    trajectory->IterateFrom(node_on_traj);

    while( (node = trajectory->GetNextNode()) ){

         ...your code here...

    }
 ~~~~~~~~~~~

 To iterate in the other direction i.e. away from the target:

 ~~~~~~~~~~~{.cpp}
    trajectory->IterateBackFrom(); // default: start from last node of trajectory
                                   // [i.e. the detector closest to the target]

    trajectory->IterateBackFrom(node_on_traj);
 ~~~~~~~~~~~
*/

class KVGeoDNTrajectory : public KVBase {

   friend class KVGroup;

   static KVGeoDNTrajectory* Factory(const char* plugin, const KVGeoDNTrajectory*, const KVGeoDetectorNode*);

   void init();

   TObjArray fNodes;//! list of nodes on trajectory
   static Int_t fGDNTrajNumber;
   mutable Int_t fIter_idx;//! index for iteration
   mutable Int_t fIter_limit;//! last index for iteration
   mutable Int_t fIter_delta;//! increment/decrement for each iteration
   mutable Int_t fIter_idx_sav;//! index for iteration
   mutable Int_t fIter_limit_sav;//! last index for iteration
   mutable Int_t fIter_delta_sav;//! increment/decrement for each iteration
   KVUniqueNameList fIDTelescopes;//! list of id telescopes on this trajectory
   Bool_t fPathInTitle;// true if path is in title, false if path is in name

   void rebuild_title();

   void increment_identified_particle_counters(int identified, int unidentified) const;

protected:
   Bool_t fAddToNodes;// if kTRUE, add trajectory to node's list

public:
   KVGeoDNTrajectory();
   KVGeoDNTrajectory(KVGeoDetectorNode*);
   KVGeoDNTrajectory(const KVGeoDNTrajectory&);
   virtual ~KVGeoDNTrajectory();
   void Copy(TObject& obj) const;
   KVGeoDNTrajectory& operator=(const KVGeoDNTrajectory&);

   void Clear(Option_t* = "");

   void SetAddToNodes(Bool_t yes = kTRUE)
   {
      fAddToNodes = yes;
   }

   KVGeoDetectorNode* GetNode(const Char_t* name) const
   {
      // return pointer to node with given name in this trajectory
      return (KVGeoDetectorNode*)fNodes.FindObject(name);
   }
   KVGeoDetectorNode* GetNodeInFront(const KVGeoDetectorNode* n) const
   {
      // return pointer to node immediately in front of 'n' on trajectory
      if (Contains(n) && !EndsAt(n)) {
         return GetNodeAt(Index(n) + 1);
      }
      return nullptr;
   }
   Bool_t Contains(const Char_t* name) const
   {
      // returns kTRUE if trajectory contains the given node/detector
      // (nodes have the same name as the detector they are associated to)
      return GetNode(name) != nullptr;
   }

   Int_t GetN() const
   {
      // return number of nodes on this trajectory
      return fNodes.GetEntries();
   }
   Int_t Index(const TObject* node) const
   {
      // return index of node (=0,1,...)
      return fNodes.IndexOf(node);
   }

   void AddLast(KVGeoDetectorNode* n)
   {
      // add node to end of trajectory
      fNodes.AddLast(n);
      if (fAddToNodes) n->AddTrajectory(this);
      rebuild_title();
   }

   void ReverseOrder();
   void AddToNodes();

   void ls(Option_t* = "") const
   {
      std::cout << GetTrajectoryName() << " : " << GetPathString() << std::endl;
   }

   Bool_t EndsAt(const Char_t* node_name) const
   {
      // Return kTRUE if node_name is last node of trajectory
      // (i.e. closest to target)

      return !strcmp(node_name, fNodes.Last()->GetName());
   }

   Bool_t BeginsAt(const Char_t* node_name) const
   {
      // Return kTRUE if node_name is first node of trajectory
      // (i.e. furthest from target)

      return !strcmp(node_name, fNodes.First()->GetName());
   }

   Bool_t EndsAt(const KVGeoDetectorNode* d) const
   {
      // Return kTRUE if n is last node of trajectory
      // (i.e. closest to target)

      return (d == (KVGeoDetectorNode*)fNodes.Last());
   }

   Bool_t BeginsAt(const KVGeoDetectorNode* d) const
   {
      // Return kTRUE if node_name is first node of trajectory
      // (i.e. furthest from target)

      return (d == (KVGeoDetectorNode*)fNodes.First());
   }

   Bool_t Contains(const KVGeoDetectorNode* n) const
   {
      return (fNodes.FindObject(n) != NULL);
   }
   Bool_t ContainsAll(const TCollection* l) const
   {
      // Returns kTRUE if trajectory contains all detectors/nodes in the list
      // N.B. we only check the names of the (TObject-derived) objects in the list
      TIter next(l);
      TObject* o;
      while ((o = next())) {
         if (!Contains(o->GetName())) return kFALSE;
      }
      return kTRUE;
   }

   KVGeoDetectorNode* GetNodeAt(Int_t i) const
   {
      // Return i-th node in trajectory
      //  i=0 -> first node, furthest from target
      //  i=GetN()-1 -> last node, closest to target

      return (KVGeoDetectorNode*)fNodes[i];
   }

   void SaveIterationState() const
   {
      // In order to perform an iteration while another is already in
      // progress, we store the current iteration.
      // Call RestoreIterationState() in order to continue.
      fIter_idx_sav = fIter_idx;
      fIter_delta_sav = fIter_delta;
      fIter_limit_sav = fIter_limit;
   }
   void RestoreIterationState() const
   {
      // See SaveIterationState().

      fIter_idx = fIter_idx_sav;
      fIter_delta = fIter_delta_sav;
      fIter_limit = fIter_limit_sav;
   }
   void IterateFrom(const KVGeoDetectorNode* node0 = nullptr) const
   {
      // FORWARD ITERATION: moving towards the target
      // Start an iteration over the trajectory nodes, starting from node0.
      // If node0=nullptr (default value), start from the first node.
      // After calling this method, calling method GetNextNode()
      // will return each node of the trajectory starting with node0
      // to the last one, after which it returns 0x0

      fIter_idx = (node0 == nullptr ? 0 : Index(node0));
      fIter_delta = 1;
      fIter_limit = GetN() - 1;
   }
   void IterateBackFrom(const KVGeoDetectorNode* node0 = nullptr) const
   {
      // BACKWARD ITERATION: moving away from the target
      // Start an iteration over the trajectory nodes, starting from node0.
      // If node0=nullptr (default value), start from the last node.
      // After calling this method, calling method GetNextNode()
      // will return each node of the trajectory starting with node0
      // to the last one, after which it returns 0x0

      fIter_idx = (node0 == nullptr ? GetN() - 1 : Index(node0));
      fIter_delta = -1;
      fIter_limit = 0;
   }

   KVGeoDetectorNode* GetNextNode() const
   {
      // Get next node in iteration over trajectory.
      // See IterateFrom(KVGeoDetectorNode*) and IterateBackFrom(KVGeoDetectorNode*)

      if (fIter_idx > -1) {
         if (fIter_delta * (fIter_limit - fIter_idx) > 0) {
            Int_t idx = fIter_idx;
            fIter_idx += fIter_delta;
            return GetNodeAt(idx);
         }
         else {
            // last node
            fIter_idx = -1;
            return GetNodeAt(fIter_limit);
         }
      }
      return nullptr;
   }

   const KVSeqCollection* GetIDTelescopes() const
   {
      // List of identification telescopes on trajectory
      return &fIDTelescopes;
   }
   KVSeqCollection* AccessIDTelescopeList()
   {
      // Modifiable list of identification telescopes on trajectory
      return &fIDTelescopes;
   }
   Int_t GetNumberOfIdentifications() const
   {
      return fIDTelescopes.GetEntries();
   }
   Bool_t ContainsPath(KVGeoDNTrajectory* other)
   {
      // Returns kTRUE if this trajectory contains the other trajectory as a sub-trajectory
      return GetPathString().Contains(other->GetPathString());
   }
   TString GetPathString() const
   {
      // Returns string containing path of trajectory
      if (fPathInTitle) return GetTitle();
      return GetName();
   }
   const Char_t* GetTrajectoryName() const
   {
      // Returns string containing name of trajectory
      if (fPathInTitle) return GetName();
      return GetTitle();
   }
   void SetPathInTitle(Bool_t intitle = kTRUE)
   {
      // intitle = kTRUE: path in title
      // intitle = kFALSE: path in name
      // Any call to this method which changes state of fPathInTitle
      // will swap the name and title of the object

      if (intitle != fPathInTitle) {
         TString s = GetName();
         SetName(GetTitle());
         SetTitle(s);
         fPathInTitle = intitle;
      }
   }
   Bool_t IsPathInTitle() const
   {
      // returns kTRUE if path is contained in title, kFALSE if in name
      return fPathInTitle;
   }
   void AddIdentifiedParticle(int modify_unidentified = -1) const
   {
      // update counters of identified/unidentified particles in detectors on trajectory
      increment_identified_particle_counters(1, modify_unidentified);
   }
   void AddUnidentifiedParticle(int modify_identified = -1) const
   {
      // update counters of identified/unidentified particles in detectors on trajectory
      increment_identified_particle_counters(modify_identified, 1);
   }

   ClassDef(KVGeoDNTrajectory, 1) //Path taken by particles through multidetector geometry
};

#endif
