//Created by KVClassFactory on Mon Oct 19 09:33:44 2015
//Author: John Frankland,,,

#ifndef __KVEVENTRECONSTRUCTOR_H
#define __KVEVENTRECONSTRUCTOR_H

#include <vector>
#include "KVBase.h"
#include "KVDetectorEvent.h"
#include "KVMultiDetArray.h"
#include "KVReconstructedEvent.h"

/**
  \class KVEventReconstructor
  \ingroup Reconstruction
  \brief Base class for event reconstruction from array data

 Event reconstruction begins with a KVMultiDetArray containing hit
 detectors (either following the simulated detection of a simulated event,
 or after reading some experimental data corresponding to an experimental
 event) and ends with a KVReconstructedEvent object filled with
 KVReconstructedNucleus objects corresponding to the reconstructed
 (and possibly identfied & calibrated) particles.

 -# Get list of hit groups (KVDetectorEvent)
 -# For each hit group, perform reconstruction/identification/ etc. in the group
    using a KVGroupReconstructor-derived object (uses plugins)
 -# Merge together the different event fragments into the output reconstructed
    event object
*/
class KVEventReconstructor : public KVBase {

private:
   KVMultiDetArray*       fArray;//!       Array for which events are to be reconstructed
   KVReconstructedEvent*  fEvent;//!       The reconstructed event
   TObjArray       fGroupReconstructor;//! array of group reconstructors
   Int_t           fNGrpRecon;//!          number of group reconstructors for current event
   std::vector<int> fHitGroups;//!         group indices in current event
   KVDetectorEvent detev;//!               list of hit groups in event

protected:
   KVMultiDetArray* GetArray()
   {
      return fArray;
   }

public:
   KVEventReconstructor(KVMultiDetArray*, KVReconstructedEvent*, Bool_t = kFALSE);
   virtual ~KVEventReconstructor() {}

   void Copy(TObject& obj) const;

   void ReconstructEvent(const TSeqCollection* = nullptr);
   void MergeGroupEventFragments();

   KVReconstructedEvent* GetEvent()
   {
      return fEvent;
   }
   void* GetEventReference()
   {
      // for KVEvent::MakeEventBranch
      return &fEvent;
   }

   ClassDef(KVEventReconstructor, 0) //Base class for handling event reconstruction
};

#endif
