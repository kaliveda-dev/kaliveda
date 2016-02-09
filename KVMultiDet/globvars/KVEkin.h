//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVEkin_h
#define KVEkin_h
#include "KVVarGlob1.h"

//#define DEBUG_KVEkin

class KVEkin: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVEkin(void);

public:
   KVEkin(void);               // constructeur par defaut
   KVEkin(Char_t* nom);
   KVEkin(const KVEkin& a);    // constructeur par Copy

   virtual ~ KVEkin(void);     // destructeur

   KVEkin& operator =(const KVEkin& a);        // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVEkin, 1)         // Global variable Sum(Ekin)
};
#endif
