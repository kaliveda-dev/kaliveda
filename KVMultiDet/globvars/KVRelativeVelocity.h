#ifndef __KVRELATIVEVELOCITY_H
#define __KVRELATIVEVELOCITY_H

#include "KVVarGlobMean.h"

/**
 \class KVRelativeVelocity
 \brief Calculate mean relative velocity of particls

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Sun Aug  9 17:26:58 2020
*/

class KVRelativeVelocity: public KVVarGlobMean {

   void init()
   {
      fType = kTwoBody;
   }

public:
   KVRelativeVelocity()
      : KVVarGlobMean()
   {
      init();
   }
   KVRelativeVelocity(const Char_t* nom)
      : KVVarGlobMean(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVRelativeVelocity, KVVarGlobMean)
   ROOT_COPY_ASSIGN_OP(KVRelativeVelocity)
   virtual ~KVRelativeVelocity(void) {}

protected:
   void fill2(const KVNucleus* n1, const KVNucleus* n2)
   {
      if (n1 != n2) FillVar((n1->GetVelocity() - n2->GetVelocity()).Mag());
   }

   ClassDef(KVRelativeVelocity, 1) //Calculate mean relative velocity of particls
};

#endif
