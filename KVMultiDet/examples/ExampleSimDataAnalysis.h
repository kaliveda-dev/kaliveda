#ifndef __EXAMPLESIMDATAANALYSIS_H
#define __EXAMPLESIMDATAANALYSIS_H

/**
 \class ExampleSimDataAnalysis
 \brief Analysis of simulated events

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Wed May 26 16:00:56 2021
*/

#include "KVEventSelector.h"

class ExampleSimDataAnalysis : public KVEventSelector {

public:
   ExampleSimDataAnalysis() {}
   virtual ~ExampleSimDataAnalysis() {}

   void InitAnalysis();
   void InitRun() {}
   Bool_t Analysis();
   void EndRun() {}
   void EndAnalysis() {}

   ClassDef(ExampleSimDataAnalysis, 1) //Analysis of simulated events
};

#endif
