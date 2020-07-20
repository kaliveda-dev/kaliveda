#ifndef KVZbound_h
#define KVZbound_h
#include "KVZtot.h"

/**
  \class KVZbound
  \ingroup GlobalVariables

  \brief Sum of atomic numbers of fragments \f$Z\geq 3\f$ in event
 */

class KVZbound: public KVZtot {

   void init()
   {
      SetSelection("_NUC_->GetZ()>=3");
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZbound, KVZtot)

   ClassDef(KVZbound, 1)
};
#endif
