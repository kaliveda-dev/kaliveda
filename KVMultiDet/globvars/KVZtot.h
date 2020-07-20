#ifndef KVZtot_h
#define KVZtot_h
#include "KVVGSum.h"

/**
  \class KVZtot
  \ingroup GlobalVariables

  \brief Sum of atomic numbers \f$Z\f$ in event

  \f[
  \sum_{i=1}^N Z_i
  \f]
 */

class KVZtot: public KVVGSum {

   void init()
   {
      SetOption("mode", "sum");
      SetOption("method", "GetZ");
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZtot, KVVGSum)

   ClassDef(KVZtot, 1)
};
#endif
