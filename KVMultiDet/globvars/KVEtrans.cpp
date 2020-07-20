#include "KVEtrans.h"

ClassImp(KVEtrans)

void KVEtrans::init()
{
   SetOption("mode", "sum");
   SetOption("method", "GetEtran");
}
