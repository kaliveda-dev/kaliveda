/*
$Id: KVDataAnalyser.h,v 1.18 2009/04/01 09:28:34 franklan Exp $
$Revision: 1.18 $
$Date: 2009/04/01 09:28:34 $
$Author: franklan $
*/

#ifndef __KVDATAANALYSER_H
#define __KVDATAANALYSER_H

#include "TObject.h"
#include "TTree.h"
#include "KVList.h"
#include "KVBatchSystem.h"
#include "KVString.h"
#include "KVNumberList.h"
#include "TEnv.h"

class KVDataSet;
class KVDataAnalysisTask;
class KVDBSystem;

class KVDataAnalyser : public TObject {

protected:

   KVDataAnalysisTask* fTask;   //task chosen by user
   KVString fDataType;          //datatype chosen by user
   Bool_t fBatch;               //set to kTRUE when used in a non-interactive environment
   KVString fBatchName;         //name of batch job when used in a non-interactive environment
   KVDBSystem* fSystem;         //system chosen by user
   KVNumberList fRunList;       //list of runs to analyse
   KVDataSet* fDataSet;         //dataset chosen by user
   Long64_t nbEventToRead;

   Bool_t fMenus; //set to kTRUE in menu-driven mode
   Bool_t fQuit;                //set to kTRUE when user wants to quit
   Bool_t fChoozDataSet;        //set to kTRUE when user wants to choose a dataset
   Bool_t fChoozTask;           //set to kTRUE when user wants to choose analysis task
   Bool_t fChoozSystem;         //set to kTRUE when user wants to choose a system
   Bool_t fChoozRuns;           //set to kTRUE when user wants to choose runs
   Bool_t fSubmit;              //set to kTRUE when user wants to submit task

   KVString fIncludes;           //user's include directories
   KVString fLibraries;          //user's libraries
   KVString fUserClass;    //user's analysis class
   KVString fUserClassImp;    //user's analysis class implementation file
   KVString fUserClassDec;    //user's analysis class header file
   Bool_t fUserClassIsOK;  //set once user class has been validated
   KVString fUserClassOptions;   //options to be passed to user analysis class

   KVDataAnalyser* fParent;   //holds address of parent analyser which created this one

   TEnv* fBatchEnv; // batch environment file
   KVBatchSystem* fBatchSystem;//batch system to use for submission of job
   Bool_t fChoseRunMode; //set to kTRUE once running mode has been set/chosen

   TList* fWorkDirInit;//list of files in working directory before task runs
   TList* fWorkDirEnd;//list of files in working directory after task runs
   virtual void ScanWorkingDirectory(TList**);
   void CopyAnalysisResultsToLaunchDirectory();

   virtual KVNumberList PrintAvailableRuns(KVString& datatype);
   virtual Bool_t CheckIfUserClassIsValid();
   virtual TObject* GetInstanceOfUserClass();

   virtual void PrintSplashScreen();

   virtual const Char_t* GetACliCMode();

public:

   KVDataAnalyser();
   virtual ~ KVDataAnalyser();

   virtual void SetUserClass(const Char_t* kvs, Bool_t check = kTRUE);
   virtual const Char_t* GetUserClass()
   {
      return fUserClass.Data();
   };
   Bool_t IsUserClassValid() const
   {
      return fUserClassIsOK;
   }

   virtual void Run();
   virtual void RunMenus();
   virtual void ChooseDataSet();
   virtual void ChooseDataType();
   virtual void ChooseAnalysisTask();
   virtual void ChooseSystem(const Char_t* data_type = "");
   virtual void ChooseRuns(KVDBSystem* system =
                              0, const Char_t* data_type = "");
   virtual void ChooseUserClass();
   virtual void ChooseRunningMode();
   virtual void SubmitTask();
   virtual Bool_t CheckTaskVariables();
   virtual Bool_t DoUserClassFilesExist();

   virtual void SetBatchMode(Bool_t on = kTRUE)
   {
      fBatch = on;
   };
   virtual Bool_t BatchMode() const
   {
      return fBatch;
   };
   virtual void SetBatchName(const Char_t* batchname)
   {
      fBatchName = batchname;
   };
   virtual const Char_t* GetBatchName()
   {
      return fBatchName.Data();
   };
   virtual void SetBatchSystem(KVBatchSystem* bs)
   {
      fBatchSystem = bs;
      fChoseRunMode = kTRUE;
      if (bs) fBatchSystem->cd();
   };
   virtual const KVBatchSystem* GetBatchSystem()
   {
      return fBatchSystem;
   };
   virtual Bool_t RunningInLaunchDirectory();
   virtual const Char_t* SystemBatchName();

   const Char_t* GetLaunchDirectory() const;
   const Char_t* GetBatchStatusFileName() const;
   void UpdateBatchStatusFile(Int_t totev, Int_t evread, TString disk) const;
   void DeleteBatchStatusFile() const;
   virtual Long64_t GetTotalEntriesToRead() const
   {
      return 0;
   }
   Bool_t CheckStatusUpdateInterval(Int_t nevents) const;
   void DoStatusUpdate(Int_t nevents) const;

   virtual KVDataAnalysisTask* GetAnalysisTask()
   {
      return fTask;
   }
   virtual KVString& GetDataType()
   {
      return fDataType;
   }
   virtual KVDBSystem* GetSystem()
   {
      return fSystem;
   }
   virtual KVNumberList& GetRunList()
   {
      return fRunList;
   }
   virtual KVDataSet* GetDataSet()
   {
      return fDataSet;
   }
   virtual KVString& GetUserIncludes()
   {
      return fIncludes;
   }
   virtual KVString& GetUserLibraries()
   {
      return fLibraries;
   }

   virtual void SetNbEventToRead(Long64_t nb = 0)
   {
      nbEventToRead = nb;
   }

   virtual Long64_t GetNbEventToRead(void)
   {
      return nbEventToRead;
   }

   virtual void ChooseNbEventToRead();
   virtual void SetDataSet(KVDataSet* ds);
   virtual void SetDataSet(const Char_t* name);
   virtual void SetAnalysisTask(KVDataAnalysisTask* at);
   virtual void SetDataType(const Char_t* name)
   {
      fDataType = name;
   }
   virtual void SetSystem(KVDBSystem* syst);
   virtual void SetRuns(KVNumberList& nl, Bool_t check = kTRUE);
   virtual void SetRuns(const Char_t*, Bool_t check = kTRUE);

   virtual void SetUserIncludes(const Char_t* incDirs = 0);
   virtual void SetUserLibraries(const Char_t* libs = 0);

   virtual void ClearRunList()
   {
      fRunList.Clear();
   };
   virtual void Reset();

   virtual void SetParent(KVDataAnalyser* da)
   {
      fParent = da;
   };
   virtual KVDataAnalyser* GetParent() const
   {
      return fParent;
   };

   static KVDataAnalyser* GetAnalyser(const Char_t* plugin);
   virtual Bool_t ReadBatchEnvFile(const Char_t*);
   virtual void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);

   virtual const Char_t* ExpandAutoBatchName(const Char_t* format);

   /* methods which can be used to customise analysis class behaviour.
      they can be called from the analysis class using the global
      pointer to the current data analyser, gDataAnalyser  */
   virtual void preInitAnalysis() {};
   virtual void postInitAnalysis() {};
   virtual void preInitRun() {};
   virtual void postInitRun() {};
   virtual void preAnalysis() {};
   virtual void postAnalysis() {};
   virtual void preEndRun() {};
   virtual void postEndRun() {};
   virtual void preEndAnalysis() {};
   virtual void postEndAnalysis() {};

   virtual void WriteBatchInfo(TTree*);
   virtual Int_t GetRunNumberFromFileName(const Char_t*);

   virtual void RegisterUserClass(TObject*) {};

   void SetUserClassOptions(const Char_t* o = "")
   {
      fUserClassOptions = o;
   }
   const KVString& GetUserClassOptions() const
   {
      return fUserClassOptions;
   }

   static void RunAnalyser(const Char_t* plugin = "");
   ClassDef(KVDataAnalyser, 0)  //For submitting & performing data analysis tasks
};

//................  global variable
R__EXTERN KVDataAnalyser* gDataAnalyser;

#endif
