//Created by KVClassFactory on Mon Oct  5 17:32:21 2015
//Author: John Frankland,,,

#ifndef __KVGEONODEITERATOR_H
#define __KVGEONODEITERATOR_H

#include "KVBase.h"
#include "KVGeoDetectorNode.h"
#include "KVGeoDNTrajectory.h"

/**
 \class KVGeoNodeIterator
 \ingroup Geometry
 \brief Iterate from node to node along trajectories in array geometry

 Use a KVGeoNodeIterator in order to iterate over all nodes on all trajectories
 starting from a given node:
~~~~{.cpp}
     KVGeoDetectorNode* start; =//= node to start iteration from

     KVGeoNodeIterator next_node(start);
     KVGeoDetectorNode* node;

     while( (node = next_node()) ){
        =//= do something to each node
     }
~~~~
 Note that the start node for the iteration is included, and will be iterated
 over at the beginning of each trajectory. To exclude the start node from
 the iteration, call method ExcludeStartNode(kTRUE) before iteration begins.

 Default direction for iteration is FORWARDS i.e. moving towards the target.
 To change direction, call method IterateBackwards(kTRUE) before iteration begins.
 */

class KVGeoNodeIterator : public KVBase {
   KVGeoDetectorNode* start_node;
   mutable KVGeoDetectorNode* current_node;
   KVGeoDNTrajectory* iter_on_traj; // limit iteration to this trajectory, if given
   TIter* next_trajectory;
   mutable KVGeoDNTrajectory* current_trajectory;
   Bool_t exclude_start_node;
   Bool_t iterate_backwards;

   void begin_iteration_on_next_trajectory() const
   {
      if (iter_on_traj) {
         if (current_trajectory == nullptr) current_trajectory = iter_on_traj;
         else current_trajectory = nullptr;
      }
      else
         current_trajectory = (KVGeoDNTrajectory*)(*next_trajectory)();
      if (current_trajectory) {
         if (iterate_backwards) current_trajectory->IterateBackFrom(start_node);
         else current_trajectory->IterateFrom(start_node);
      }
   }
   void get_next_node_on_current_trajectory() const
   {
      if (current_trajectory) current_node = current_trajectory->GetNextNode();
      else current_node = nullptr;
   }


public:
   KVGeoNodeIterator(KVGeoDetectorNode* start, KVGeoDNTrajectory* trajectory = nullptr) :
      start_node(start), current_node(start), iter_on_traj(trajectory), next_trajectory(new TIter(start->GetTrajectories())),
      current_trajectory(nullptr), exclude_start_node(kFALSE), iterate_backwards(kFALSE)
   {
      // Begin an iteration over nodes starting at 'start'
      // If 'trajectory' is given, only nodes on 'trajectory' will be followed
      // Otherwise, all trajectories will be followed

      begin_iteration_on_next_trajectory();
   }

   virtual ~KVGeoNodeIterator()
   {
      delete next_trajectory;
   }

   void ExcludeStartNode(Bool_t u = kTRUE)
   {
      // Exclude start node from iteration
      exclude_start_node = u;
   }
   void IterateBackwards(Bool_t i = kTRUE)
   {
      // Perform all iterations backwards i.e. move away from target
      iterate_backwards = i;
   }

   KVGeoDetectorNode* operator()(void) const
   {
      // Return next node in iteration

      get_next_node_on_current_trajectory();
      if (current_node == nullptr) {
         begin_iteration_on_next_trajectory();
         get_next_node_on_current_trajectory();
      }
      if (exclude_start_node && current_node == start_node) return this->operator()();
      return current_node;
   }

   void Reset(KVGeoDetectorNode* start = nullptr, KVGeoDNTrajectory* trajectory = nullptr);
   void ResetTrajectory(KVGeoDNTrajectory* trajectory = nullptr);

   ClassDef(KVGeoNodeIterator, 0) //Iterator over geometry nodes/trajectories
};

#endif
