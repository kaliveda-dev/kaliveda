/*
$Id: KVFAZIARawDataAnalyser.h,v 1.2 2007/05/31 09:59:22 ebonnet Exp $
$Revision: 1.2 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Wed Apr  5 23:50:04 2006
//Author: John Frankland

#ifndef __KVFAZIARawDataAnalyser_H
#define __KVFAZIARawDataAnalyser_H

#include "KVDataAnalyser.h"
#include "KVFAZIAReader.h"
#include "KVDetectorEvent.h"
class TChain;

class KVFAZIARawDataAnalyser: public KVDataAnalyser {

protected:
   KVString fDataSelector;//name of KVDataSelector to use
   KVString fDataSelectorImp;//name of KVDataSelector implementation file (if it exists)
   KVString fDataSelectorDec;//name of KVDataSelector header file (if it exists)

   virtual KVNumberList PrintAvailableRuns(KVString& datatype);
   KVFAZIAReader* fSelector;// the data analysis class
   TChain* theChain;//chain of TTrees to be analysed
   //TTree* theRawData;//raw data TTree in recon file
   Int_t NbParFired;
   TObjArray* parList;
   Long64_t Entry;
   //void ConnectRawDataTree();

   KVString fDataVersion;//KV version used to write analysed data
   KVString fDataSeries;//KV series used to write analysed data
   Int_t fDataReleaseNum;//KV release number used to write analysed data

public:

   KVFAZIARawDataAnalyser();
   virtual ~ KVFAZIARawDataAnalyser();

   virtual void SetKVDataSelector(const Char_t* kvs = "");

   virtual const Char_t* GetKVDataSelector(void)
   {
      return fDataSelector.Data();
   }

   virtual void Reset();
   virtual void ChooseKVDataSelector();

   virtual Bool_t CheckTaskVariables(void);
   virtual void SubmitTask();
   virtual void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);
   virtual Bool_t ReadBatchEnvFile(const Char_t*);

   virtual const Char_t* ExpandAutoBatchName(const Char_t* format);

   void preInitAnalysis();
   void preAnalysis();
   void preInitRun();
   virtual void RegisterUserClass(TObject* obj)
   {
      fSelector = (KVFAZIAReader*)obj;
   };
   void PrintTreeInfos();
   TEnv* GetReconDataTreeInfos() const;

   KVString GetDataVersion() const
   {
      return fDataVersion;
   }
   KVString GetDataSeries() const
   {
      return fDataSeries;
   }
   Int_t GetDataReleaseNumber() const
   {
      return fDataReleaseNum;
   }


   ClassDef(KVFAZIARawDataAnalyser, 0) //For analysing raw FAZIA data
};

#endif
