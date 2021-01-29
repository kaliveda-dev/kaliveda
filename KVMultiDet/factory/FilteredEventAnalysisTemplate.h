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

   void SetAnalysisFrame()
   {
      // Calculate centre of mass kinematics
      GetEvent()->SetFrame("CM", gDataAnalyser->GetKinematics()->GetCMVelocity());
   }

   ClassDef(FilteredEventAnalysisTemplate, 1) //event analysis class
};
