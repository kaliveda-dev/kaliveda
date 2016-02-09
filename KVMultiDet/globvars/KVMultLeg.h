//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVMultLeg_h
#define KVMultLeg_h
#include "KVVarGlob1.h"

//#define DEBUG_KVMultLeg

class KVMultLeg: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Champ propre:
protected:
   Int_t zmax;
// Methodes
protected:
   void init_KVMultLeg(void);

public:
   KVMultLeg(void);            // constructeur par defaut
   KVMultLeg(Char_t* nom);
   KVMultLeg(const KVMultLeg& a);      // constructeur par Copy

   virtual ~ KVMultLeg(void);  // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVMultLeg& operator =(const KVMultLeg& a);          // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   virtual Int_t GetZmax(void) const;
   virtual void SetZmax(Int_t zm);

   ClassDef(KVMultLeg, 1)      // Multiplicity for particules with Z <= Zmax
};
#endif
