#ifndef KVRiso_h
#define KVRiso_h
#include "KVVarGlob.h"

/**
////////////////////////////////////////////////////////////////////////////////
   Global variable returning the Riso variable. It is defined as follow:

           Etrans
   Riso= ----------
           2*Epar


with   Etrans=Sum(etrans(i)) : sum of the transverse energies of the
                               particules and fragments
       Epar=Sum(epar(i)) : sum of the parallel energies of the
                               particules and fragments

By default Riso is calculated from particle momenta in the default (laboratory) frame.
However, this can be changed by giving the name of the desired frame
with method SetFrame() :
      riso->SetFrame( "QP" ); // calculate in "QP" frame

 The names of the variables and the corresponding indexes are as follows:

      Name            Index                   Meaning
----------------------------------------------------------------------------
      Riso            0       (default)       Riso
      Epar            1                       Sum of parallel kinetic energies
      Eperp           2                       Sum of transverse kinetic energies


 All these values can be obtained by calling the GetValuePtr() method which
 returns an array of Double_t containing the values.

 Here are examples on how obtaining values.

 KVRiso *priso=new KVRiso();
 KVRiso riso;
 ...
 Double_t r=priso->GetValue();  |
          r=prsio->GetValue(0); |--> Riso
          r=priso("Riso");      |
 Double_t epar=riso(1);---------> Sum of parallel kinetic energies
 Double_t etrans=priso->GetValue("Eperp");--> Sum of transverse kinetic
                                              energies
 Double_t *values=riso.GetValuePtr();
          r=values[0];      --> Riso
          epar=values[1];   --> Epar
          etrans=values[2]; --> Eperp


 Look at KVVarGlob class to have an example of use.


 */
class KVRiso: public KVVarGlob {

   Double_t Riso;
   Double_t Epar;
   Double_t Etrans;

   void init();

protected:
   Double_t getvalue_int(Int_t) const;
   void fill(const KVNucleus*);

public:
   KVRiso() : KVVarGlob("KVRiso")
   {
      init();
   }
   KVRiso(const Char_t* nom) : KVVarGlob(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVRiso, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVRiso)
   virtual ~KVRiso() {}

   void Copy(TObject& obj) const;

   void Init(void)
   {
      init();
   }
   void Reset(void)
   {
      Epar = Etrans = 0;
   }
   void Calculate();

   ClassDef(KVRiso, 1)// Global variable Riso=Sum(epar)/2*Sum(eper)
};
#endif
