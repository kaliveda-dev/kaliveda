//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVMultLegAv_h
#define KVMultLegAv_h
#include "KVMultLeg.h"

//#define DEBUG_KVMultLeg

class KVMultLegAv: public KVMultLeg {
// Methodes
public:
   KVMultLegAv(void);           // constructeur par defaut
   KVMultLegAv(char* nom);

   virtual ~ KVMultLegAv(void);        // destructeur

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVMultLegAv, 1)    // Multiplicity for particules with Z <= Zmax and Vz > Vcm
};
#endif
