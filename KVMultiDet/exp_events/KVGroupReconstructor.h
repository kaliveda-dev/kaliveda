//Created by KVClassFactory on Mon Oct 19 14:11:26 2015
//Author: John Frankland,,,

#ifndef __KVGROUPRECONSTRUCTOR_H
#define __KVGROUPRECONSTRUCTOR_H

#include "KVBase.h"
#include "KVGroup.h"
#include "KVReconstructedEvent.h"
#ifdef WITH_CPP11
#include <unordered_map>
#else
#include <map>
#endif
#include <string>

/**
  \class KVGroupReconstructor
  \ingroup Reconstruction
  \brief Base class for particle reconstruction in the one group of a detector array

Recalling that a group of detectors (KVGroup) is the largest part of an array which can be trated
independently of all other detectors, KVGroupReconstructor is the basic working unit of event
reconstruction. A KVEventReconstructor will use many KVGroupReconstructor objects in order to
reconstruct an event from data detected by the array.

Daughter classes of KVGroupReconstructor can be specialised for event reconstruction in
specific arrays (or even specific parts of specific arrays).

\sa KVEventReconstructor, KVGroup, KVMultiDetArray
 */
class KVGroupReconstructor : public KVBase {

   static bool fDoIdentification;
   static bool fDoCalibration;

   KVGroup*              fGroup;//!        the group where we are reconstructing
   KVReconstructedEvent* fGrpEvent;//!     event containing particles reconstructed in this group
   TString               fPartSeedCond;//! condition for seeding reconstructed particles
protected:
   mutable int nfireddets;//! number of fired detectors in group for current event
   KVIdentificationResult partID;//! identification to be applied to current particle
   KVIDTelescope* identifying_telescope;//! telescope which identified current particle
#ifdef WITH_CPP11
   std::unordered_map<std::string, KVIdentificationResult*> id_by_type; //! identification results by type for current particle
#else
   std::map<std::string, KVIdentificationResult*> id_by_type; //! identification results by type for current particle
#endif
   virtual KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);
   void ReconstructParticle(KVReconstructedNucleus* part, const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);
   virtual void PostReconstructionProcessing();
   virtual void IdentifyParticle(KVReconstructedNucleus& PART);
   virtual void CalibrateParticle(KVReconstructedNucleus* PART);

   Double_t GetTargetEnergyLossCorrection(KVReconstructedNucleus* ion);
   TString GetPartSeedCond() const
   {
      return fPartSeedCond;
   }
   void TreatStatusStopFirstStage(KVReconstructedNucleus&);

public:
   KVGroupReconstructor();
   virtual ~KVGroupReconstructor();

   void SetReconEventClass(TClass* c);
   void Copy(TObject& obj) const;
   int GetNFiredDets() const
   {
      return nfireddets;
   }

   KVReconstructedEvent* GetEventFragment() const
   {
      return fGrpEvent;
   }
   virtual void SetGroup(KVGroup* g);
   KVGroup* GetGroup() const
   {
      return fGroup;
   }

   static KVGroupReconstructor* Factory(const TString& plugin = "");

   void Process();
   void Reconstruct();
   virtual void Identify();
   void Calibrate();

   void AnalyseParticles();
   Int_t GetNIdentifiedInGroup()
   {
      //number of identified particles reconstructed in group
      Int_t n = 0;
      if (GetEventFragment()->GetMult()) {
         for (KVReconstructedEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
            KVReconstructedNucleus& nuc = it.get_reference();
            n += (Int_t) nuc.IsIdentified();
         }
      }
      return n;
   }
   Int_t GetNUnidentifiedInGroup()
   {
      //number of unidentified particles reconstructed in group
      return (GetEventFragment()->GetMult() - GetNIdentifiedInGroup());
   }
   static void SetDoIdentification(bool on = kTRUE)
   {
      // Enable/Disable identification step in KVGroupReconstructor::Process
      fDoIdentification = on;
   }
   static void SetDoCalibration(bool on = kTRUE)
   {
      // Enable/Disable calibration step in KVGroupReconstructor::Process
      fDoCalibration = on;
   }

   ClassDef(KVGroupReconstructor, 0) //Base class for handling event reconstruction in detector groups
};

#endif
