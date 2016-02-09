//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVZboundAv_h
#define KVZboundAv_h
#include "KVZbound.h"

//#define DEBUG_KVZbound

class KVZboundAv: public KVZbound {
// Methodes
public:
   KVZboundAv(void);            // constructeur par defaut
   KVZboundAv(char* nom);

   virtual ~ KVZboundAv(void); // destructeur

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVZboundAv, 1)     // Global variable Sum(Z) for Z>=Zmin and/or Z<=Zmax and Vz > 0
};
#endif
