//Created by KVClassFactory on Wed Jan 23 16:46:57 2013
//Author: John Frankland,,,

#ifndef __KVQUADMOMENT_H
#define __KVQUADMOMENT_H

#include "KVVarGlob.h"

class KVQuadMoment : public KVVarGlob {
   Double_t matrix[3][3];//!

   void init_KVQuadMoment(void);
   virtual Double_t getvalue_void(void) const;
   virtual Double_t getvalue_int(Int_t i);

public:
   KVQuadMoment();
   KVQuadMoment(const KVQuadMoment& obj);
   KVQuadMoment(Char_t* nom);
   virtual ~KVQuadMoment();
   void Copy(TObject& obj) const;

   virtual void Init(void);

   virtual void Reset(void);

   virtual void Fill(KVNucleus* c);

   ClassDef(KVQuadMoment, 1) //Quadrupole moment tensor of particle momenta
};

#endif
