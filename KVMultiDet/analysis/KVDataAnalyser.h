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
#include "KV2Body.h"

class KVDataAnalysisTask;
class KVDataAnalyser : public TObject {

private:
   KVDataAnalysisTask* fTask;   //task chosen by user
   KVString fDataType;          //datatype chosen by user
   Bool_t fBatch;               //set to kTRUE when used in a non-interactive environment
   KVString fBatchName;         //name of batch job when used in a non-interactive environment
   Long64_t nbEventToRead;

   Bool_t fMenus; //set to kTRUE in menu-driven mode
   Bool_t fQuit;                //set to kTRUE when user wants to quit
   Bool_t fSubmit;              //set to kTRUE when user wants to submit task

   KVString fIncludes;           //user's include directories
   KVString fLibraries;          //user's libraries
   KVString fUserClass;    //user's analysis class
   KVString fUserClassImp;    //user's analysis class implementation file
   KVString fUserClassDec;    //user's analysis class header file
   Bool_t fUserClassIsOK;  //set once user class has been validated
   KVString fUserClassOptions;   //options to be passed to user analysis class
   KVString fUserClassAlternativeBaseClass;    //alternative base class for user's analysis class

   KVDataAnalyser* fParent;   //holds address of parent analyser which created this one

   TEnv* fBatchEnv; // batch environment file
   KVBatchSystem* fBatchSystem;//batch system to use for submission of job
   Bool_t fChoseRunMode; //set to kTRUE once running mode has been set/chosen

   TList* fWorkDirEnd;//list of files in working directory after task runs
   static Bool_t fCleanAbort;//flag to force abort of processing
   Bool_t fUseBaseClassSubmitTask;

   Double_t fStatusUpdateInterval;

protected:
   TList* fWorkDirInit;//list of files in working directory before task runs
   void ScanWorkingDirectory(TList**);
   void CopyAnalysisResultsToLaunchDirectory();

   virtual Bool_t CheckIfUserClassIsValid(const KVString& alternative_base_class = "");
   TObject* GetInstanceOfUserClass(const KVString& alternative_base_class = "");

   const Char_t* GetACliCMode();

   TEnv* GetBatchInfoFile() const
   {
      return fBatchEnv;
   }
   virtual Bool_t PreSubmitCheck()
   {
      return kTRUE;
   }
   virtual void PostRunReset();
   virtual Bool_t NeedToChooseWhatToDo() const
   {
      return !GetAnalysisTask();
   }
   virtual Bool_t NeedToChooseWhatToAnalyse() const
   {
      AbstractMethod("NeedToChooseWhatToAnalyse");
      return kTRUE;
   }
   virtual void ChooseWhatToAnalyse();
   virtual void ChooseWhatToDo();
   virtual Bool_t CheckWhatToAnalyseAndHow();

   virtual void set_up_analyser_for_task(KVDataAnalyser* the_analyser);

public:
   enum EProofMode {
      None,
      Lite,
      Proof
   };
private:
   EProofMode fProofMode;
public:
   void SetProofMode(EProofMode e)
   {
      fProofMode = e;
   }
   EProofMode GetProofMode() const
   {
      return fProofMode;
   }

   static void SetAbortProcessingLoop(Bool_t now = kTRUE)
   {
      // Set flag to force a clean abort of the processing loop
      fCleanAbort = now;
   }
   static Bool_t AbortProcessingLoop()
   {
      return fCleanAbort;
   }

   KVDataAnalyser();
   virtual ~ KVDataAnalyser();

   void SetUserClass(const Char_t* kvs, Bool_t check = kTRUE);
   const Char_t* GetUserClass()
   {
      return fUserClass.Data();
   }
   Bool_t IsUserClassValid() const
   {
      return fUserClassIsOK;
   }

   virtual Int_t GetNumberOfFilesToAnalyse() const
   {
      return 0;
   }

   virtual void Run();
   virtual void RunMenus();
   void ChooseUserClass();
   void ChooseRunningMode();
   virtual void SubmitTask();
   virtual Bool_t CheckTaskVariables();
   Bool_t DoUserClassFilesExist();

   void SetBatchMode(Bool_t on = kTRUE)
   {
      fBatch = on;
   }
   Bool_t BatchMode() const
   {
      return fBatch;
   }
   void SetBatchName(const Char_t* batchname)
   {
      fBatchName = batchname;
   }
   const Char_t* GetBatchName()
   {
      return fBatchName.Data();
   }
   void SetBatchSystem(KVBatchSystem* bs)
   {
      fBatchSystem = bs;
      fChoseRunMode = kTRUE;
      if (bs) fBatchSystem->cd();
   }
   virtual const KVBatchSystem* GetBatchSystem()
   {
      return fBatchSystem;
   }
   Bool_t RunningInLaunchDirectory();

   const Char_t* GetLaunchDirectory() const;
   const Char_t* GetBatchStatusFileName() const;
   void UpdateBatchStatusFile(Int_t totev, Int_t evread, TString disk) const;
   void DeleteBatchStatusFile() const;
   virtual Long64_t GetTotalEntriesToRead() const
   {
      return 0;
   }
   virtual Bool_t CheckStatusUpdateInterval(Long64_t nevents) const;
   void SetStatusUpdateInterval(double t)
   {
      fStatusUpdateInterval = t;
   }
   Double_t GetStatusUpdateInterval() const
   {
      return fStatusUpdateInterval;
   }

   void DoStatusUpdate(Long64_t nevents) const;

   KVDataAnalysisTask* GetAnalysisTask() const
   {
      return fTask;
   }
   const KVString& GetDataType() const
   {
      return fDataType;
   }
   const KVString& GetUserIncludes() const
   {
      return fIncludes;
   }
   const KVString& GetUserLibraries() const
   {
      return fLibraries;
   }
   virtual void SetFileList(TList*)
   {
      AbstractMethod("SetFileList(TList*)");
   }
   virtual void SetAuxFileList(TList*)
   {
      AbstractMethod("SetAuxFileList(TList*)");
   }

   void SetNbEventToRead(Long64_t nb = 0)
   {
      nbEventToRead = nb;
   }

   Long64_t GetNbEventToRead(void) const
   {
      return nbEventToRead;
   }

   void ChooseNbEventToRead();
   void SetAnalysisTask(KVDataAnalysisTask* at);
   void SetDataType(const Char_t* name)
   {
      fDataType = name;
   }

   void SetUserIncludes(const Char_t* incDirs = 0);
   void SetUserLibraries(const Char_t* libs = 0);

   virtual void Reset();

   void SetParent(KVDataAnalyser* da)
   {
      fParent = da;
   }
   KVDataAnalyser* GetParent() const
   {
      return fParent;
   }

   const KVString& GetUserClassImp() const
   {
      return fUserClassImp;
   }
   const KVString& GetUserClassDec() const
   {
      return fUserClassDec;
   }

   static KVDataAnalyser* GetAnalyser(const Char_t* plugin);
   virtual Bool_t ReadBatchEnvFile(const Char_t*);
   virtual void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);

   virtual TString ExpandAutoBatchName(const Char_t* format) const;

   /* methods which can be used to customise analysis class behaviour.
      they can be called from the analysis class using the global
      pointer to the current data analyser, gDataAnalyser  */
   virtual void preInitAnalysis() {}
   virtual void postInitAnalysis() {}
   virtual void preInitRun() {}
   virtual void postInitRun() {}
   virtual void preAnalysis() {}
   virtual void postAnalysis() {}
   virtual void preEndRun() {}
   virtual void postEndRun() {}
   virtual void preEndAnalysis() {}
   virtual void postEndAnalysis() {}

   void WriteBatchInfo(TTree*);

   virtual void RegisterUserClass(TObject*) {}

   void SetUserClassOptions(const Char_t* o = "")
   {
      fUserClassOptions = o;
   }
   const KVString& GetUserClassOptions() const
   {
      return fUserClassOptions;
   }

   static void RunAnalyser(const Char_t* plugin = "");

   static Bool_t IsRunningBatchAnalysis();

   virtual void AddJobDescriptionList(TList*);

   void SetMenus(Bool_t on = kTRUE)
   {
      fMenus = on;
   }
   Bool_t IsMenus() const
   {
      return fMenus;
   }
   void SetQuit(Bool_t yes = kTRUE)
   {
      fQuit = yes;
   }
   Bool_t IsQuit() const
   {
      return fQuit;
   }
   void SetSubmit(Bool_t yes = kTRUE)
   {
      fSubmit = yes;
   }
   Bool_t IsSubmit() const
   {
      return fSubmit;
   }
   virtual KVString GetRootDirectoryOfDataToAnalyse() const
   {
      AbstractMethod("GetRootDirectoryOfDataToAnalyse");
      return "";
   }
   virtual const Char_t* GetRecognisedAutoBatchNameKeywords() const;
   void UseBaseClassSubmitTask()
   {
      fUseBaseClassSubmitTask = kTRUE;
   }
   virtual const KV2Body* GetKinematics() const
   {
      return nullptr;
   }
   virtual KVDBSystem* GetAnalysedSystem() const
   {
      return nullptr;
   }
   virtual KVDBRun* GetAnalysedRun() const
   {
      return nullptr;
   }
 #ifdef USING_ROOT6
   virtual void SetTriggerConditionsForRun(int) {}
#endif
  ClassDef(KVDataAnalyser, 0)  //For submitting & performing data analysis tasks
};

//................  global variable
R__EXTERN KVDataAnalyser* gDataAnalyser;

#endif
