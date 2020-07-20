#ifndef KVEtrans_h
#define KVEtrans_h
#include "KVVGSum.h"

/**
  \class KVEtrans
  \ingroup GlobalVariables

  \brief Sum of transverse kinetic energies in event

  \f[
  \sum_{i=1}^N E_i\sin^2\theta_i
  \f]
 */

class KVEtrans: public KVVGSum {

   void init();

public:
   ROOT_FULL_SET_WITH_INIT(KVEtrans, KVVGSum)

   ClassDef(KVEtrans, 1) // Global variable Sum(Etrans)
};
#endif
