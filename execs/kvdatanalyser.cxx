/*
$Id: kvdatanalyser.cxx,v 1.6 2008/02/08 08:22:57 franklan Exp $

KaliVeda.C

Application for launching KaliVeda data analysis tasks
Usage:

   kvdatanalyser [repository=...] [plugin=...]

where:
   [repository] is the name of a known data repository (as defined in .kvrootrc)
   if not given, default repository is used.
   */

#include <cstdlib>
#include "TString.h"
#include "TApplication.h"
#include "KVDataSetManager.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataAnalyser.h"

int main(int argc, char** argv)
{
   //without this, the plugins don't work for user classes
   TApplication* myapp = new TApplication("myapp", &argc, argv);

   TString repo, plugin, tmp;
   /* analyse arguments */
   for (int i = 1; i < argc; i++) {
      tmp = argv[i];
      if (tmp.Contains("repository=")) {
         tmp.Remove(0, 11);
         repo = tmp;
      } else if (tmp.Contains("plugin=")) {
         tmp.Remove(0, 7);
         plugin = tmp;
      }
   }
   gSystem->AddIncludePath("-I$KVROOT/include");
   new KVDataRepositoryManager;
   gDataRepositoryManager->Init();
   if (gDataRepositoryManager->GetListOfRepositories()->GetEntries() == 0) exit(1);
   if (repo != "") gDataRepositoryManager->GetRepository(repo.Data())->cd();
   KVDataAnalyser::RunAnalyser(plugin);

   delete myapp;
   delete gDataRepositoryManager;

   return 0;
}
