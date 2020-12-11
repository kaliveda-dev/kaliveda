//Created by KVClassFactory on Wed Feb 21 13:43:10 2018
//Author: John Frankland,,,

#ifndef __KVINDRAETALONGROUPRECONSTRUCTOR_H
#define __KVINDRAETALONGROUPRECONSTRUCTOR_H

#include "KVINDRABackwardGroupReconstructor.h"

/**
\class KVINDRAEtalonGroupReconstructor
\brief Reconstruct particles in INDRA groups with etalon telescopes
\ingroup INDRAReconstruction
*/

class KVINDRAEtalonGroupReconstructor : public KVINDRABackwardGroupReconstructor {

   Double_t fESi75, fESiLi;

protected:
   Bool_t CoherencyEtalons(KVReconstructedNucleus& PART);
   KVSi75* GetSi75(KVReconstructedNucleus* n)
   {
      return (KVSi75*)n->GetReconstructionTrajectory()->GetDetector("SI75");
   }
   KVSiLi* GetSiLi(KVReconstructedNucleus* n)
   {
      return (KVSiLi*)n->GetReconstructionTrajectory()->GetDetector("SILI");
   }
   void CalculateSiLiDEFromResidualEnergy(Double_t ERES, KVSiLi* sili, KVReconstructedNucleus* n);
   void CalculateSi75DEFromResidualEnergy(Double_t ERES, KVSi75* si75, KVReconstructedNucleus* n);
public:
   KVINDRAEtalonGroupReconstructor() {}
   virtual ~KVINDRAEtalonGroupReconstructor() {}

   KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);

   bool DoCoherencyAnalysis(KVReconstructedNucleus& PART);
   void DoCalibration(KVReconstructedNucleus* PART);

   ClassDef(KVINDRAEtalonGroupReconstructor, 1) //Reconstruct particles in INDRA groups with etalon telescopes
};

#endif
