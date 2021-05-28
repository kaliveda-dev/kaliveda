#ifndef __KVRECONNUCTRAJECTORY_H
#define __KVRECONNUCTRAJECTORY_H

#include "KVGeoDNTrajectory.h"
#ifdef WITH_CPP11
#include <unordered_map>
#else
#include <map>
#endif

/**
  \class KVReconNucTrajectory
  \ingroup Reconstruction
  \brief Path through detector array used to reconstruct detected particle

  This class handles trajectories for reconstructed nuclei (see KVReconstructedNucleus).
  Each detected particle is reconstructed from a trajectory through the array corresponding
  to the detectors hit by the particle. When dealing with reconstructed particles, the trajectory
  of each can be accessed like so:

~~~~~~{.cpp}
KVReconstructedNucleus* rnuc_p; // some pointer to reconstructed nucleus
KVReconNucTrajectory* rnt = rnuc_p->GetReconstructionTrajectory();
~~~~~~

  For looping over all detectors on the trajectory, see KVGeoDNTrajectory class reference
  (section Iterate Over Trajectory).

  Note that, as for parent trajectory class KVGeoDNTractory, by default all reconstruction trajectories begin
  from the detector in which a nucleus stopped, and iteration over them moves towards the target (i.e. backwards in
  time regarding the flight of the nucleus into the detectors).
 */
class KVReconNucTrajectory : public KVGeoDNTrajectory {

   Int_t fIndependentIdentifications;//number of independent ID telescopes on trajectory
#ifdef WITH_CPP11
   std::unordered_map<std::string, KVDetector*> fDetLabels; //! map detector labels to detectors
#else
   std::map<std::string, KVDetector*> fDetLabels; //! map detector labels to detectors
#endif

public:
   KVReconNucTrajectory() : KVGeoDNTrajectory(), fIndependentIdentifications(0)
   {
      fAddToNodes = kFALSE;
   }
   KVReconNucTrajectory(const KVReconNucTrajectory&);
   KVReconNucTrajectory(const KVGeoDNTrajectory*, const KVGeoDetectorNode*);
   virtual ~KVReconNucTrajectory() {}
   KVReconNucTrajectory& operator=(const KVReconNucTrajectory&);

   void Copy(TObject& obj) const;

   Int_t GetNumberOfIndependentIdentifications() const
   {
      // Return number of independent ID telescopes on trajectory
      return fIndependentIdentifications;
   }
   void ls(Option_t* = "") const;
   KVDetector* GetDetector(const TString& label) const;

   ClassDef(KVReconNucTrajectory, 1) //Reconstructed trajectory of nucleus detected by array
};

#endif
