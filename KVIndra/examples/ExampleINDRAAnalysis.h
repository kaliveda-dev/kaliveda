//Created by KVClassFactory on Mon Feb 19 14:32:51 2018
//Author: John Frankland

#ifndef __ExampleINDRAAnalysis_H
#define __ExampleINDRAAnalysis_H

#include "KVINDRAEventSelector.h"

class ExampleINDRAAnalysis : public KVINDRAEventSelector {
   Int_t Run;
   Int_t EventNumber;
   Int_t MTensor;
   Int_t Trigger;
#ifdef USING_ROOT6
   Int_t current_run_system_ztot;
#endif

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
