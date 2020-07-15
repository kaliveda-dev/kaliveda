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
#ifdef USING_ROOT6
      SetSelection({"Z>=3", [](const KVNucleus * nuc)
      {
         return nuc->GetZ() >= 3;
      }
                   });
#else
      SetSelection("_NUC_->GetZ()>=3");
#endif
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZbound, KVZtot)

   ClassDef(KVZbound, 1)
};
#endif
