#ifndef KVMULT_H
#define KVMULT_H

#include "KVVGSum.h"

/**
  \class KVMult
  \ingroup GlobalVariables

  \brief Multiplicity of all nuclei in event (including \f$Z=0\f$)

 */

class KVMult : public KVVGSum {
   void init();
public:
   ROOT_FULL_SET_WITH_INIT(KVMult, KVVGSum)

   ClassDef(KVMult, 1)
};

#endif // KVMULT_H
