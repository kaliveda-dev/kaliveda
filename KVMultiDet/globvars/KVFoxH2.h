/*
$Id: KVFoxH2.h,v 1.2 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.2 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Tue Nov 27 17:47:49 2007
//Author: franklan

#ifndef __KVFOXH2_H
#define __KVFOXH2_H

#include "KVVarGlob1.h"

/**
\class KVFoxH2
\brief Event shape global variable : second Fox-Wolfram moment, H(2)
\ingroup GlobalVariables

We use the following formula:
\f[
H_{2} = \frac{\sum_{i,j} |p_{i}| |p_{j}| (3cos^{2}\theta_{rel} - 1) }{2\sum_{i,j} |p_{i}||p_{j}|}
\f]
to calculate \f$H(2)\f$. By default, momenta in "CM" frame are used. (change with SetFrame()).
*/

class KVFoxH2: public KVVarGlob1 {

   Double_t num;
   Double_t den;

   void init();

protected:
   void fill2(const KVNucleus* n1, const KVNucleus* n2);

public:
   KVFoxH2()
      : KVVarGlob1()
   {
      init();
   }
   KVFoxH2(const Char_t* nom)
      : KVVarGlob1(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVFoxH2, KVVarGlob1)
   ROOT_COPY_ASSIGN_OP(KVFoxH2)
   virtual ~KVFoxH2() {}

   void Init();
   void Reset();
   void Calculate();

   virtual void Copy(TObject& obj) const;

   ClassDef(KVFoxH2, 1) //Event shape global variable : second Fox-Wolfram moment, H(2)
};


#endif
