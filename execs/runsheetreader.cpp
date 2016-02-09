#include "KVINDRARunSheetGUI.h"
#include "TApplication.h"
#include "TRint.h"
#include "KVDataRepositoryManager.h"

#define WITHRINT

int main(int argc, char** argv)
{
#ifdef WITHRINT
   TRint* myapp = new TRint("KVDataBaseGUI", &argc, argv, NULL, 0);
#else
   TApplication* myapp = new TApplication("KVDataBaseGUI", &argc, argv);
#endif
   KVINDRARunSheetGUI g;
   myapp->Run();
   if (gDataRepositoryManager) delete gDataRepositoryManager;
   return 0;
}
