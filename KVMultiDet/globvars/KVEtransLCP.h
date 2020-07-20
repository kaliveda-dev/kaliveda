#ifndef KVEtransLCP_h
#define KVEtransLCP_h
#include "KVEtrans.h"

/**
  \class KVEtransLCP
  \ingroup GlobalVariables

  \brief Sum of transverse kinetic energies of LCP (\f$Z\leq 2\f$) in event

  \f[
  \sum_{i=1}^N (Z_i\leq 2)E_i\sin^2\theta_i
  \f]
 */

class KVEtransLCP: public KVEtrans {

   void init();

public:
   ROOT_FULL_SET_WITH_INIT(KVEtransLCP, KVEtrans)

   ClassDef(KVEtransLCP, 1)
};
#endif
