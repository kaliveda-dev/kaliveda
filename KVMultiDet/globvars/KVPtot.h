#ifndef KVPtot_h
#define KVPtot_h
#include "KVVGVectorSum.h"

/**
  \class KVPtot
  \ingroup GlobalVariables
  \brief Sum of particle momenta
 */

class KVPtot: public KVVGVectorSum {

protected:
   void fill(const KVNucleus* n)
   {
      Add(n->GetMomentum());
   }

public:
   KVPtot() : KVVGVectorSum("KVPtot") {}
   KVPtot(const Char_t* name) : KVVGVectorSum(name) {}
   virtual ~KVPtot() {}

   ClassDef(KVPtot, 1) //Sum of particle momenta
};
#endif
