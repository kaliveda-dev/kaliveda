//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVMultIMF_h
#define KVMultIMF_h
#include "KVZbound.h"

//#define DEBUG_KVMultIMF

class KVMultIMF: public KVZbound {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVMultIMF(void);

public:
   KVMultIMF(void);            // constructeur par defaut
   KVMultIMF(Char_t* nom);

   virtual ~ KVMultIMF(void);  // destructeur

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVMultIMF, 1)      // Multiplicity for fragments with Z >= Zmin
};
#endif
