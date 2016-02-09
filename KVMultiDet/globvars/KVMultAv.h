//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVMultAv_h
#define KVMultAv_h
#include "KVVarGlob1.h"

//#define DEBUG_KVMultAv

class KVMultAv: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVMultAv(void);

public:
   KVMultAv(void);             // constructeur par defaut
   KVMultAv(Char_t* nom);
   KVMultAv(const KVMultAv& a);        // constructeur par Copy

   virtual ~ KVMultAv(void);   // destructeur

   KVMultAv& operator =(const KVMultAv& a);    // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVMultAv, 1)       // Multiplicity for particules and fragments with V z> 0
};
#endif
