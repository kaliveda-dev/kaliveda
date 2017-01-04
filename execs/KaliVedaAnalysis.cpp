//
// Standalone program launching the KVDataAnalysisLauncher
//

#include "TApplication.h"
#include "Riostream.h"
#include "TSystem.h"
#include "KVBase.h"
#include "KVDataRepositoryManager.h"
#include "KVDataAnalyser.h"
#include <csignal>     /* signal, raise, sig_atomic_t */
#include <cstdio>
extern "C"
{
   void xcpu_signal_handler(int)
   {
      printf("Process received XCPU signal -- analysis will be aborted...\n");
      KVDataAnalyser::SetAbortProcessingLoop(kTRUE);
   }
}

using namespace std;

int main(int argc, char* argv[])
{
   //Set environment variable KVBATCHNAME with name of batch job before running
   //Set environment variable KVLAUNCHDIR with launch directory path before running

   signal(SIGXCPU, xcpu_signal_handler); //in order to cleanly abort batch jobs which are signalled

   TString batchName;
   batchName = gSystem->Getenv("KVBATCHNAME");
   if (batchName == "") {
      printf("*** Error in KaliVedaAnalysis *** : Set environment variable KVBATCHNAME with name of batch job before running\n");
      return 0;
   }
   TString launchDir = gSystem->Getenv("KVLAUNCHDIR");
   if (launchDir != "" && launchDir != gSystem->WorkingDirectory()) {
      cout << "*** Copying user .kvrootrc & .rootrc from directory : "
           << launchDir.Data() << "\n*** to directory : " << gSystem->WorkingDirectory() << endl;
      TString path_src, path_trg;
      //.kvrootrc
      AssignAndDelete(path_src, gSystem->ConcatFileName(launchDir.Data(), ".kvrootrc"));
      AssignAndDelete(path_trg, gSystem->ConcatFileName(gSystem->WorkingDirectory(), ".kvrootrc"));
      gSystem->CopyFile(path_src.Data(), path_trg.Data());
      //.rootrc
      AssignAndDelete(path_src, gSystem->ConcatFileName(launchDir.Data(), ".rootrc"));
      AssignAndDelete(path_trg, gSystem->ConcatFileName(gSystem->WorkingDirectory(), ".rootrc"));
      gSystem->CopyFile(path_src.Data(), path_trg.Data());
   }

   //without this, the plugins don't work for user classes
   TApplication* myapp = new TApplication("myapp", &argc, argv);

   new KVDataRepositoryManager();
   gDataRepositoryManager->Init();

   //create analyser and initialise it using the batch env file
   KVDataAnalyser* ia = new KVDataAnalyser();
   ia->SetBatchMode();
   ia->SetBatchName(batchName.Data());
   ia->Run();

   //delete batch env files
   TString path;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(gSystem->HomeDirectory(),
                                           Form(".%s", batchName.Data())));
   gSystem->Unlink(path.Data());
   AssignAndDelete(path,
                   gSystem->ConcatFileName(gSystem->HomeDirectory(),
                                           Form(".%s.bak", batchName.Data())));
   gSystem->Unlink(path.Data());

   delete myapp;
   delete gDataRepositoryManager;

// avoid batch jobs running long after analysis has completed
// the only way to do it is to kill the process
   int pid = gSystem->GetPid();
   gSystem->Exec(Form("kill -9 %d", pid));

   return 0;
}
