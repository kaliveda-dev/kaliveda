//Created by KVClassFactory on Wed Apr  3 14:15:11 2013
//Author: John Frankland,,,

#include "KVBatchJob.h"
#include "TEnv.h"
#include "Riostream.h"
using namespace std;

ClassImp(KVBatchJob)

KVBatchJob::KVBatchJob()
{
   // Default constructor
   cpu_used = 0;
   mem_used = "-";
   cpu_max = 0;
   events_total = 0;
   events_read = 0;
}


KVBatchJob::~KVBatchJob()
{
   // Destructor
}

void KVBatchJob::ls(Option_t*) const
{
   cout << GetJobID() << " " << GetName() << " " << GetStatus() << " " << GetSubmitted().AsSQLString();
   if (!strcmp(GetStatus(), "r")) cout << " " << GetCPUusage() << " " << GetMemUsed();
   cout << endl;
}

void KVBatchJob::UpdateDiskUsedEventsRead()
{
   // We look for a TEnv file in the user's $HOME directory
   // with the name [jobname].status
   TEnv stats(Form("%s.status", GetName()));
   if (!stats.Defined("TotalEvents")) {
      // file does not exist: job not running yet
      return;
   }
   SetEventsTotal(stats.GetValue("TotalEvents", 0));
   SetEventsRead(stats.GetValue("EventsRead", 0));
   SetDiskUsed(stats.GetValue("DiskUsed", ""));
}
