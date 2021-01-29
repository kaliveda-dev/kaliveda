#include "KVEventSelector.h"

class ROOT6SimulatedEventAnalysisTemplate : public KVEventSelector {

public:
   ROOT6SimulatedEventAnalysisTemplate() {}
   virtual ~ROOT6SimulatedEventAnalysisTemplate() {}

   void InitAnalysis();
   void InitRun() {}
   Bool_t Analysis();
   void EndRun() {}
   void EndAnalysis() {}

   ClassDef(ROOT6SimulatedEventAnalysisTemplate, 1) //event analysis class
};
