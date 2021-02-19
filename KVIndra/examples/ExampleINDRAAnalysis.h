#ifndef __EXAMPLEINDRAANALYSIS_H
#define __EXAMPLEINDRAANALYSIS_H

/**
 \class ExampleINDRAAnalysis
 \brief User analysis class

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Fri Feb 19 13:41:03 2021
*/

#include "KVINDRAEventSelector.h"

class ExampleINDRAAnalysis : public KVINDRAEventSelector {

   double ztot_sys, zvtot_sys;

public:
   ExampleINDRAAnalysis() {};
   virtual ~ExampleINDRAAnalysis() {};

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(ExampleINDRAAnalysis, 1) //User analysis class
};

#endif
