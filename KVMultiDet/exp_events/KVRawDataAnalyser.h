//Created by KVClassFactory on Thu Sep 24 11:07:45 2009
//Author: John Frankland,,,

#ifndef __KVRAWDATAANALYSER_H
#define __KVRAWDATAANALYSER_H

#include "KVDataSetAnalyser.h"
#include "KVHashList.h"
#include "KVRawDataReader.h"
#include "KVDBRun.h"

#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <TProfile2D.h>

class KVDetectorEvent;

/**
   \class KVRawDataAnalyser
   \brief Abstract base class for user analysis of raw data
 \ingroup Analysis
*/

class KVRawDataAnalyser : public KVDataSetAnalyser {
protected:

   KVRawDataReader* fRunFile;    //currently analysed run file
   Int_t fRunNumber;             //run number of current file
   Long64_t fEventNumber;        //event number in current run
   KVHashList fHistoList;        //list of histograms of user analysis
   KVHashList fTreeList;         //list of trees of user analysis
   TFile* fTreeFile;
   KVDBRun* fCurrentRun;         //poiner to current run
   Long64_t TotalEntriesToRead;
   KVString fCombinedOutputFile;// optional name for single results file with trees and histos

   void ProcessRun();

   void AbortDuringRunProcessing();

   /*** Methods copied from KVEventSelector ***/

   void FillTH1(TH1* h1, Double_t one, Double_t two)
   {
      h1->Fill(one, two);
   }
   void FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three)
   {
      h1->Fill(one, two, three);
   }
   void FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three)
   {
      h2->Fill(one, two, three);
   }
   void FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four)
   {
      h2->Fill(one, two, three, four);
   }
   void FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four)
   {
      h3->Fill(one, two, three, four);
   }

   void SetCombinedOutputFile(const KVString& filename)
   {
      // Call in InitAnalysis() to set the name of the single output file
      // containing all histograms and TTrees produced by analysis.
      // This is equivalent to running the analysis with option
      //    CombinedOutputFile=[filename]
      // but setting this option in InitAnalysis() will not work.
      // Note that if this method is not called/the option is not given,
      // histograms and TTrees will be written in separate files.
      fCombinedOutputFile = filename;
   }
   /*** END: Methods copied from KVEventSelector ***/

public:
   Long64_t GetTotalEntriesToRead() const
   {
      return TotalEntriesToRead;
   }

   KVRawDataAnalyser();
   virtual ~KVRawDataAnalyser();

   KVDetectorEvent* GetDetectorEvent() const
   {
      Obsolete("GetDetectorEvent", "1.11", "2.0");
      return nullptr;
   }

   virtual void InitAnalysis() = 0;
   virtual void InitRun() = 0;
   virtual Bool_t Analysis() = 0;
   virtual void EndRun() = 0;
   virtual void EndAnalysis() = 0;

   Int_t GetRunNumber() const
   {
      return fRunNumber;
   }
   Long64_t GetEventNumber() const
   {
      return fEventNumber;
   }
   KVDBRun* GetCurrentRun() const
   {
      return fCurrentRun;
   }

   void SubmitTask();
   static void Make(const Char_t* kvsname = "MyOwnRawDataAnalyser");

   void CalculateTotalEventsToRead();

   const KVRawDataReader& GetRunFileReader() const
   {
      return *fRunFile;
   }

   /*** Methods copied from KVEventSelector ***/

   void AddHisto(TH1* histo)
   {
      fHistoList.Add(histo);
   }
   void AddTree(TTree* tree);

   void FillHisto(const Char_t* sname, Double_t one, Double_t two = 1, Double_t three = 1, Double_t four = 1);
   void FillHisto(const Char_t* sname, const Char_t* label, Double_t weight = 1);
   void FillTree(const Char_t* sname = "");

   const KVHashList* GetHistoList() const
   {
      return &fHistoList;
   }
   const KVHashList* GetTreeList() const
   {
      return &fTreeList;
   }

   TH1* GetHisto(const Char_t* name) const
   {
      return fHistoList.get_object<TH1>(name);
   }
   TTree* GetTree(const Char_t* name) const
   {
      return fTreeList.get_object<TTree>(name);
   }

   virtual void SaveHistos(const Char_t* filename = "", Option_t* option = "recreate", Bool_t onlyfilled = kFALSE);
   void SetJobOutputFileName(const TString& filename)
   {
      // Call in InitAnalysis() to set the name of the single output file
      // containing all histograms and TTrees produced by analysis.
      //
      // For interactive jobs or jos using PROOF, filename will be used for
      // the ROOT file. For jobs using a batch system to execute many
      // jobs in parallel, we use the job name with the '.root' extension.

#ifdef WITH_CPP11
      if (IsRunningBatchAnalysis() && (GetProofMode() == KVDataAnalyser::EProofMode::None))
#else
      if (IsRunningBatchAnalysis() && (GetProofMode() == KVDataAnalyser::None))
#endif
         SetCombinedOutputFile(Form("%s.root", GetBatchSystem()->GetJobName()));

      else
         SetCombinedOutputFile(filename);
   }
   Bool_t CreateTreeFile(const Char_t* filename = "");

   /*** END: Methods copied from KVEventSelector ***/

   ClassDef(KVRawDataAnalyser, 1) //Abstract base class for user analysis of raw data
};

#endif
