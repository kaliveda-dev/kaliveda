#ifndef __KVINDRADSTReader_H
#define __KVINDRADSTReader_H

#include "KVDataSetAnalyser.h"
#include "KVDataSet.h"

/**
\class KVINDRADSTReader
\brief Read INDRA DST file and fill a TTree
\ingroup INDRAnalysis
*/
class KVINDRADSTReader : public KVDataSetAnalyser {

protected:

   TTree* data_tree;//tree for writing events

   Int_t fRunNumber;//run number of current file
   Int_t fCampNumber;
   Bool_t camp1;

   Bool_t camp2;//set to kTRUE when reading 2nd campaign: => phoswich code is 4, not 2

   Bool_t camp4;//set to kTRUE when reading 4th campaign: => ring1 is Si-CsI

   void SetCampagneNumber()
   {
      fCampNumber = -1;
      KVString snom = gDataSet->GetName();
      KVNumberList nl = "1 2 4";
      nl.Begin();
      while (!nl.End()) {
         Int_t cc = nl.Next();
         if (snom.EndsWith(Form("%d", cc))) {
            fCampNumber = cc;
            break;
         }
      }
      Info("SetCampagneNumber", "%s -> Campagne numero %d", gDataSet->GetName(), fCampNumber);
   }

   void DefineSHELLVariables();

public:

   KVINDRADSTReader() {}
   virtual ~KVINDRADSTReader() {}

   virtual void InitRun();
   virtual void ProcessRun();
   virtual void EndRun();

   virtual void SubmitTask();


   ClassDef(KVINDRADSTReader, 1) //Read INDRA DST files and fill ROOT TTree
};

#endif
