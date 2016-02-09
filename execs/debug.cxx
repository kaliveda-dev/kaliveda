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
#include "TRint.h"
#include "KVDataSetManager.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataAnalyser.h"

int main(int argc, char** argv)
{
   //without this, the plugins don't work for user classes
   TRint* myapp = new TRint("myapp", &argc, argv);

   KVBase::InitEnvironment();
   new KVDataRepositoryManager;
   gDataRepositoryManager->Init();

   gDataSetManager->GetDataSet("ISOFAZIA")->cd();

   myapp->Run();
   delete gDataRepositoryManager;

   return 0;
}
