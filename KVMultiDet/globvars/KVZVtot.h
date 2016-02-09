//
// D.Cussol
//
// 20/03/2006:
// Creation de la Variable Globale ZVtot
//

#ifndef KVZVtot_h
#define KVZVtot_h
#include "KVPtot.h"

//#define DEBUG_KVZVtot

class KVZVtot: public KVPtot {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;

// Methodes
protected:
   void init_KVZVtot(void);

public:
   KVZVtot(void);              // constructeur par defaut
   KVZVtot(Char_t* nom, const Char_t* frm = "");
   KVZVtot(const KVZVtot& a);  // constructeur par Copy

   virtual ~ KVZVtot(void);    // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVZVtot& operator =(const KVZVtot& a);      // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVZVtot, 1)        // Global variable ZVtot=Sum(Z(i)*v(i)) (units: cm/ns)
};
#endif
