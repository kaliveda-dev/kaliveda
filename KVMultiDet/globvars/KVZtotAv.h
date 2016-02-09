//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVZtotAv_h
#define KVZtotAv_h
#include "KVZtot.h"

//#define DEBUG_KVZtot

class KVZtotAv: public KVZtot {
// Methodes
public:
   KVZtotAv(void);              // constructeur par defaut
   KVZtotAv(char* nom);

   virtual ~ KVZtotAv(void);   // destructeur

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVZtotAv, 1)       // Global variable Sum(Z) for Vz > Vcm
};
#endif
