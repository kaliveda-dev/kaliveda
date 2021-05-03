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

/**
\class KVBatchSystem
\brief Base class for interface to a batch job management system
\ingroup Infrastructure

<p>The different "batch" systems available are described in the $KVROOT/KVFiles/.kvrootrc
or the user's $HOME/.kvrootrc, as in the following example:</p>
<pre>
BatchSystem:     Xterm
Xterm.BatchSystem.Title:    Execute task in an X-terminal window
Xterm.BatchSystem.DefaultJobOptions:   -T #JobName# -e
Xterm.BatchSystem.JobSubCmd:   xterm
Xterm.BatchSystem.JobScript: "$KVROOT/bin/KaliVedaAnalysis -b -n"&
</pre>
<p>For each named batch system, there may be a corresponding plugin which
defines the actual class to use, if it is necessary to extend the functionality of the
KVBatchSystem base class, as for example:</p>
<pre>
Plugin.KVBatchSystem:    Xterm    KVRootBatch     KVMultiDet    "KVRootBatch(const Char_t*)"
</pre>
<p>The "default" batch system is defined as follows:</p>
<pre>
#Default batch system
Default.BatchSystem:   Xterm
</pre>
<p>It can be accessed via the batch system manager (see <a href="KVBatchSystemManager.html">KVBatchSystemManager</a>):</p>

~~~~{.cpp}
...
#include "KVBatchSystemManager.h"
...
KVBatchSystem* default = gBatchSystemManager->GetDefaultBatchSystem();
~~~~

<h4>Access to batch parameters in user analysis class</h4>
<p>Global pointer gBatchSystem is 0 if analysis task is not run in batch mode
If it is non-zero, it gives access to the batch system used to run the task. Therefore, if the user
puts</p>
~~~~{.cpp}
#include "KVBatchSystem.h"
~~~~
<p>in her analysis code, she can then use gBatchSystem as follows:</p>
~~~~{.cpp}
if( gBatchSystem ) {
   //running in batch. name results file after job name.
   new TFile( Form( "%s.root", gBatchSystem->GetJobName() ), "recreate");
}
else {
   //not running in batch. fixed name.
   new TFile("Results.root", "recreate");
}
~~~~

 */
class KVBatchSystem : public KVBase {

protected:

   KVDataAnalyser* fAnalyser;//the analyser object which requests job submission, it has all details on the job
   KVNameValueList fParList;        //list of parameters/switches to be passed on job submission command line
   KVString fJobName;            //base job name
   KVString fJobSubCmd;          //shell command for submitting job
   KVString fJobScript;          //full path of shell script to be executed by batch system
   KVString fDefOpt;             //default options for job submission command
   mutable KVString fCurrJobName; //name of current job being submitted
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
   virtual const Char_t* GetJobName() const;

   virtual const Char_t* GetJobSubCmdLine();

   virtual void Clear(Option_t* opt = "");

   static KVBatchSystem* GetBatchSystem(const Char_t* plugin);

   void cd();

   virtual Bool_t MultiJobsMode() const
   {
      return kFALSE;
   }

   virtual void WriteBatchEnvFile(TEnv*);
   virtual void ReadBatchEnvFile(TEnv*);
   virtual void Print(Option_t* /*option*/ = "") const;

   virtual void SetAnalyser(KVDataAnalyser* da)
   {
      fAnalyser = da;
   }


   virtual void SanitizeJobName() const {}

   virtual KVList* GetListOfJobs();

   virtual void AlterJobs(TGWindow*, TList*)
   {
      ;
   }

   virtual void GetBatchSystemParameterList(KVNameValueList&);
   virtual void SetBatchSystemParameters(const KVNameValueList&);

   ClassDef(KVBatchSystem, 2)   //Base class for interface to batch job management system
};

//................  global variable
R__EXTERN KVBatchSystem* gBatchSystem;

#endif
