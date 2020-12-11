//Created by KVClassFactory on Tue Feb 27 11:35:25 2018
//Author: John Frankland,,,

#ifndef __KVINDRABACKWARDGROUPRECONSTRUCTOR_H
#define __KVINDRABACKWARDGROUPRECONSTRUCTOR_H

#include "KVINDRAGroupReconstructor.h"

/**
\class KVINDRABackwardGroupReconstructor
\brief Reconstruct data in rings 10-17 of INDRA
\ingroup INDRAReconstruction
*/

class KVINDRABackwardGroupReconstructor : public KVINDRAGroupReconstructor {
protected:
   Bool_t CoherencyChIoCsI(KVReconstructedNucleus& PART);
public:
   KVINDRABackwardGroupReconstructor() {}
   virtual ~KVINDRABackwardGroupReconstructor() {}

   bool DoCoherencyAnalysis(KVReconstructedNucleus& PART);
   void DoCalibration(KVReconstructedNucleus* PART);

   ClassDef(KVINDRABackwardGroupReconstructor, 1) //Reconstruct data in rings 10-17 of INDRA
};

#endif
