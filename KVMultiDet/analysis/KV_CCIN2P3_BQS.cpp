/*
$Id: KV_CCIN2P3_BQS.cpp,v 1.13 2008/04/14 08:49:37 franklan Exp $
$Revision: 1.13 $
$Date: 2008/04/14 08:49:37 $
*/

//Created by KVClassFactory on Thu Apr 13 13:08:06 2006
//Author: John Frankland

#include "KV_CCIN2P3_BQS.h"
#include "TSystem.h"
#include "TEnv.h"
#include "KVDataAnalyser.h"
#include "KVDataAnalysisTask.h"

using namespace std;

ClassImp(KV_CCIN2P3_BQS)
////////////////////////////////////////////////////////////////////////////////
// Interface to CCIN2P3 batch job management system
////////////////////////////////////////////////////////////////////////////////
KV_CCIN2P3_BQS::KV_CCIN2P3_BQS(const Char_t* name)
   : KVBatchSystem(name)
{
   //Default constructor
   //Sets default job time, memory and disk space as defined in $KVROOT/KVFiles/.kvrootrc

   fDefJobTime = gEnv->GetValue("BQS.BatchSystem.DefaultJobTime", 4000);
   fDefJobMem = gEnv->GetValue("BQS.BatchSystem.DefaultJobMemory", "256MB");
   fDefJobDisk = gEnv->GetValue("BQS.BatchSystem.DefaultJobDisk", "100MB");
   fTimeSet = fDiskSet = fMemSet = kFALSE;
}

//_______________________________________________________________________________//

void KV_CCIN2P3_BQS::Clear(Option_t* opt)
{
   //Clear previously set parameters in order to create a new job submission command
   KVBatchSystem::Clear(opt);
   fTimeSet = fDiskSet = fMemSet = kFALSE;
}

//_______________________________________________________________________________//

KV_CCIN2P3_BQS::~KV_CCIN2P3_BQS()
{
   //Destructor
}

void KV_CCIN2P3_BQS::SetJobTime(Int_t ss, Int_t mm, Int_t hh)
{
   //Set CPU time for batch job.
   //      SetJobTime(0) => use default time
   //      SetJobTime(100) => 100 seconds
   //      SetJobTime(30,2)        => 2 minutes 30 seconds
   //      SetJobTime(0,30,5)      => 5 hours 30 minutes (0 seconds)

   KVString tmp = "";
   if (hh)
      tmp.Form("%d:%02d:%02d", hh, mm, ss);
   else if (mm)
      tmp.Form("%d:%02d", mm, ss);
   else if (ss)
      tmp.Form("%d", ss);
   SetJobTimeString(tmp.Data());
}

void KV_CCIN2P3_BQS::SetJobTimeString(const Char_t* time)
{
   //Set CPU time for batch job.
   //      SetJobTime() => use default time
   KVString tmp(time);
   if (tmp == "") tmp = Form("%d", fDefJobTime);
   fParList.SetValue("-l T=", tmp);
   fTimeSet = kTRUE;
}

void KV_CCIN2P3_BQS::SetJobMemory(const Char_t* mem)
{
   //Set maximum memory used by job.
   //Include units in string, i.e. "100MB", "1GB" etc.
   //If mem="", use default value
   KVString tmp(mem);
   if (tmp == "") tmp = fDefJobMem;
   fParList.SetValue("-l M=", tmp);
   fMemSet = kTRUE;
}

void KV_CCIN2P3_BQS::SetJobDisk(const Char_t* diks)
{
   //Set maximum disk space used by job.
   //Include units in string, i.e. "100MB", "1GB" etc.
   //If diks="", use default value
   KVString tmp(diks);
   if (tmp == "") tmp = fDefJobDisk;
   fParList.SetValue("-l scratch=", tmp);
   fDiskSet = kTRUE;
}

void KV_CCIN2P3_BQS::PrintJobs(Option_t* opt)
{
   //Print list of owner's jobs.
   //opt="" (default) : print all jobs
   //opt="r[unning]" : print running jobs
   //opt="q[ueued]" : print queued jobs
   KVString _opt(opt), cmd("qjob");
   _opt.ToUpper();
   if (_opt.BeginsWith("R"))
      cmd += " -r";
   else if (_opt.BeginsWith("Q"))
      cmd += " -q";
   gSystem->Exec(cmd.Data());
}

Bool_t KV_CCIN2P3_BQS::CheckJobParameters()
{
   // Checks the job and asks for any missing parameters

   if (!KVBatchSystem::CheckJobParameters()) return kFALSE;

   if (!fTimeSet) ChooseJobTime();

   if (!fDiskSet) ChooseJobDisk();

   if (!fMemSet) ChooseJobMemory();

   return kTRUE;
}

void KV_CCIN2P3_BQS::ChooseJobTime()
{
   KVString tmp = "";
   cout << "Enter max CPU time per job (ss/mn:ss/hh:mn:ss) ["
        << fDefJobTime << "] : ";
   cout.flush();
   tmp.ReadToDelim(cin);
   if (!tmp.Length()) {
      SetJobTimeString();
      return;
   }
   Int_t sec = tmp.Atoi();
   Int_t i = 0;
   while ((i = tmp.Index(":")) > 0) {
      sec *= 60;
      tmp.Remove(0, i + 1);
      sec += tmp.Atoi();
   }
   SetJobTime(sec);
}

void KV_CCIN2P3_BQS::ChooseJobMemory()
{
   KVString tmp = "";
   cout << "Enter max memory per job (xKB/xMB/xGB) ["
        << fDefJobMem.Data() << "] : ";
   cout.flush();
   tmp.ReadToDelim(cin);
   SetJobMemory(tmp.Data());
}

void KV_CCIN2P3_BQS::ChooseJobDisk()
{
   KVString tmp = "";
   cout << "Enter max scratch disk per job (xKB/xMB/xGB) ["
        << fDefJobDisk.Data() << "] : ";
   cout.flush();
   tmp.ReadToDelim(cin);
   SetJobDisk(tmp.Data());
}

const Char_t* KV_CCIN2P3_BQS::GetJobTime(void)
{
// returns the parameter string corresponding to the job CPU time
   return fParList.GetStringValue("-l T=");
}

const Char_t* KV_CCIN2P3_BQS::GetJobMemory(void)
{
// returns the parameter string corresponding to the job Memory
   return fParList.GetStringValue("-l M=");
}

const Char_t* KV_CCIN2P3_BQS::GetJobDisk(void)
{
// returns the parameter string corresponding to the job Disk
   return fParList.GetStringValue("-l scratch=");
}

//_______________________________________________________________________________//

void KV_CCIN2P3_BQS::WriteBatchEnvFile(TEnv* env)
{
   //Store any useful information on batch system in the TEnv
   //(this method is used by KVDataAnalyser::WriteBatchEnvFile)
   KVBatchSystem::WriteBatchEnvFile(env);
   env->SetValue("BatchSystem.Time", GetJobTime());
   env->SetValue("BatchSystem.Memory", GetJobMemory());
   env->SetValue("BatchSystem.Disk", GetJobDisk());
}

//_______________________________________________________________________________//

void KV_CCIN2P3_BQS::ReadBatchEnvFile(TEnv* env)
{
   //Read any useful information on batch system from the TEnv
   //(this method is used by KVDataAnalyser::ReadBatchEnvFile)
   KVBatchSystem::ReadBatchEnvFile(env);
   SetJobTimeString(env->GetValue("BatchSystem.Time", ""));
   SetJobMemory(env->GetValue("BatchSystem.Memory", ""));
   SetJobDisk(env->GetValue("BatchSystem.Disk", ""));
}

//_______________________________________________________________________________//

void KV_CCIN2P3_BQS::Print(Option_t* option) const
{
   //if option="log", print infos for batch log file
   //if option="all", print detailed info on batch system
   if (!strcmp(option, "log")) {
      KVBatchSystem::Print(option);
      cout << "* DISK_REQ:                 " << const_cast<KV_CCIN2P3_BQS*>(this)->GetJobDisk() << "                            *" << endl;
      cout << "* MEM_REQ:         " << const_cast<KV_CCIN2P3_BQS*>(this)->GetJobMemory() << "                             *" << endl;
   } else
      KVBatchSystem::Print(option);
}

//_______________________________________________________________________________//

void KV_CCIN2P3_BQS::ChangeDefJobOpt(KVDataAnalyser* da)
{
   // PRIVATE method called by SubmitTask() at moment of job submission.
   // Depending on the current environment, the default job submission options
   // may be changed by this method.
   //
   // This method overrides and augments KVBatchSystem::ChangeDefJobOpt (which
   // changes the options as a function of the type of analysis task).
   // Here we add the CCIN2P3-specific case where the job is launched from a directory
   // on the /sps/ semi-permanent storage facility. In this case we need to add
   // the option '-l u_sps_indra' to the 'qsub' command (if not already in the
   // default job options)
   //
   // Due to many people being caught out by this mechanism when submitting
   // raw->recon, raw->ident, etc. jobs from an SPS directory (and thus being penalised
   // unfairly by the limited number of SPS-ressource-declaring jobs), we only declare
   // u_sps_indra if the analysis task is not "Reconstruction", "Identification1",
   // "RawIdent", or "Identification2". We also add some warning messages.

   KVBatchSystem::ChangeDefJobOpt(da);
   KVString taskname = da->GetAnalysisTask()->GetName();
   Bool_t recId = (taskname == "Reconstruction" || taskname == "Identification1" || taskname == "RawIdent" || taskname == "Identification2");
   KVString wrkdir(gSystem->WorkingDirectory());
   KVString oldoptions(GetDefaultJobOptions());
   if (wrkdir.Contains("/sps/") && !oldoptions.Contains("u_sps_indra")) {
      if (recId) {
         // submitting recon/ident job from /sps directory. do not add u_sps_indra ressource
         Warning("ChangeDefJobOpt",
                 "Your job is being submitted from %s.\nHowever, for reconstruction/identification tasks, we do not declare the 'u_sps_indra' ressource, so that the number of jobs which can be treated concurrently will not be limited.",
                 wrkdir.Data());
      } else {
         oldoptions += " -l u_sps_indra";
         SetDefaultJobOptions(oldoptions.Data());
         Warning("ChangeDefJobOpt",
                 "Your job is being submitted from %s.\nTherefore the ressource 'u_sps_indra' has been declared and the number of jobs which can be treated concurrently will be limited.",
                 wrkdir.Data());
      }
   }
}

//_______________________________________________________________________________//
TString KV_CCIN2P3_BQS::BQS_Request(KVString value, KVString jobname)
{
   //Permet d interroger un job vis la commande BQS qselect
   //sur differentes grandeurs (qselect -list -v)
   /*
   //Ici les commandes les plus utiles actuellement
   //
   bastacputime   //temps ecoule depuis le start
   cpu_limit   //temps max
   cpurate     //temps CPU

   req_scratch             requested scratch size (MB)
   cur_scratch             current scratch size (MB)
   max_scratch             max scratch size (MB)

   req_mem                 requested memory size (MB)
   cur_mem                 current memory size (MB)
   max_mem                 max memory size (MB)
   */

   if (jobname == "")
      jobname.Form("%s", GetJobName());
   KVString inst;
   inst.Form("qselect -N %s %s", jobname.Data(), value.Data());
   return gSystem->GetFromPipe(inst.Data());

}
