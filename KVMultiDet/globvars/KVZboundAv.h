#ifndef KVZboundAv_h
#define KVZboundAv_h
#include "KVZtotAv.h"

/**
  \class KVZboundAv
  \ingroup GlobalVariables

  \brief Sum of atomic numbers of fragments \f$Z\geq 3\f$ in forward c.m. hemisphere

  This is a KVZtotAv with the added condition

  ~~~~~~~{.cpp}
  SetSelection("_NUC_->GetZ()>=3");
  ~~~~~~~
 */

class KVZboundAv: public KVZtotAv {
   void init()
   {
#ifdef USING_ROOT6
      AddSelection({"Z>=3", [](const KVNucleus * nuc)
      {
         return nuc->GetZ() >= 3;
      }
                   });
#else
      AddSelection("_NUC_->GetZ()>=3");
#endif
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZboundAv, KVZtotAv)

   ClassDef(KVZboundAv, 1)
};
#endif
