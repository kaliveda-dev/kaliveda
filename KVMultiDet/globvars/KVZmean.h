#ifndef KVZmean_h
#define KVZmean_h
#include "KVZtot.h"

/**
  \class KVZmean
  \ingroup GlobalVariables

  \brief Mean atomic number \f$\langle Z\rangle\f$ of selected nuclei

  \f[
  \frac{1}{N}\sum_{i=1}^N Z_i
  \f]
 */
class KVZmean: public KVZtot {
   void init()
   {
      SetOption("mode", "mean");
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZmean, KVZtot)

   ClassDef(KVZmean, 1)
};
#endif
