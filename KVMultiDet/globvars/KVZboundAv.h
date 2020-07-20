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
      AddSelection("_NUC_->GetZ()>=3");
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZboundAv, KVZtotAv)

   ClassDef(KVZboundAv, 1)
};
#endif
