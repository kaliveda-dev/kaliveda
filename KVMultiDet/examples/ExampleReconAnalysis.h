#ifndef __EXAMPLERECONANALYSIS_H
#define __EXAMPLERECONANALYSIS_H

/**
 \class ExampleReconAnalysis
 \brief Analysis of reconstructed events

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Wed May 26 16:00:50 2021
*/

#include "KVReconEventSelector.h"

class ExampleReconAnalysis : public KVReconEventSelector {

public:
   ExampleReconAnalysis() {}
   virtual ~ExampleReconAnalysis() {}

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(ExampleReconAnalysis, 1) //Analysis of reconstructed events
};

#endif
