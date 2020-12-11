//Created by KVClassFactory on Tue Jan 26 16:01:47 2016
//Author: bonnet,,,

#ifndef __KVFAZIARAWDATAANALYSER_H
#define __KVFAZIARAWDATAANALYSER_H

#include "KVDataSetAnalyser.h"

/**
\class KVFAZIARawDataAnalyser
\brief Pilot analysis of raw FAZIA data
\ingroup FAZIAnal
*/

class KVFAZIARawDataAnalyser : public KVDataSetAnalyser {
   TTree* theTree;
   Int_t fRunNumber;             //run number of current file

public:
   KVFAZIARawDataAnalyser();
   virtual ~KVFAZIARawDataAnalyser();

   virtual void preInitAnalysis();
   virtual void preInitRun();

   void SubmitTask();
   void ProcessRun();
   Int_t GetCurrentRunNumber() const
   {
      return fRunNumber;
   }

   ClassDef(KVFAZIARawDataAnalyser, 1) //Handle analysis for FAZIA data
};

#endif
