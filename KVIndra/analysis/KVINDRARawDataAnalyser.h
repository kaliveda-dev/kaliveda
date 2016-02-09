//Created by KVClassFactory on Fri May  4 15:26:02 2007
//Author: franklan

#ifndef __KVINDRARAWDATAANALYSER_H
#define __KVINDRARAWDATAANALYSER_H

#include "KVRawDataAnalyser.h"
#include "KVDetectorEvent.h"
#include "KVDataSet.h"
#include "KVGANILDataReader.h"

class KVINDRARawDataAnalyser : public KVRawDataAnalyser {
   friend class KVDataSet;

protected:
   Int_t INDRA_events;
   Int_t gene_events;
   Int_t other_events;

   virtual KVNumberList PrintAvailableRuns(KVString& datatype);

public:

   KVINDRARawDataAnalyser();
   virtual ~KVINDRARawDataAnalyser();

   void postInitRun();
   void preAnalysis();
   void preEndRun();

   KVGANILDataReader* GetRawDataReader() const
   {
      return (KVGANILDataReader*)fRunFile;
   };

   static void Make(const Char_t* kvsname = "MyOwnINDRARawDataAnalyser");

   virtual void CalculateTotalEventsToRead();

   ClassDef(KVINDRARawDataAnalyser, 2) //Analysis of raw INDRA data
};

#endif
