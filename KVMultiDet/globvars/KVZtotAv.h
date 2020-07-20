#ifndef KVZtotAv_h
#define KVZtotAv_h
#include "KVZtot.h"

/**
  \class KVZtotAv
  \ingroup GlobalVariables

  \brief Sum of atomic numbers \f$Z\f$ in forward c.m. hemisphere

  This is a KVZtot with the added conditions

  ~~~~~~~{.cpp}
  SetFrame("CM");
  SetSelection("_NUC_->GetVpar()>0");
  ~~~~~~~
 */

class KVZtotAv: public KVZtot {

   void init()
   {
      SetFrame("CM");
      SetSelection("_NUC_->GetVpar()>0");
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZtotAv, KVZtot)

   ClassDef(KVZtotAv, 1)
};
#endif
