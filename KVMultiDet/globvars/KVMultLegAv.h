#ifndef KVMultLegAv_h
#define KVMultLegAv_h
#include "KVMultAv.h"

/**
  \class KVMultLegAv
  \ingroup GlobalVariables

  \brief Multiplicity of LCP \f$Z\leq 2\f$ in forward hemisphere of c.m. frame

  This is a KVMultAv with the added condition

  ~~~~~~~{.cpp}
  AddSelection("_NUC_->GetZ()<=2");
  ~~~~~~~
 */

class KVMultLegAv: public KVMultAv {

   void init()
   {
#ifdef USING_ROOT6
      AddSelection({"Z<=2", [](const KVNucleus * nuc)
      {
         return nuc->GetZ() <= 2;
      }
                   });
#else
      AddSelection("_NUC_->GetZ()<=2");
#endif
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVMultLegAv, KVMultAv)

   ClassDef(KVMultLegAv, 1)
};
#endif
