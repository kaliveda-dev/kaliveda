#include <KVTreeAnalyzer.h>
#include "TRint.h"

int main(int argc, char** argv)
{
   TRint* myapp = new TRint("kvtreeanalyzer", &argc, argv, NULL, 0);
   new KVTreeAnalyzer(kFALSE);
   myapp->Run();
   delete myapp;
   return 0;
}

