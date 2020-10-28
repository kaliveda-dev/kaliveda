#ifndef __KVDIRECTIVITY_H
#define __KVDIRECTIVITY_H

#include "KVVarGlob1.h"

/**
 \class KVDirectivity
 \brief Transverse momentum directivity (FOPI)

 Calculate the transverse-momentum directivity defined by
 \f[
 D=\frac{\left | \sum \mathbf{p}_t \right |_{y \geq y_{c.m.}}}{\sum \left | \mathbf{p}_t \right |_{y \geq y_{c.m.}}}
 \f]
 as in Phys. Rev. Lett. 69 (1992) 889 (FOPI Collaboration).
 Sums are over transverse CM momenta for particles in forward CM hemisphere.

 \author John Frankland
 \date Fri Nov 27 09:59:51 2020
*/

class KVDirectivity: public KVVarGlob1 {

   double total_mod_trans;
   TVector3 total_trans;

public:
   KVDirectivity()
      : KVVarGlob1()
   {
      init();
   }
   KVDirectivity(const Char_t* nom)
      : KVVarGlob1(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVDirectivity, KVVarGlob1)
   ROOT_COPY_ASSIGN_OP(KVDirectivity)

   virtual ~KVDirectivity(void) {}

   virtual void Copy(TObject& obj) const;

   void Calculate();
   void Init()
   {
      total_trans = TVector3();
      total_mod_trans = 0;
      SetFrame("CM");
   }
   void Reset()
   {
      Init();
   }

private:
   void init();

protected:
   void fill(const KVNucleus* n);

   ClassDef(KVDirectivity, 1) //Transverse momentum directivity (FOPI)
};

#endif
