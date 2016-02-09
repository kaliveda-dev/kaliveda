//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVEtrans_h
#define KVEtrans_h
#include "KVVarGlob1.h"

//#define DEBUG_KVEtrans

class KVEtrans: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVEtrans(void);

public:
   KVEtrans(void);             // constructeur par defaut
   KVEtrans(Char_t* nom);
   KVEtrans(const KVEtrans& a);        // constructeur par Copy

   virtual ~ KVEtrans(void);   // destructeur

   KVEtrans& operator =(const KVEtrans& a);    // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVEtrans, 1)       // Global variable Sum(Etrans)
};
#endif
