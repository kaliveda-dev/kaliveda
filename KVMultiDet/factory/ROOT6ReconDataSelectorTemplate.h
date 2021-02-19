#include "KVReconEventSelector.h"

class ROOT6ReconDataSelectorTemplate : public KVReconEventSelector {

public:
   ROOT6ReconDataSelectorTemplate() {}
   virtual ~ROOT6ReconDataSelectorTemplate() {}

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(ROOT6ReconDataSelectorTemplate, 0)
};
