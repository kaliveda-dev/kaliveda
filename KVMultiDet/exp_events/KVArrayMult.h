#ifndef __KVARRAYMULT_H
#define __KVARRAYMULT_H

#include "KVMult.h"
#include "KVReconstructedNucleus.h"

/**
 \class KVArrayMult
 \brief Calculate multiplicity in given array
 \ingroup GlobalVariables

Only use with reconstructed (experimental or filtered) data!

The initial name given to the variable (i.e. in the constructor or as argument to KVGVList::AddGV())
is the name of the array in which the multiplicity is to be calculated.

The actual name of the variable (as it will appear e.g. in a TTree branch) will be `Mult_[array]`.
*/

class KVArrayMult : public KVMult {
public:
   KVArrayMult()
      : KVMult()
   {}
   KVArrayMult(const Char_t* name)
      : KVMult(Form("Mult_%s", name))
   {
      // \param[in] name name of array for which to calculate multiplcity

      SetTitle(name);
#ifdef USING_ROOT6
      SetSelection({Form("Mult_%s", name), [&](const KVNucleus * n)
      {
         return dynamic_cast<const KVReconstructedNucleus*>(n)->InArray(GetTitle());
      }
                   });
#else
      SetSelection(Form("_NUC_->InArray(\"%s\")", name));
#endif
   }
   virtual ~KVArrayMult() {}

   ClassDef(KVArrayMult, 1) //Calculate multiplicity in given array
};

#endif
