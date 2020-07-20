#ifndef KVZmax_h
#define KVZmax_h
#include "KVVarGlob.h"

/**
\class KVZMax
\ingroup GlobalVariables
\brief Global variable used to sort particles in order of decreasing atomic number</h4>

  Global variable returning the charge of the `(i+1)`-th heaviest fragment
  and the pointer to the `(i+1)`-th heaviest fragment, where `i=0,1,...,M-1`
  with `M =` total multiplicity of event.

  All nuclei of the event are sorted in descending order according to their charge
  (`Z(0)` > `Z(1)` > `Z(2)` > ... > `Z(M-1)`) .

  Simple interface GetValue() returns Z of heaviest fragment
  and GetObject returns pointer to this fragment.

  All other fragments accessible via GetValue(i) and GetZmax(i), `i` ranging
  from 0 to `M-1` (`M`=total multiplicity)
  (the latter method returns a KVNucleus pointer directly).

  Also GetValue(`"Zmax1"`), GetValue(`"Zmax2"`), ..., GetValue(`"Zmax50"`)
  can be used to obtain Z of 50 heaviest nuclei.

  By default when used in a KVGVList global variable list and automatic TTree branch creation is used
  (see method KVGVList::MakeBranches and the example given below), the first 3 largest Z will be stored.
  You can change this with method SetMaxNumBranches().

 */

class KVZmax: public KVVarGlob {

   KVList heaviest;            //sorted list of pointers to nuclei, sorted by decreasing Z
   Bool_t fSorted;              //true if list has been sorted already

   void init();
protected:
   Double_t getvalue_int(Int_t i) const
   {
      //returns the Z of the (i+1)th heaviest fragment
      // i=0 : Zmax
      // i=1 : Zmax2
      // etc.

      if (!GetZmax(i))
         return -1.0;
      return (Double_t) GetZmax(i)->GetZ();
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVZmax, KVVarGlob)

   virtual void Copy(TObject& obj) const;
   void Init()
   {
      fSorted = kFALSE;
   }
   void Calculate()
   {
      heaviest.Sort();
      fSorted = kTRUE;
   }
   void Reset()
   {
      heaviest.Clear();
      fSorted = kFALSE;
   }
   void fill(const KVNucleus* c)
   {
      heaviest.Add(const_cast<KVNucleus*>(c));
   }
   virtual TObject* GetObject(void) const
   {
      // Retourne le pointeur du fragment le plus lourd.
      return GetZmax(0);
   }

   KVNucleus* GetZmax(Int_t i) const
   {
      // Pointer to (i+1)th heaviest fragment (i=0 : Zmax, i=1 : Zmax2, etc.)

      return (KVNucleus*) heaviest.At(i);
   }

   std::vector<Double_t> GetValuePtr(void) const;

   ClassDef(KVZmax, 1)//Rank fragments by charge and return Z or pointer of i_th heaviest fragment
};
#endif
