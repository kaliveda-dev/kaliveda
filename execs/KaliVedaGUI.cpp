//
// Standalone program launching the KVDataAnalysisLauncher
//

#include "TApplication.h"
#include "TRint.h"
#include "Riostream.h"
#include "KVDataAnalysisLauncher.h"
#include "KVDataRepositoryManager.h"
#include <cstdlib>

//#define WITHRINT

int main(int argc, char* argv[])
{
//cout << "Creating myapp" << endl;
#ifdef WITHRINT
   TRint* myapp = new TRint("RootSession", &argc, argv, NULL, 0);
#else
   TApplication* myapp = new TApplication("myapp", &argc, argv);
#endif
   gSystem->AddIncludePath("-I$KVROOT/include");
//cout << "Launching GUI" << endl;
   if (argc > 1) new KVDataAnalysisLauncher(argv[1], gClient->GetRoot());
   else new KVDataAnalysisLauncher("INDRA", gClient->GetRoot());

   myapp->SetReturnFromRun(kTRUE);
   myapp->Run();

// avoid GUI hanging after "Quit" button is pressed
// the only way to do it is to kill it
   int pid = gSystem->GetPid();
   gSystem->Exec(Form("kill -9 %d", pid));

   return 0;
}
