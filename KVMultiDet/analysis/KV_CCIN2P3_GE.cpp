//Created by KVClassFactory on Wed Apr 27 15:43:08 CEST 2011
//Author: John Frankland

#include "KV_CCIN2P3_GE.h"
#include "TSystem.h"
#include "TEnv.h"
#include "KVDataAnalyser.h"
#include "KVDataAnalysisTask.h"
#include "KVGEBatchJob.h"
#include "KVDataRepository.h"
#include "KVDataSetAnalyser.h"
#include "KVSimDirAnalyser.h"

using namespace std;

ClassImp(KV_CCIN2P3_GE)

KV_CCIN2P3_GE::KV_CCIN2P3_GE(const Char_t* name)
   : KVBatchSystem(name), fMultiJobs(kTRUE)
{
   //Default constructor
   //Sets default job time, memory and disk space as defined in $KVROOT/KVFiles/.kvrootrc

   fDefJobTime = gEnv->GetValue("GE.BatchSystem.DefaultJobTime", "5:00");
   fDefJobMem = gEnv->GetValue("GE.BatchSystem.DefaultJobMemory", "2G");
   fDefJobDisk = gEnv->GetValue("GE.BatchSystem.DefaultJobDisk", "50M");
   fTimeSet = fDiskSet = fMemSet = kFALSE;
   //default number of runs per job in multi jobs mode (default=1)
   SetRunsPerJob(gEnv->GetValue("GE.BatchSystem.RunsPerJob", 1));
}

//_______________________________________________________________________________//

void KV_CCIN2P3_GE::Clear(Option_t* opt)
{
   //Clear previously set parameters in order to create a new job submission command
   KVBatchSystem::Clear(opt);
   fTimeSet = fDiskSet = fMemSet = kFALSE;
   fMultiJobs = kTRUE;
}

//_______________________________________________________________________________//

KV_CCIN2P3_GE::~KV_CCIN2P3_GE()
{
   //Destructor
}

void KV_CCIN2P3_GE::SetJobTime(const Char_t* time)
{
   //Set CPU time for batch job.
   //      SetJobTime() => use default time
   KVString tmp(time);
   if (tmp == "") tmp = fDefJobTime;
   //time given as either "hh:mm:ss" or "ss" but NOT "mm:ss"!
   if (tmp.GetNValues(":") == 2) tmp.Prepend("0:");
   fParList.SetValue("-l ct=", tmp);
   fTimeSet = kTRUE;
}

void KV_CCIN2P3_GE::SetJobMemory(const Char_t* mem)
{
   //Set maximum memory used by job.
   //Include units in string, i.e. "100M", "1G" etc.
   //If mem="", use default value
   KVString tmp(mem);
   if (tmp == "") tmp = fDefJobMem;
   fParList.SetValue("-l vmem=", tmp);
   fMemSet = kTRUE;
}

void KV_CCIN2P3_GE::SetJobDisk(const Char_t* diks)
{
   //Set maximum disk space used by job.
   //Include units in string, i.e. "100M", "1G" etc.
   //If diks="", use default value
   KVString tmp(diks);
   if (tmp == "") tmp = fDefJobDisk;
   fParList.SetValue("-l fsize=", tmp);
   fDiskSet = kTRUE;
}

void KV_CCIN2P3_GE::PrintJobs(Option_t*)
{
   //Print list of owner's jobs.
   KVList* j = GetListOfJobs();
   j->ls();
   delete j;
}

Bool_t KV_CCIN2P3_GE::CheckJobParameters()
{
   // Checks the job and asks for any missing parameters

   if (!KVBatchSystem::CheckJobParameters()) return kFALSE;

   if (!fTimeSet) ChooseJobTime();

   if (!fDiskSet) ChooseJobDisk();

   if (!fMemSet) ChooseJobMemory();

   return kTRUE;
}

void KV_CCIN2P3_GE::ChooseJobTime()
{
   KVString tmp = "";
   cout << "Enter max CPU time per job (ss/mn:ss/hh:mn:ss) ["
        << fDefJobTime << "] : ";
   cout.flush();
   tmp.ReadToDelim(cin);
   if (!tmp.Length()) {
      SetJobTime();
      return;
   }
   else
      SetJobTime(tmp);
}

void KV_CCIN2P3_GE::ChooseJobMemory()
{
   KVString tmp = "";
   cout << "Enter max memory per job (xKB/xMB/xGB) ["
        << fDefJobMem.Data() << "] : ";
   cout.flush();
   tmp.ReadToDelim(cin);
   SetJobMemory(tmp.Data());
}

void KV_CCIN2P3_GE::ChooseJobDisk()
{
   KVString tmp = "";
   cout << "Enter max scratch disk per job (xKB/xMB/xGB) ["
        << fDefJobDisk.Data() << "] : ";
   cout.flush();
   tmp.ReadToDelim(cin);
   SetJobDisk(tmp.Data());
}

const Char_t* KV_CCIN2P3_GE::GetJobTime(void) const
{
// returns the parameter string corresponding to the job CPU time
   return fParList.GetStringValue("-l ct=");
}

const Char_t* KV_CCIN2P3_GE::GetJobMemory(void) const
{
// returns the parameter string corresponding to the job Memory
   return fParList.GetStringValue("-l vmem=");
}

const Char_t* KV_CCIN2P3_GE::GetJobDisk(void) const
{
// returns the parameter string corresponding to the job Disk
   return fParList.GetStringValue("-l fsize=");
}

//_______________________________________________________________________________//

void KV_CCIN2P3_GE::WriteBatchEnvFile(TEnv* env)
{
   //Store any useful information on batch system in the TEnv
   //(this method is used by KVDataAnalyser::WriteBatchEnvFile)
   KVBatchSystem::WriteBatchEnvFile(env);
   env->SetValue("BatchSystem.MultiJobs", MultiJobsMode());
   if (MultiJobsMode()) env->SetValue("BatchSystem.CurrentRunList", fCurrJobRunList.AsString());
   env->SetValue("BatchSystem.Time", GetJobTime());
   env->SetValue("BatchSystem.Memory", GetJobMemory());
   env->SetValue("BatchSystem.Disk", GetJobDisk());
   // if analysis of simulated data is being used, we copy the files to analyse to the
   // scratch disk of the batch job (make sure enough disk space is requested)
   env->SetValue("SimDirAnalyser.CopyFilesToWorkingDirectory", true);
}

//_______________________________________________________________________________//

void KV_CCIN2P3_GE::ReadBatchEnvFile(TEnv* env)
{
   //Read any useful information on batch system from the TEnv
   //(this method is used by KVDataAnalyser::ReadBatchEnvFile)
   KVBatchSystem::ReadBatchEnvFile(env);
   SetMultiJobsMode(env->GetValue("BatchSystem.MultiJobs", kFALSE));
   if (MultiJobsMode()) fCurrJobRunList.SetList(env->GetValue("BatchSystem.CurrentRunList", ""));
   SetJobTime(env->GetValue("BatchSystem.Time", ""));
   SetJobMemory(env->GetValue("BatchSystem.Memory", ""));
   SetJobDisk(env->GetValue("BatchSystem.Disk", ""));
}

//_______________________________________________________________________________//

void KV_CCIN2P3_GE::Print(Option_t* option) const
{
   //if option="log", print infos for batch log file
   //if option="all", print detailed info on batch system
   if (!strcmp(option, "log")) {
      KVBatchSystem::Print(option);
      cout << "* DISK_REQ:                 " << GetJobDisk() << "                            *" << endl;
      cout << "* MEM_REQ:         " << GetJobMemory() << "                             *" << endl;
   }
   else
      KVBatchSystem::Print(option);
}

//_______________________________________________________________________________//

void KV_CCIN2P3_GE::ChangeDefJobOpt(KVDataAnalyser* da)
{
   // PRIVATE method called by SubmitTask() at moment of job submission.
   // Depending on the current environment, the default job submission options
   // may be changed by this method.
   //
   // This method overrides and augments KVBatchSystem::ChangeDefJobOpt (which
   // changes the options as a function of the type of analysis task).
   // Here we add the CCIN2P3-specific case where the job is launched from a directory
   // on the /sps/ semi-permanent storage facility, or if the data being analysed is
   // stored in a repository on /sps/. In this case we need to add
   // the option '-l u_sps_indra' to the 'qsub' command (if not already in the
   // default job options)
   //
   KVBatchSystem::ChangeDefJobOpt(da);
   KVString taskname = da->GetAnalysisTask()->GetName();
   KVString rootdir = da->GetRootDirectoryOfDataToAnalyse();
   Bool_t repIsSPS = rootdir.BeginsWith("/sps/");

   KVString wrkdir(gSystem->WorkingDirectory());
   KVString oldoptions(GetDefaultJobOptions());

   if (!oldoptions.Contains("sps")) {
      Bool_t NeedToAddSPS = wrkdir.Contains("/sps/");
      if ((NeedToAddSPS || repIsSPS)) {
         oldoptions += " -l sps=1";
         SetDefaultJobOptions(oldoptions.Data());
         Info("ChangeDefJobOpt",
              "Your job is being launched from /sps/... zone.\nTherefore the ressource 'sps' has been declared and the number of jobs which can be treated concurrently will be limited.");
      }
   }
}


void KV_CCIN2P3_GE::SanitizeJobName() const
{
   // Batch-system dependent sanitization of jobnames
   // Grid Engine does not allow:
   //   :
   // Any such character appearing in the current jobname will be replaced
   // with '_'

   fCurrJobName.ReplaceAll(":", "_");
}

KVList* KV_CCIN2P3_GE::GetListOfJobs()
{
   // Create and fill list with KVBatchJob objects describing current jobs
   // Delete list after use

   KVList* list_of_jobs = new KVList;

   // use qstat -r to get list of job ids and jobnames
   TString reply = gSystem->GetFromPipe("qstat -r");

   TObjArray* lines = reply.Tokenize("\n");
   Int_t nlines = lines->GetEntries();
   for (Int_t line_number = 0; line_number < nlines; line_number++) {
      TString thisLine = ((TObjString*)(*lines)[line_number])->String();
      if (thisLine.Contains("Full jobname:")) {
         // previous line contains job-id and status
         TString lastLine = ((TObjString*)(*lines)[line_number - 1])->String();
         TObjArray* bits = lastLine.Tokenize(" ");
         Int_t jobid = ((TObjString*)(*bits)[0])->String().Atoi();
         TString status = ((TObjString*)(*bits)[4])->String();
         // date & time jobs started (running job) or submitted (queued job)
         TString sdate = ((TObjString*)(*bits)[5])->String();// mm/dd/yyyy
         TString stime = ((TObjString*)(*bits)[6])->String();// hh:mm:ss
         Int_t dd, MM, yyyy, hh, mm, ss;
         sscanf(sdate.Data(), "%d/%d/%d", &MM, &dd, &yyyy);
         sscanf(stime.Data(), "%d:%d:%d", &hh, &mm, &ss);
         KVDatime submitted(yyyy, MM, dd, hh, mm, ss);
         delete bits;
         bits = thisLine.Tokenize(": ");
         TString jobname = ((TObjString*)(*bits)[2])->String();
         delete bits;

         KVGEBatchJob* job = new KVGEBatchJob();
         job->SetName(jobname);
         job->SetJobID(jobid);
         job->SetStatus(status);
         job->SetSubmitted(submitted);
         list_of_jobs->Add(job);
      }
   }
   delete lines;

   if (!list_of_jobs->GetEntries()) return list_of_jobs;

   // use qstat -j [jobid] to get cpu and memory used and also the resource requests
   TIter next_job(list_of_jobs);
   KVGEBatchJob* job;
   while ((job = (KVGEBatchJob*)next_job())) {

      // for running jobs, read in from [jobname].status file
      // the number of events read/to read, disk used
      if (!strcmp(job->GetStatus(), "r")) job->UpdateDiskUsedEventsRead();

      reply = gSystem->GetFromPipe(Form("qstat -j %d", job->GetJobID()));
      lines = reply.Tokenize("\n");
      nlines = lines->GetEntries();
      for (Int_t line_number = 0; line_number < nlines; line_number++) {
         TString thisLine = ((TObjString*)(*lines)[line_number])->String();
         if (thisLine.BeginsWith("usage")) {
            TObjArray* bits = thisLine.Tokenize("=,");
            TString stime = ((TObjString*)(*bits)[1])->String();// hh:mm:ss or d:hh:mm:ss
            Int_t dd, hh, mm, ss;
            TObjArray* tmp = stime.Tokenize(":");
            dd = 0;
            if (tmp->GetEntries() == 4) sscanf(stime.Data(), "%d:%2d:%2d:%2d", &dd, &hh, &mm, &ss);
            else sscanf(stime.Data(), "%2d:%2d:%2d", &hh, &mm, &ss);
            delete tmp;
            job->SetCPUusage((dd * 24 + hh) * 3600 + mm * 60 + ss);
            TString smem = ((TObjString*)(*bits)[7])->String();// xxx.xxxxM
            job->SetMemUsed(smem);
            delete bits;
         }
         else if (thisLine.BeginsWith("hard resource_list:")) {
            TObjArray* bits = thisLine.Tokenize(": ");
            TString res = ((TObjString*)(*bits)[2])->String();//os=sl5,xrootd=1,irods=1,s_vmem=1024M,s_fsize=50M,s_cpu=36000
            res.ReplaceAll("s_vmem", "vmem");
            res.ReplaceAll("s_fsize", "fsize");
            res.ReplaceAll("s_cpu", "ct");
            job->SetResources(res);
            TObjArray* bbits = res.Tokenize(",");
            TIter next_res(bbits);
            TObjString* ss;
            while ((ss = (TObjString*)next_res())) {
               TString g = ss->String();
               if (g.BeginsWith("ct=")) {
                  g.Remove(0, 3);
                  job->SetCPUmax(g.Atoi());
               }
               else if (g.BeginsWith("vmem=")) {
                  g.Remove(0, 5);
                  job->SetMemMax(g);
               }
               else if (g.BeginsWith("fsize=")) {
                  g.Remove(0, 6);
                  job->SetDiskMax(g);
               }
            }
            delete bits;
            delete bbits;
         }
      }
      delete lines;
      //}
   }

   return list_of_jobs;
}

void KV_CCIN2P3_GE::SetSendMailOnJobStart()
{
   // add option to send mail when job starts
   fParList.SetValue("-m b", "");
}

void KV_CCIN2P3_GE::SetSendMailOnJobEnd()
{
   // add option to send mail when job ends
   fParList.SetValue("-m e", "");
}

void KV_CCIN2P3_GE::SetSendMailAddress(const char* email)
{
   // set email address for notifications
   fParList.SetValue("-M ", email);
}

void KV_CCIN2P3_GE::Run()
{
   //Processes the job requests for the batch system.
   //In normal mode, this submits one job for the data analyser fAnalyser
   //In multijobs mode, this submits one job for each run in the runlist associated to fAnalyser

   if (!CheckJobParameters()) return;

   if (MultiJobsMode()) {
      if (fAnalyser->InheritsFrom("KVDataSetAnalyser")) {
         //submit jobs for every GetRunsPerJob() runs in runlist
         KVDataSetAnalyser* ana = dynamic_cast<KVDataSetAnalyser*>(fAnalyser);
         KVNumberList runs = ana->GetRunList();
         runs.Begin();
         Int_t remaining_runs = runs.GetNValues();
         fCurrJobRunList.Clear();
         while (remaining_runs && !runs.End()) {
            Int_t run = runs.Next();
            remaining_runs--;
            fCurrJobRunList.Add(run);
            if ((fCurrJobRunList.GetNValues() == GetRunsPerJob()) || runs.End()) {
               // submit job for GetRunsPerJob() runs (or less if we have reached end of runlist 'runs')
               ana->SetRuns(fCurrJobRunList, kFALSE);
               ana->SetFullRunList(runs);
               SubmitJob();
               fCurrJobRunList.Clear();
            }
         }
         ana->SetRuns(runs, kFALSE);
      }
      else if (fAnalyser->InheritsFrom("KVSimDirAnalyser")) {
         // here we understand "run" to mean "file"
         KVSimDirAnalyser* ana = dynamic_cast<KVSimDirAnalyser*>(fAnalyser);
         TList* file_list = ana->GetFileList();
         Int_t remaining_runs = ana->GetNumberOfFilesToAnalyse();
         fCurrJobRunList.Clear();
         TList cur_file_list;
         TObject* of;
         TIter it(file_list);
         Int_t file_no = 1;
         while ((of = it())) {
            cur_file_list.Add(of);
            fCurrJobRunList.Add(file_no);
            remaining_runs--;
            file_no++;
            if ((fCurrJobRunList.GetNValues() == GetRunsPerJob()) || (remaining_runs == 0)) {
               // submit job for GetRunsPerJob() files (or less if we have reached end of list)
               ana->SetFileList(&cur_file_list);
               SubmitJob();
               fCurrJobRunList.Clear();
               cur_file_list.Clear();
            }
         }
         ana->SetFileList(file_list);
      }
   }
   else {
      SubmitJob();
   }

}

void KV_CCIN2P3_GE::GetBatchSystemParameterList(KVNameValueList& nl)
{
   // Fill the list with all relevant parameters for batch system,
   // set to their default values.
   //
   // Parameters defined here are:
   //   JobTime        [string]
   //   JobMemory      [string]
   //   JobDisk        [string]
   //   MultiJobsMode  [bool]
   //   RunsPerJob     [int]
   //   EMailOnStart   [bool]
   //   EMailOnEnd     [bool]
   //   EMailAddress   [string]

   KVBatchSystem::GetBatchSystemParameterList(nl);
   nl.SetValue("JobTime", fDefJobTime);
   nl.SetValue("JobMemory", fDefJobMem);
   nl.SetValue("JobDisk", fDefJobDisk);
   nl.SetValue("MultiJobsMode", MultiJobsMode());
   nl.SetValue("RunsPerJob", fRunsPerJob);
   nl.SetValue("EMailOnStart", kFALSE);
   nl.SetValue("EMailOnEnd", kFALSE);
   nl.SetValue("EMailAddress", "");
}

void KV_CCIN2P3_GE::SetBatchSystemParameters(const KVNameValueList& nl)
{
   // Use the parameters in the list to set all relevant parameters for batch system.

   KVBatchSystem::SetBatchSystemParameters(nl);
   SetJobTime(nl.GetStringValue("JobTime"));
   SetJobMemory(nl.GetStringValue("JobMemory"));
   SetJobDisk(nl.GetStringValue("JobDisk"));
   SetMultiJobsMode(nl.GetBoolValue("MultiJobsMode"));
   SetRunsPerJob(nl.GetIntValue("RunsPerJob"));
   if (nl.GetTStringValue("EMailAddress") != "") {
      if (nl.GetBoolValue("EMailOnStart")) SetSendMailOnJobStart();
      if (nl.GetBoolValue("EMailOnEnd")) SetSendMailOnJobEnd();
      SetSendMailAddress(nl.GetStringValue("EMailAddress"));
   }
}
