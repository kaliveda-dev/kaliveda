#ifndef KVEkin_h
#define KVEkin_h
#include "KVVGSum.h"

/**
  \class KVEkin
  \ingroup GlobalVariables

  \brief Sum of kinetic energies in event

  \f[
  \sum_{i=1}^N E_i
  \f]

 */
class KVEkin: public KVVGSum {
   void init();
public:
   ROOT_FULL_SET_WITH_INIT(KVEkin, KVVGSum)

   ClassDef(KVEkin, 1)// Global variable Sum(Ekin)
};
#endif
