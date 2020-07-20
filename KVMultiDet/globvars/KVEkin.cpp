#include "KVEkin.h"

ClassImp(KVEkin)

void KVEkin::init()
{
   SetOption("mode", "sum");
   SetOption("method", "GetKE");
}

