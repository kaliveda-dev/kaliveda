#ifndef KVZVtot_h
#define KVZVtot_h
#include "KVVGVectorSum.h"

/**
\class KVZVtot
\ingroup GlobalVariables
\brief Sum of particle pseudo-momentum
 */

class KVZVtot: public KVVGVectorSum {

protected:
   void fill(const KVNucleus* n)
   {
      Add(n->GetZ()*n->GetVelocity());
   }

public:
   KVZVtot() : KVVGVectorSum("KVZVtot") {}
   KVZVtot(const Char_t* name) : KVVGVectorSum(name) {}
   virtual ~KVZVtot() {}

   ClassDef(KVZVtot, 1)        // Global variable ZVtot=Sum(Z(i)*v(i)) (units: cm/ns)
};
#endif
