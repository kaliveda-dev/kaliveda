#include "KVEventSelector.h"
#include "KVZmax.h"
#include "KVDataAnalyser.h"

class ROOT6FilteredEventAnalysisTemplate : public KVEventSelector {

   double ZVproj;

   Bool_t link_to_unfiltered_simulation;

public:
   ROOT6FilteredEventAnalysisTemplate() {}
   virtual ~ROOT6FilteredEventAnalysisTemplate() {}

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

   ClassDef(ROOT6FilteredEventAnalysisTemplate, 1) //event analysis class
};
