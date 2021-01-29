#include "KVEventSelector.h"

class SimulatedEventAnalysisTemplate : public KVEventSelector {

public:
   SimulatedEventAnalysisTemplate() {}
   virtual ~SimulatedEventAnalysisTemplate() {}

   void InitAnalysis();
   void InitRun() {}
   Bool_t Analysis();
   void EndRun() {}
   void EndAnalysis() {}

   ClassDef(SimulatedEventAnalysisTemplate, 1) //event analysis class
};
