#ifndef KVRiso_h
#define KVRiso_h
#include "KVVarGlob.h"

/**
  \class KVRiso
  \brief Energy isotropy ratio \f$R_{E}\f$
  \ingroup GlobalVariables

It is defined as follow:

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
      Mult            3                       Number of particles included in calculation

 */
class KVRiso: public KVVarGlob {

   Double_t Riso;
   Double_t Epar;
   Double_t Etrans;
   Int_t Mult;

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
   }
   void Reset(void)
   {
      Epar = Etrans = 0;
      Mult = 0;
   }
   void Calculate();
   Char_t GetValueType(Int_t i) const
   {
      // "Mult" is an integer, return 'I' if i==3
      if (i == 3) return 'I';
      return 'D';
   }

   ClassDef(KVRiso, 1)// Global variable Riso=Sum(epar)/2*Sum(eper)
};
#endif
