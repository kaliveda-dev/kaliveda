#ifndef __EXAMPLEFILTEREDSIMDATAANALYSIS_H
#define __EXAMPLEFILTEREDSIMDATAANALYSIS_H

/**
 \class ExampleFilteredSimDataAnalysis
 \brief Analysis of filtered simulated events

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Wed May 26 16:01:02 2021
*/

#include "KVEventSelector.h"
#include "KVZmax.h"
#include "KVDataAnalyser.h"

class ExampleFilteredSimDataAnalysis : public KVEventSelector {

   double ZVproj;

   Bool_t link_to_unfiltered_simulation;

public:
   ExampleFilteredSimDataAnalysis() {}
   virtual ~ExampleFilteredSimDataAnalysis() {}

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

   ClassDef(ExampleFilteredSimDataAnalysis, 1) //Analysis of filtered simulated events
};

#endif
