//Created by KVClassFactory on Sat Oct 10 09:37:42 2015
//Author: John Frankland,,,

#ifndef __KVDETECTIONSIMULATOR_H
#define __KVDETECTIONSIMULATOR_H

#include "KVBase.h"
#include "KVMultiDetArray.h"
#include "KVEvent.h"
#include "KVDetectorEvent.h"
#include "KVTarget.h"
#include "KVRangeTableGeoNavigator.h"

/**
  \class KVDetectionSimulator
  \ingroup Simulation
  \brief Simulate detection of particles or events in a detector array
 */
class KVDetectionSimulator : public KVBase {

private:
   KVMultiDetArray* fArray;//           array used for detection
   KVDetectorEvent fHitGroups;//        used to reset hit detectors in between events
   Bool_t fCalcTargELoss;//             whether to include energy loss in target, if defined

public:
   KVDetectionSimulator() : KVBase(), fArray(nullptr), fCalcTargELoss(kTRUE) {}
   KVDetectionSimulator(KVMultiDetArray* a, Double_t cut_off = 1.e-3);
   virtual ~KVDetectionSimulator() {}

   void SetArray(KVMultiDetArray* a)
   {
      fArray = a;
   }
   KVMultiDetArray* GetArray() const
   {
      return fArray;
   }
   KVTarget* GetTarget() const
   {
      return fArray->GetTarget();
   }
   void SetIncludeTargetEnergyLoss(Bool_t y = kTRUE)
   {
      fCalcTargELoss = y;
   }
   Bool_t IncludeTargetEnergyLoss() const
   {
      return fCalcTargELoss;
   }
   Double_t GetMinKECutOff() const
   {
      return static_cast<KVRangeTableGeoNavigator*>(GetArray()->GetNavigator())->GetCutOffKEForPropagation();
   }
   void SetMinKECutOff(Double_t cutoff)
   {
      static_cast<KVRangeTableGeoNavigator*>(GetArray()->GetNavigator())->SetCutOffKEForPropagation(cutoff);
   }

   void DetectEvent(KVEvent* event, const Char_t* detection_frame = "");
   KVNameValueList DetectParticle(KVNucleus*);
   KVNameValueList DetectParticleIn(const Char_t* detname, KVNucleus* kvp);

   void ClearHitGroups()
   {
      // Reset any detectors/groups hit by previous detection
      fHitGroups.Clear();
   }

   ClassDef(KVDetectionSimulator, 0) //Simulate detection of particles or events in a detector array
};

#endif
