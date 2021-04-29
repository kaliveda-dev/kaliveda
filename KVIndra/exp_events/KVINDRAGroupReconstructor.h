//Created by KVClassFactory on Wed Feb 21 13:42:47 2018
//Author: John Frankland,,,

#ifndef __KVINDRAGROUPRECONSTRUCTOR_H
#define __KVINDRAGROUPRECONSTRUCTOR_H

#include "KVGroupReconstructor.h"
#include "KVChIo.h"
#include "KVCsI.h"
#include "KVINDRAReconNuc.h"

#define SETINDRAECODE(n,x) if(n->InheritsFrom("KVINDRAReconNuc")) \
      n->SetECode(kECode##x); \
   else \
      n->SetECode(x)
#define SETINDRAIDCODE(n,x) if(n->InheritsFrom("KVINDRAReconNuc")) \
      n->SetIDCode(kIDCode##x); \
   else \
      n->SetIDCode(x)


/**
\class KVINDRAGroupReconstructor
\brief Reconstruct particles in INDRA groups
\ingroup INDRAReconstruction
*/

class KVINDRAGroupReconstructor : public KVGroupReconstructor {

   friend class KVINDRA;

protected:
   KVChIo* theChio;                 // the ChIo of the group
   Double_t fECsI, fESi, fEChIo;
   bool print_part;//debug
   static TString CSI_ID_TYPE;

   void SetBadCalibrationStatus(KVReconstructedNucleus* n)
   {
      SETINDRAECODE(n, 15);
      n->SetEnergy(-1.0);
   }
   void SetNoCalibrationStatus(KVReconstructedNucleus* n)
   {
      SETINDRAECODE(n, 0);
      n->SetEnergy(0.0);
   }

   double DoBeryllium8Calibration(KVReconstructedNucleus* n);
   void CheckCsIEnergy(KVReconstructedNucleus* n);
   KVCsI* GetCsI(KVReconstructedNucleus* n)
   {
      return (KVCsI*)n->GetReconstructionTrajectory()->GetDetector("CSI");
   }

   void CalculateChIoDEFromResidualEnergy(KVReconstructedNucleus* n, Double_t ERES);
public:
   KVINDRAGroupReconstructor() {}
   virtual ~KVINDRAGroupReconstructor() {}

   void SetGroup(KVGroup* g)
   {
      KVGroupReconstructor::SetGroup(g);
      theChio = (KVChIo*)g->GetDetectorByType("CI");
   }

   KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);

   void Identify();
   void IdentifyParticle(KVReconstructedNucleus& PART);
   void CalibrateParticle(KVReconstructedNucleus* PART);

   virtual bool DoCoherencyAnalysis(KVReconstructedNucleus&) = 0;

   virtual void DoCalibration(KVReconstructedNucleus*) {}

   ClassDef(KVINDRAGroupReconstructor, 1) //Reconstruct particles in INDRA groups
};

#endif
