//
// D.Cussol
//
// 18/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVZmean_h
#define KVZmean_h
#include "KVVarGlobMean.h"

//#define DEBUG_KVZmean

class KVZmean: public KVVarGlobMean {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVZmean(void);

public:
   KVZmean(void);              // constructeur par defaut
   KVZmean(char* nom);
   KVZmean(const KVZmean& a);  // constructeur par Copy

   virtual ~ KVZmean(void);    // destructeur

   KVZmean& operator =(const KVZmean& a);      // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVZmean, 1)        // Global variable <Z>
};
#endif
