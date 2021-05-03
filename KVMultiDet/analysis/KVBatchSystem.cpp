/*
$Id: KVBatchSystem.cpp,v 1.12 2008/04/14 08:49:11 franklan Exp $
$Revision: 1.12 $
$Date: 2008/04/14 08:49:11 $
*/

//Created by KVClassFactory on Thu Apr 13 13:07:59 2006
//Author: John Frankland

#include "KVBatchSystem.h"
#include "KVBase.h"
#include "TEnv.h"
#include "TPluginManager.h"
#include "TSystem.h"
#include "KVDataAnalyser.h"
#include "KVDataAnalysisTask.h"

using namespace std;

ClassImp(KVBatchSystem)

KVBatchSystem* gBatchSystem = 0;

KVBatchSystem::KVBatchSystem(const Char_t* name)
   : KVBase(name), fAnalyser(nullptr)
{
   // Constructor with name of batch system. Name will be used to retrieve
   // resources from configuration file, e.g. batch system title, job submission
   // command, name of batch script, default job options, runs per job in multijob mode:
   //
   //   [batch system name].BatchSystem.Title:
   //   [batch system name].BatchSystem.JobSubCmd:
   //   [batch system name].BatchSystem.JobScript:
   //   [batch system name].BatchSystem.DefaultJobOptions:

   //set title of batch system
   SetTitle(gEnv->GetValue(Form("%s.BatchSystem.Title", name), ""));
   //command for job submission
   fJobSubCmd = gEnv->GetValue(Form("%s.BatchSystem.JobSubCmd", name), "");
   //check command is valid
   if (!KVBase::FindExecutable(fJobSubCmd)) {
      Warning("KVBatchSystem", "Batch system %s has unknown job submission command: %s",
              name, fJobSubCmd.Data());
   }
   //script for batch job
   SetJobScript(gEnv->GetValue(Form("%s.BatchSystem.JobScript", name), ""));
   //set default job options
   SetDefaultJobOptions(gEnv->GetValue(Form("%s.BatchSystem.DefaultJobOptions", name), ""));
}

KVBatchSystem::~KVBatchSystem()
{
   //Destructor
   if (gBatchSystem == this)
      gBatchSystem = 0;
}

const Char_t* KVBatchSystem::GetJobSubCmdLine()
{
   //Builds and returns static string containing full command line necessary to submit a batch
   //job. This is constructed as follows:
   //
   //      [JobSubCmd] [default options] [par1 val1] [par2 val2] ... [JobScript]
   //
   //The 'default options' can be set with SetDefaultJobOptions().
   //
   //The 'par1,val1' pairs are the named parameters held in the fParList KVParameterList.
   //For each parameter in the list, the name of the parameter is used for 'par1' and the value
   //of the parameter is used for 'val1' etc. etc.
   //e.g. if the job submission command requires an option to be passed with the following
   //syntax
   //      [JobSubCmd] -Noption
   //one of the parameters in the list should be called "-N" and its value should be 'option'
   //
   //In order to add a simple flag such as
   //      [JobSubCmd] -V
   //add a parameter called "-V" with value "".
   //
   //The special variable #JobName# can be used anywhere and will be replaced by the
   //jobname returned by GetJobName() at the moment of job submission.

   static TString command_line;
   command_line.Form("%s %s ", fJobSubCmd.Data(), fDefOpt.Data());
   if (fParList.GetNpar()) {
      for (int i = 0; i < fParList.GetNpar(); i++) {
         KVNamedParameter* par = fParList.GetParameter(i);
         command_line += par->GetName();
         if (par->GetTString() != "") {
            command_line += par->GetString();
         }
         command_line += " ";
      }
   }
   command_line += fJobScript;
   //replace #JobName# with name of current job
   command_line.ReplaceAll("#JobName#", GetJobName());
   return command_line.Data();
}

//_______________________________________________________________________________//

void KVBatchSystem::Clear(Option_t*)
{
   //Clear previously set parameters in order to create a new job submission command
   //(default options are not affected: use SetDefaultJobOptions to change them)

   fJobName = "";
   fParList.Clear();
   fAnalyser = nullptr;
}

//_______________________________________________________________________________//

void KVBatchSystem::cd()
{
   //Make this the default batch system
   gBatchSystem = this;
}

//_______________________________________________________________________________//

void KVBatchSystem::SubmitJob()
{
   //Submits a job to batch system, i.e. executes the string formed by GetJobSubCmdLine,
   //if all necessary parameters have been given (any missing ones will be asked for).
   //Parameters specific to a given batch system can be added my modifying the
   //CheckJobParameters() method for the associated child class.

   //set environment variables required by KaliVedaAnalysis batch executable
   gSystem->Setenv("KVBATCHNAME", GetJobName());
   gSystem->Setenv("KVLAUNCHDIR", gSystem->WorkingDirectory());
   cout << GetJobSubCmdLine() << endl;
   if (fAnalyser) {
      fAnalyser->WriteBatchEnvFile(GetJobName());
      gSystem->Setenv("KVANALYSER", fAnalyser->ClassName());
   }
   gSystem->Exec(GetJobSubCmdLine());
}

//_______________________________________________________________________________//

void KVBatchSystem::Run()
{
   //Processes the job requests for the batch system.
   //In normal mode, this submits one job for the data analyser fAnalyser
   //In multijobs mode, this submits one job for each run in the runlist associated to fAnalyser

   if (!CheckJobParameters()) return;

   SubmitJob();
}

//_______________________________________________________________________________//

KVBatchSystem* KVBatchSystem::GetBatchSystem(const Char_t* plugin)
{
   // Create batch system object defined as a plugin in .kvrootrc
   // If no plugin is found, we create a new KVBatchSystem base object which
   // will be initialised from resources defined in .kvrootrc using its name.

   //check and load plugin library
   TPluginHandler* ph = KVBase::LoadPlugin("KVBatchSystem", plugin);
   if (!ph)
      return new KVBatchSystem(plugin);

   //execute constructor/macro for multidetector
   return ((KVBatchSystem*) ph->ExecPlugin(1, plugin));
}

//_______________________________________________________________________________//

void KVBatchSystem::SubmitTask(KVDataAnalyser* da)
{
   // Submit data analysis task described by KVDataAnalyser object to the batch system.
   //
   // Note that the default options for this batch system may be changed just before
   // job submission depending on the current environment, see ChangeDefJobOpt().

   Info("SubmitTask", "Task submission for analyser class : %s", da->ClassName());
   SetAnalyser(da);
   //change job submission options depending on task, environment, etc.
   ChangeDefJobOpt(da);
   Run();
}

//_______________________________________________________________________________//

void KVBatchSystem::ChangeDefJobOpt(KVDataAnalyser* da)
{
   // PRIVATE method called by SubmitTask() at moment of job submission.
   // Depending on the current environment, the default job submission options
   // may be changed by this method.
   // For example, default options may be changed depending on the analysis
   // task to be performed. We look for an environment variable of the form:
   //
   // [batch system name].BatchSystem.DefaultJobOptions.[analysis task name]
   //
   // and if found we use the options defined by it. If not, we use the default
   // job options defined by
   // [batch system name].BatchSystem.DefaultJobOptions

   TString tmp = gEnv->GetValue(Form("%s.BatchSystem.DefaultJobOptions.%s",
                                     GetName(), da->GetAnalysisTask()->GetName()), "");
   if (tmp.Length()) {
      Info("ChangeDefJobOpt", "Changing default batch options for task %s.", da->GetAnalysisTask()->GetName());
      Info("ChangeDefJobOpt", "Batch job options for this job are : %s", tmp.Data());
      SetDefaultJobOptions(tmp.Data());
   }
   else {
      tmp = gEnv->GetValue(Form("%s.BatchSystem.DefaultJobOptions", GetName()), "");
      SetDefaultJobOptions(tmp.Data());
   }
}

//_______________________________________________________________________________//

const Char_t* KVBatchSystem::GetJobName() const
{
   //Returns name of batch job, either during submission of batch jobs or when an analysis
   //task is running in batch mode (access through gBatchSystem global pointer).
   //
   //In multi-job mode, the job name is generated from the base name set by SetJobName()
   //plus the extension "_Rxxxx-yyyy" with "xxxx" and "yyyy" the number of the first and last run
   //which will be analysed by the current job.
   //
   // Depending on the batch system, some sanitization of the jobname may be required
   // e.g. to remove "illegal" characters from the jobname. This is done by SanitizeJobName()
   // before the jobname is returned.

   if (!fAnalyser) {
      //stand-alone batch submission ?
      fCurrJobName = fJobName;
   }
   else {
      //replace any special symbols with their current values
      fCurrJobName = fAnalyser->ExpandAutoBatchName(fJobName.Data());
      if (MultiJobsMode() && !fAnalyser->BatchMode()) {
         KVString tmp;
         if (fCurrJobRunList.GetNValues() > 1)
            tmp.Form("_R%d-%d", fCurrJobRunList.First(), fCurrJobRunList.Last());
         else
            tmp.Form("_R%d", fCurrJobRunList.First());
         fCurrJobName += tmp;
      }
   }
   SanitizeJobName();
   return fCurrJobName.Data();
}

//_______________________________________________________________________________//

Bool_t KVBatchSystem::CheckJobParameters()
{
   // Checks the job and ask for the job name if needed
   KVString jobName = fJobName;
   while (!jobName.Length()) {
      cout << "Please enter the job name : ";
      cout.flush();
      jobName.ReadToDelim(cin);
      if (jobName.Length()) {
         SetJobName(jobName.Data());
      }
   }
   return kTRUE;
}

//_______________________________________________________________________________//

void KVBatchSystem::WriteBatchEnvFile(TEnv* env)
{
   //Store any useful information on batch system in the TEnv
   //(this method is used by KVDataAnalyser::WriteBatchEnvFile)
   env->SetValue("BatchSystem.JobName", GetJobName());
}

//_______________________________________________________________________________//

void KVBatchSystem::ReadBatchEnvFile(TEnv* env)
{
   //Read any useful information on batch system from the TEnv
   //(this method is used by KVDataAnalyser::ReadBatchEnvFile)
   fJobName = env->GetValue("BatchSystem.JobName", "");
}

//_______________________________________________________________________________//

void KVBatchSystem::Print(Option_t* option) const
{
   //if option="log", print infos for batch log file
   //if option="all", print detailed info on batch system
   if (!strcmp(option, "log")) {
      cout << "Job " << GetJobName()
           << " executed by batch system " << GetName() << endl;
   }
   else if (!strcmp(option, "all")) {
      cout << ClassName() << " : Name = " << GetName() << endl << " Title = " << GetTitle() << endl;
      cout << " fJobSubCmd = " << fJobSubCmd.Data() << endl;
      cout << " fJobScript = " << fJobScript.Data() << endl;
      cout << " fDefOpt = " << fDefOpt.Data() << endl;
      fParList.Print();        //list of parameters/switches to be passed on job submission command line
   }
   else
      KVBase::Print(option);
}

KVList* KVBatchSystem::GetListOfJobs()
{
   // Create and fill list with KVBatchJob objects, one for each job currently
   // handled by the batch system.
   //
   // Needs to be implemented for specific systems in child classes.
   // This method returns 0x0.
   return 0x0;
}

void KVBatchSystem::GetBatchSystemParameterList(KVNameValueList& nl)
{
   // Fill the list with all relevant parameters for batch system,
   // set to their default values.
   //
   // Parameters defined here are:
   //   JobName [string]
   //   AutoJobName [bool]
   //   AutoJobNameFormat [string]

   nl.Clear();
   nl.SetTitle(GetTitle());
   nl.SetValue("JobName", "");
   nl.SetValue("AutoJobName", kTRUE);
   nl.SetValue("AutoJobNameFormat", "$UserClass");
}

void KVBatchSystem::SetBatchSystemParameters(const KVNameValueList& nl)
{
   // Use the parameters in the list to set all relevant parameters for batch system.

   if (nl.GetBoolValue("AutoJobName"))
      SetJobName(nl.GetStringValue("AutoJobNameFormat"));
   else
      SetJobName(nl.GetStringValue("JobName"));
   Info("SetBatchSystemParameters", "JobName = %s", GetJobName());
}

//_______________________________________________________________________________//

void KVBatchSystem::SetJobName(const Char_t* name)
{
   //Set the job name. In MultiJobsMode this will be used as the base name for all jobs;
   //each individual job will have the name 'basejobname_Rxxxx", with xxxx=run number for job.
   //
   //The job name can be generated automatically by replacing certain special symbols
   //in the name given here depending on the characteristics of the job. See
   //KVDataAnalyser::ExpandAutoBatchName for allowed symbols.
   fJobName = name;
}
