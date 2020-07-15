#include "KVEtransLCP.h"

void KVEtransLCP::init()
{
#ifdef USING_ROOT6
   SetSelection({"Z<3", [](const KVNucleus * nuc)
   {
      return nuc->GetZ() < 3;
   }
                });
#else
   SetSelection("_NUC_->GetZ()<3");
#endif
}

ClassImp(KVEtransLCP)
