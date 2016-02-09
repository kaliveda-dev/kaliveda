//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVZtot_h
#define KVZtot_h
#include "KVVarGlob1.h"

//#define DEBUG_KVZtot

class KVZtot: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVZtot(void);

public:
   KVZtot(void);               // constructeur par defaut
   KVZtot(char* nom);
   KVZtot(const KVZtot& a);    // constructeur par Copy

   virtual ~ KVZtot(void);     // destructeur

   KVZtot& operator =(const KVZtot& a);        // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVZtot, 1)         // Global variable Sum(Z)
};
#endif
