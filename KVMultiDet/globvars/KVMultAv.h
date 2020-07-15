#ifndef KVMultAv_h
#define KVMultAv_h
#include "KVMult.h"

/**
  \class KVMultAv
  \ingroup GlobalVariables

  \brief Multiplicity of nuclei in forward c.m. hemisphere

  This is a KVMult with the added conditions

  ~~~~~~~{.cpp}
  SetFrame("CM");
  SetSelection("_NUC_->GetVpar()>0");
  ~~~~~~~
 */

class KVMultAv: public KVMult {

   void init()
   {
      SetFrame("CM");
#ifdef USING_ROOT6
      SetSelection({"Vpar>0", [](const KVNucleus * nuc)
      {
         return nuc->GetVpar() > 0;
      }
                   });
#else
      SetSelection("_NUC_->GetVpar()>0");
#endif
   }


public:
   ROOT_FULL_SET_WITH_INIT(KVMultAv, KVMult)

   ClassDef(KVMultAv, 1)
};
#endif
