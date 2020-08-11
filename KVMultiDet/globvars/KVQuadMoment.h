//Created by KVClassFactory on Wed Jan 23 16:46:57 2013
//Author: John Frankland,,,

#ifndef __KVQUADMOMENT_H
#define __KVQUADMOMENT_H

#include "KVVarGlob.h"
/**
\class KVQuadMoment
\brief Quadrupole moment tensor of particle momenta
\ingroup GlobalVariables
*/
class KVQuadMoment : public KVVarGlob {
   Double_t matrix[3][3];//!

   void init_KVQuadMoment(void);
protected:
   virtual Double_t getvalue_int(Int_t i) const;
   virtual void fill(const KVNucleus* c);

public:
   KVQuadMoment();
   KVQuadMoment(const Char_t* nom);
   ROOT_COPY_CTOR(KVQuadMoment, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVQuadMoment)
   virtual ~KVQuadMoment() {}
   void Copy(TObject& obj) const;

   virtual void Init(void);
   virtual void Reset(void);

   ClassDef(KVQuadMoment, 1) //Quadrupole moment tensor of particle momenta
};

#endif
