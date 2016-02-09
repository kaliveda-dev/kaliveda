//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVMultIMFAv_h
#define KVMultIMFAv_h
#include "KVMultIMF.h"

//#define DEBUG_KVMultIMF

class KVMultIMFAv: public KVMultIMF {
// Methodes
public:
   KVMultIMFAv(void);           // constructeur par defaut
   KVMultIMFAv(char* nom);

   virtual ~ KVMultIMFAv(void);        // destructeur

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   ClassDef(KVMultIMFAv, 1)    // Multiplicity for fragments with Z >= Zmin and Vz > Vcm
};
#endif
