#ifndef KVFAZIAGROUPRECONSTRUCTOR_H
#define KVFAZIAGROUPRECONSTRUCTOR_H

#include "KVGroupReconstructor.h"

class KVFAZIAGroupReconstructor : public KVGroupReconstructor {
protected:

   void CalibrateParticle(KVReconstructedNucleus* PART);
   void PostReconstructionProcessing();
   void IdentifyParticle(KVReconstructedNucleus& PART);

   void  ChangeReconstructedTrajectory(KVReconstructedNucleus& PART, const char* det);

public:
   KVFAZIAGroupReconstructor() {}
   virtual ~KVFAZIAGroupReconstructor() {}

   ClassDef(KVFAZIAGroupReconstructor, 1) // Reconstruct particles in FAZIA telescopes
};

#endif // KVFAZIAGROUPRECONSTRUCTOR_H
