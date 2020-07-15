#ifndef KVMultIMF_h
#define KVMultIMF_h
#include "KVMult.h"

/**
  \class KVMultIMF
  \ingroup GlobalVariables

  \brief Multiplicity of IMF \f$Z\geq 3\f$ in event
 */

class KVMultIMF: public KVMult {
   void init()
   {
#ifdef USING_ROOT6
      SetSelection({"Z>=3", [](const KVNucleus * nuc)
      {
         return nuc->GetZ() >= 3;
      }
                   });
#else
      SetSelection("_NUC_->GetZ()>=3");
#endif
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVMultIMF, KVMult)

   ClassDef(KVMultIMF, 1)
};
#endif
