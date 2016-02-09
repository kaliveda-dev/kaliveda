/*
$Id: KVBatchSystem.h,v 1.11 2008/04/03 07:35:45 franklan Exp $
$Revision: 1.11 $
$Date: 2008/04/03 07:35:45 $
*/

//Created by KVClassFactory on Thu Apr 13 13:07:59 2006
//Author: John Frankland

#ifndef __KVBATCHSYSTEM_H
#define __KVBATCHSYSTEM_H

#include "KVNameValueList.h"
#include "KVNumberList.h"
#include "KVString.h"
#include "KVBase.h"

class KVDataAnalyser;
class TEnv;
class TGWindow;
class KVList;
class KVBatchSystem : public KVBase {

protected:

   KVDataAnalyser* fAnalyser;//the analyser object which requests job submission, it has all details on the job
   KVNameValueList fParList;        //list of parameters/switches to be passed on job submission command line
   KVString fJobName;            //base job name
   KVString fJobSubCmd;          //shell command for submitting job
   KVString fJobScript;          //full path of shell script to be executed by batch system
   KVString fDefOpt;             //default options for job submission command
   Bool_t fMultiJobs;  //set to kTRUE if several jobs are to be submitted for the runlist set in fAnalyser
   Int_t fRunsPerJob; //number of runs per job submitted in multi job mode (default=1)
   KVString fCurrJobName; //name of current job being submitted
   KVNumberList fCurrJobRunList;//runlist for (multi job mode) job being submitted

   virtual void ChangeDefJobOpt(KVDataAnalyser* da);

public:

   KVBatchSystem(const Char_t* name);
   virtual ~ KVBatchSystem();

   KVNameValueList& GetParameters()
   {
      return fParList;
   };

   virtual const Char_t* GetJobSubCmd() const
   {
      return fJobSubCmd.Data();
   };
   virtual void SubmitTask(KVDataAnalyser* da);
   virtual void SubmitJob();
   virtual void PrintJobs(Option_t* /* opt */ = "")
   {
      ;
   }
   virtual Bool_t CheckJobParameters();

   virtual void Run();

   virtual void SetJobScript(const Char_t* path)
   {
      fJobScript = path;
   };

   virtual void SetDefaultJobOptions(const Char_t* opt)
   {
      //fDefOpt.Form(opt,gSystem->GetGroupInfo()->fGroup.Data());
      fDefOpt = opt;
   };
   virtual const Char_t* GetDefaultJobOptions() const
   {
      return fDefOpt.Data();
   };

   virtual void SetJobName(const Char_t* name);
   virtual const Char_t* GetJobName();

   virtual const Char_t* GetJobSubCmdLine();

   virtual void Clear(Option_t* opt = "");

   static KVBatchSystem* GetBatchSystem(const Char_t* plugin);

   void cd();

   virtual void SetMultiJobsMode(Bool_t on = kTRUE)
   {
      fMultiJobs = on;
   };
   virtual Bool_t MultiJobsMode() const
   {
      return fMultiJobs;
   };

   virtual void SetRunsPerJob(Int_t n)
   {
      // Set number of runs per job submitted in multi jobs mode (default=1)
      fRunsPerJob = n;
   };
   virtual Int_t GetRunsPerJob() const
   {
      // Returns number of runs per job submitted in multi jobs mode
      return fRunsPerJob;
   };

   virtual void WriteBatchEnvFile(TEnv*);
   virtual void ReadBatchEnvFile(TEnv*);
   virtual void Print(Option_t* /*option*/ = "") const;

   virtual void SetAnalyser(KVDataAnalyser* da)
   {
      fAnalyser = da;
   };

   virtual TString BQS_Request(KVString /*value*/, KVString /*jobname*/ = "")
   {

      Warning("BQS_Request", "Only define for KV_CCIN2P3_BQS sub class");
      return "";
   }
   virtual void SetJobTimeString(const Char_t* = "") {};
   virtual void SetJobTime(Int_t, Int_t = 0, Int_t = 0) {};
   virtual void SetJobMemory(const Char_t* = "") {};
   virtual void SetJobDisk(const Char_t* = "") {};

   virtual void SanitizeJobName() {}

   virtual KVList* GetListOfJobs();

   virtual void AlterJobs(TGWindow*, TList*)
   {
      ;
   }

   ClassDef(KVBatchSystem, 2)   //Base class for interface to batch job management system
};

//................  global variable
R__EXTERN KVBatchSystem* gBatchSystem;

#endif
