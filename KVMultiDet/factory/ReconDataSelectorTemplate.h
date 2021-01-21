#include "KVReconEventSelector.h"

class ReconDataSelectorTemplate : public KVReconEventSelector {

public:
   ReconDataSelectorTemplate() {}
   virtual ~ReconDataSelectorTemplate() {}

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(ReconDataSelectorTemplate, 0)
};
