#include "KVEventSelector.h"
#include "KVZmax.h"
#include "KVDataAnalyser.h"

class FilteredEventAnalysisTemplate : public KVEventSelector {

   Bool_t link_to_unfiltered_simulation;

public:
   FilteredEventAnalysisTemplate() {}
   virtual ~FilteredEventAnalysisTemplate() {}

   void InitAnalysis();
   void InitRun();
   Bool_t Analysis();
   void EndRun() {}
   void EndAnalysis() {}

   ClassDef(FilteredEventAnalysisTemplate, 1) //event analysis class
};
