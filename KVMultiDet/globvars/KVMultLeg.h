#ifndef KVMultLeg_h
#define KVMultLeg_h
#include "KVMult.h"

/**
  \class KVMultLeg
  \ingroup GlobalVariables

  \brief Multiplicity of LCP \f$Z\leq 2\f$ in event
 */

class KVMultLeg: public KVMult {

   void init();

public:
   ROOT_FULL_SET_WITH_INIT(KVMultLeg, KVMult)

   ClassDef(KVMultLeg, 1)
};
#endif
