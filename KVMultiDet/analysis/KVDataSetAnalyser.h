//Created by KVClassFactory on Tue Feb 14 12:32:06 2017
//Author: John Frankland,,,

#ifndef __KVDATASETANALYSER_H
#define __KVDATASETANALYSER_H

#include "KVDataAnalyser.h"
#include "KVDataSet.h"

/**
\class KVDataSetAnalyser
\brief Pilots user analysis of experimental data
\ingroup Analysis
*/

class KVDataSetAnalyser : public KVDataAnalyser {
private:
   KVDBSystem* fSystem;         //system chosen by user
   KVDBRun* fRun;               //currently analysed run
   KVNumberList fRunList;       //list of runs to analyse
   KVNumberList fFullRunList;   //list of all runs for the analysis task
   KVDataSet* fDataSet;         //dataset chosen by user

   Bool_t fChoozDataSet;        //set to kTRUE when user wants to choose a dataset
   Bool_t fChoozTask;           //set to kTRUE when user wants to choose analysis task
   Bool_t fChoozSystem;         //set to kTRUE when user wants to choose a system
   Bool_t fChoozRuns;           //set to kTRUE when user wants to choose runs

protected:
   void SetCurrentRun(KVDBRun* r)
   {
      fRun = r;
   }
   virtual KVNumberList PrintAvailableRuns(KVString& datatype);
   virtual void set_dataset_pointer(KVDataSet* ds);
   virtual void set_dataset_name(const Char_t* name);
   void _set_dataset_pointer(KVDataSet* ds)
   {
      fDataSet = ds;
   }
   virtual Bool_t PreSubmitCheck();
   virtual void PostRunReset();

   Bool_t NeedToChooseWhatToDo() const
   {
      return fChoozDataSet || IsChooseTask();
   }
   Bool_t NeedToChooseWhatToAnalyse() const
   {
      return fChoozSystem || fChoozRuns;
   }
   void ChooseWhatToAnalyse();
   void ChooseWhatToDo();
   Bool_t CheckWhatToAnalyseAndHow();
   virtual void set_up_analyser_for_task(KVDataAnalyser* the_analyser);

public:
   TString SystemBatchName() const;
   KVDataSetAnalyser();
   virtual ~KVDataSetAnalyser();

   void ChooseDataSet();
   void ChooseDataType();
   virtual void ChooseAnalysisTask();
   void ChooseSystem(const Char_t* data_type = "");
   void ChooseRuns(KVDBSystem* system = nullptr, const Char_t* data_type = "");
   void Reset();

   const KVDBSystem* GetSystem() const
   {
      return fSystem;
   }
   const KVNumberList& GetRunList() const
   {
      return fRunList;
   }
   Int_t GetNumberOfFilesToAnalyse() const
   {
      return fRunList.GetNValues();
   }
   const KVNumberList& GetFullRunList() const
   {
      return fFullRunList;
   }
   const KVDataSet* GetDataSet() const
   {
      return fDataSet;
   }
   void SetDataSet(KVDataSet* ds)
   {
      set_dataset_pointer(ds);
   }
   void SetDataSet(const Char_t* name)
   {
      set_dataset_name(name);
   }
   void SetSystem(KVDBSystem* syst);
   void SetRuns(const KVNumberList& nl, Bool_t check = kTRUE);
   void SetFullRunList(const KVNumberList& nl)
   {
      fFullRunList = nl;
   }

   void ClearRunList()
   {
      fRunList.Clear();
   }
   Int_t GetRunNumberFromFileName(const Char_t*);
   void SetChooseDataSet(Bool_t yes = kTRUE)
   {
      fChoozDataSet = yes;
   }
   Bool_t IsChooseDataSet() const
   {
      return fChoozDataSet;
   }
   void SetChooseTask(Bool_t yes = kTRUE)
   {
      fChoozTask = yes;
   }
   Bool_t IsChooseTask() const
   {
      return fChoozTask;
   }
   void SetChooseSystem(Bool_t yes = kTRUE)
   {
      fChoozSystem = yes;
   }
   Bool_t IsChooseSystem() const
   {
      return fChoozSystem;
   }
   void SetChooseRuns(Bool_t yes = kTRUE)
   {
      fChoozRuns = yes;
   }
   Bool_t IsChooseRuns() const
   {
      return fChoozRuns;
   }
   void WriteBatchEnvFile(const Char_t* jobname, Bool_t save = kTRUE);
   Bool_t ReadBatchEnvFile(const Char_t* filename);
   TString ExpandAutoBatchName(const Char_t* format) const;
   virtual void AddJobDescriptionList(TList*);
   KVString GetRootDirectoryOfDataToAnalyse() const;
   const Char_t* GetRecognisedAutoBatchNameKeywords() const;

   const KV2Body* GetKinematics() const;
   KVDBSystem* GetAnalysedSystem() const
   {
      return fSystem;
   }
   KVDBRun* GetAnalysedRun() const
   {
      return fRun;
   }
   virtual void SetTree(TTree*) {}

   ClassDef(KVDataSetAnalyser, 1) //Analysis of data in datasets
};

#endif
