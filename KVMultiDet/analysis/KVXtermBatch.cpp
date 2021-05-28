#include "KVDataSetAnalyser.h"
#include "KVXtermBatch.h"
#include <cassert>

void KVXtermBatch::Run()
{
   //Processes the job requests for the batch system.
   //In multijobs mode, this submits one job for each run in the runlist associated to fAnalyser

   if (!CheckJobParameters()) return;

   //submit jobs for every GetRunsPerJob() runs in runlist
   KVDataSetAnalyser* ana = dynamic_cast<KVDataSetAnalyser*>(fAnalyser);
   assert(ana);
   KVNumberList runs = ana->GetRunList();
   Int_t remaining_runs = runs.GetNValues();

   // to optimize use of CPUs, some jobs will have multiple runs
   std::vector<int> runs_per_job(WITH_MULTICORE_CPU);
   int job_index = 0;
   runs.Begin();
   while (!runs.End()) {
      runs.Next();
      ++runs_per_job[job_index++];
      if (job_index == WITH_MULTICORE_CPU) job_index = 0;
   }

   fCurrJobRunList.Clear();
   runs.Begin();
   job_index = 0;
   while (remaining_runs && !runs.End()) {
      Int_t run = runs.Next();
      remaining_runs--;
      fCurrJobRunList.Add(run);
      if ((fCurrJobRunList.GetNValues() == runs_per_job[job_index]) || runs.End()) {
         // submit job for run
         ana->SetRuns(fCurrJobRunList, kFALSE);
         ana->SetFullRunList(runs);
         std::cout << "Submitting job " << job_index + 1 << " for runs:" << fCurrJobRunList.AsString() << std::endl;
         SubmitJob();
         fCurrJobRunList.Clear();
         ++job_index;
      }
   }
   ana->SetRuns(runs, kFALSE);
}

ClassImp(KVXtermBatch)


