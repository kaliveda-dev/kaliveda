#include "KVMultLeg.h"

void KVMultLeg::init()
{
#ifdef USING_ROOT6
   SetSelection({"Z<=2", [](const KVNucleus * nuc)
   {
      return nuc->GetZ() <= 2;
   }
                });
#else
   SetSelection("_NUC_->GetZ()<=2");
#endif
}

ClassImp(KVMultLeg)
