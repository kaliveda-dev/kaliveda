//Created by KVClassFactory on Wed Jan 28 09:54:34 2015
//Author: Eric Bonnet

#ifndef __KVFAZIASelector_H
#define __KVFAZIASelector_H

#include "KVEventSelector.h"
#include "KVFAZIARawEvent.h"

/**
\class KVFAZIASelector
\brief Analysis of FAZIA data
\ingroup FAZIAnal
*/

class KVFAZIASelector : public KVEventSelector {

protected:

   Int_t fCurrentRun;

   Bool_t fAutomatic;
   TFile* fOutputFile;
   TString fOutputPath;

   Bool_t fRawData;
   KVFAZIARawEvent* RawEvent;
   TString rawdatabranchname;

   void SetAutomaticManagementOfOutputs(Bool_t automatic = kTRUE);
   Bool_t CreateOutputFile();
   Bool_t SaveAndCloseOutputFile();

public:
   KVFAZIASelector() : fCurrentRun(0), fRawData(kFALSE), RawEvent(nullptr)
   {
      rawdatabranchname = "rawevent";
      SetBranchName("FAZIAReconEvent");
      SetEventsReadInterval(10000);
      SetAutomaticManagementOfOutputs(kFALSE);
   };

   virtual ~KVFAZIASelector() {};

   virtual void    SlaveBegin(TTree* tree);
   virtual void    Init(TTree* tree);
   virtual Bool_t  Notify();
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0)
   {
      if (RawEvent) RawEvent->Clear();
      Int_t res = fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
      if (NeedToReadRawData() && RawEvent)
         ConnectSignalsToDetectors();
      return res;
   }
   Int_t GetCurrentRunNumber() const
   {
      return fCurrentRun;
   }

   Bool_t NeedToReadRawData() const
   {
      return fRawData;
   }
   Bool_t SetReadingOfRawData(Bool_t need = kFALSE)
   {
      return fRawData = need;
   }
   void LinkRawData();
   void ConnectSignalsToDetectors();

   virtual void ParseOptions();

   static void Make(const Char_t* kvsname = "MyOwnFAZIASelector");
   static void CheckListOfWellEndedRuns(const Char_t* kvsname, const Char_t* dsname);
   static void MergeListOfEndedRuns(const Char_t* kvsname, const Char_t* dsname);
   static void GetMissingRunsListing(const Char_t* kvsname, const Char_t* dsname);
   static void CheckBatchOutputFiles(const Char_t* kvsname, Bool_t eraseafter = kFALSE);

   virtual void CheckEndOfRun();

   ClassDef(KVFAZIASelector, 1) //selector to analyze FAZIA reconstructed data
};


#endif
