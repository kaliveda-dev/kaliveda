#include "TRint.h"
#include "KVBase.h"

int main(int argc, char** argv)
{
   KVBase::InitEnvironment();
   TRint* myapp = new TRint("kaliveda", &argc, argv, NULL, 0);
   myapp->SetPrompt("kaliveda [%d] ");
   myapp->Run();
   return 0;
}
