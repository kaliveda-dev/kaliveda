//Created by KVClassFactory on Mon Oct  5 17:32:21 2015
//Author: John Frankland,,,

#include "KVGeoNodeIterator.h"


ClassImp(KVGeoNodeIterator)


void KVGeoNodeIterator::Reset(KVGeoDetectorNode* start, KVGeoDNTrajectory* trajectory)
{
   // Reset the iterator in order to perform a new iteration:
   //
   //    it.Reset();  => iterate from same node on same trajectory(ies)
   //                    i.e. if you originally specified a trajectory to iterate over,
   //                    it will be used again.
   //                    If you want to iterate from the same node but now
   //                    lifting the restriction on the trajectory, call it.ResetTrajectory()
   //
   //    it.Reset(start);  => iterate from a new node, using all trajectories
   //
   //    it.Reset(start,trajectory) => iterate from a new node using given trajectory

   if (start == nullptr) {
      current_node = start_node;
      next_trajectory->Reset();
   }
   else {
      current_node = start_node = start;
      iter_on_traj = trajectory;
      delete next_trajectory;
      next_trajectory = new TIter(start->GetTrajectories());
   }
   current_trajectory = nullptr;
   begin_iteration_on_next_trajectory();
}

void KVGeoNodeIterator::ResetTrajectory(KVGeoDNTrajectory* trajectory)
{
   // Reset the iterator in order to perform a new iteration starting
   // from the same node but either with a different trajectory, or lifting
   // the restriction on the trajectory originally given to the constructor:
   //
   //    KVGeoNodeIterator it(start,trajectory);
   //    while( (KVGeoDetectorNode* n = it()) ){
   //        /* iterate only over 'trajectory' */
   //    }
   //    it.ResetTrajectory();
   //    while( (KVGeoDetectorNode* n = it()) ){
   //        /* iterate over all trajectories */
   //    }
   //    it.ResetTrajectory(trajectory2);
   //    while( (KVGeoDetectorNode* n = it()) ){
   //        /* iterate only over 'trajectory2' */
   //    }

   current_node = start_node;
   iter_on_traj = trajectory;
   next_trajectory->Reset();
   current_trajectory = nullptr;
   begin_iteration_on_next_trajectory();
}
