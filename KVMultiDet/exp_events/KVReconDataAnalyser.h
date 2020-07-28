//Created by KVClassFactory on Thu Jul 19 2018
//Author: John Frankland

#ifndef __KVReconDataAnalyser_H
#define __KVReconDataAnalyser_H

#include "KVDataSetAnalyser.h"
#include "KVReconEventSelector.h"
class TChain;
class KVDBRun;

/**
  \class KVReconDataAnalyser
  \ingroup Analysis
  \brief Manages user analysis of reconstructed experimental data
 */
class KVReconDataAnalyser: public KVDataSetAnalyser {

protected:
   KVReconEventSelector* fSelector;// the data analysis class
   TTree* theChain;//chain of TTrees to be analysed
   Long64_t Entry;

   Long64_t TotalEntriesToRead;
   KVString fDataVersion;//KV version used to write analysed data
   KVString fDataSeries;//KV series used to write analysed data
   Int_t fDataReleaseNum;//KV release number used to write analysed data

public:

   Long64_t GetTotalEntriesToRead() const
   {
      return TotalEntriesToRead;
   }

   KVReconDataAnalyser();
   virtual ~ KVReconDataAnalyser();

   TTree* GetTree() const
   {
      return theChain;
   }
   void SetTree(TTree* t)
   {
      theChain = t;
   }

   virtual void Reset();

   virtual Bool_t CheckTaskVariables(void);
   virtual void SubmitTask();
   virtual void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);

   void preInitAnalysis();
   void preAnalysis();
   void preInitRun();
   virtual void RegisterUserClass(TObject* obj)
   {
      // The user class inherits from KVReconEventSelector
      fSelector = dynamic_cast<KVReconEventSelector*>(obj);
   }
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

   ClassDef(KVReconDataAnalyser, 0) //For analysing reconstructed data
};

#endif
