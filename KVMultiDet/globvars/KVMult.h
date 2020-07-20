#ifndef KVMULT_H
#define KVMULT_H

#include "KVVGSum.h"

/**
  \class KVMult
  \ingroup GlobalVariables

  \brief Multiplicity of nuclei in event
 */

class KVMult : public KVVGSum {
   void init();
public:
   ROOT_FULL_SET_WITH_INIT(KVMult, KVVGSum)

   ClassDef(KVMult, 1)
};

#endif // KVMULT_H
