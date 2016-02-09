//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVRisoAv_h
#define KVRisoAv_h
#include "KVRiso.h"

//#define DEBUG_KVRiso

class KVRisoAv: public KVRiso {
// Methodes
public:
   KVRisoAv(void);              // constructeur par defaut
   KVRisoAv(char* nom);

   virtual ~ KVRisoAv(void);   // destructeur

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVRisoAv, 1)       // Global variable Riso
};
#endif
