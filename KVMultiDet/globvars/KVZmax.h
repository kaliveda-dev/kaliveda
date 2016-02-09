//
// D.Cussol
//
// 18/02/2004
// Creation d'une classe Variable Globale
//

#ifndef KVZmax_h
#define KVZmax_h
#include "KVVarGlob1.h"

//#define DEBUG_KVZmax

class KVZmax: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Champs prives:
protected:
   TList* heaviest;            //sorted list of pointers to nuclei, sorted by decreasing Z
   Bool_t fSorted;              //true if list has been sorted already
// Methodes
protected:
   void init_KVZmax(void);
   virtual Double_t getvalue_void(void) const;
   virtual Double_t getvalue_int(Int_t);

public:
   KVZmax(void);               // constructeur par defaut
   KVZmax(Char_t* nom);
   KVZmax(const KVZmax& a);    // constructeur par Copy

   virtual ~ KVZmax(void);     // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVZmax& operator =(const KVZmax& a);        // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.
   virtual void Reset(void);    // Remise a zero de la charge et du
   // pointeur.
   virtual void Init(void);     // Remise a zero de la charge et du
   // pointeur.
   virtual TObject* GetObject(void) const
   {
      // Retourne le pointeur du fragment le plus lourd.
      return (TObject*) const_cast<KVZmax*>(this)->GetZmax(0);
   };

   KVNucleus* GetZmax(Int_t i);

   Double_t* GetValuePtr(void);

   ClassDef(KVZmax, 1)//Rank fragments by charge and return Z or pointer of i_th heaviest fragment
};
#endif
