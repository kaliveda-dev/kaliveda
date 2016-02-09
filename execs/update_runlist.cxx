/*
$Id: update_runlist.cxx,v 1.4 2008/02/08 08:22:57 franklan Exp $

Executable for updating available runlists for different datasets
Usage:

   update_runlist [data repository name] [dataset name] [data type]

*/

#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "KVDataSet.h"
#include "Riostream.h"

using namespace std;

int main(int argc, char** argv)
{

   if (argc < 3) {
      cout << "Executable for updating available runlists for different KaliVeda datasets" << endl;
      cout << "Usage:" << endl << endl;
      cout << "\tupdate_runlist [data repository name] [dataset name] [data type]" << endl << endl;
      return 0;
   }

   new KVDataRepositoryManager;
   gDataRepositoryManager->Init();
   gDataRepositoryManager->GetRepository(argv[1])->cd();
   gDataSetManager->GetDataSet(argv[2])->cd();
   if (argc == 5) {
      cout << "update_runlist error: no such function" << endl;
   } else {
      gDataSet->UpdateAvailableRuns(argv[3]);
   }
   delete gDataRepositoryManager;
   return 0;
}
