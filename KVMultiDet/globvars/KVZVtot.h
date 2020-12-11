#ifndef KVZVtot_h
#define KVZVtot_h
#include "KVVGVectorSum.h"

/**
\class KVZVtot
\ingroup GlobalVariables
\brief Sum of particle pseudo-momentum

Total pseudo-momentum is calculated as
\f[
\vec{P} = \sum_i Z_i\vec{V}_i
\f]
where the sum runs over whatever selection of particles is defined,
and the velocity vectors are in whatever reference frame is defined
(default: laboratory).

Normalization to projectile pseudo-momentum: call SetNormalization() with value calculated for entrance channel, e.g.
~~~~{.cpp}
KVZVtot vg("ZVTOT");
vg.SetNormalization(...);
~~~~
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
