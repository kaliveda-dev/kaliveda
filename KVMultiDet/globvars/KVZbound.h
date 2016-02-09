//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVZbound_h
#define KVZbound_h
#include "KVVarGlob1.h"

//#define DEBUG_KVZbound

class KVZbound: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Champ propre:
protected:
   Int_t zmin;
   Int_t zmax;
   Double_t vmin;
   Double_t vmax;

// Methodes
protected:
   void init_KVZbound(void);
   enum {
      kZMin = BIT(14),
      kZMax = BIT(15),
      kZBoth = BIT(16),
      kVMin = BIT(17),
      kVMax = BIT(18),
      kVBoth = BIT(19),
      kVTest = BIT(20)
   };

public:
   KVZbound(void);             // constructeur par defaut
   KVZbound(Char_t* nom);
   KVZbound(const KVZbound& a);        // constructeur par Copy

   virtual ~ KVZbound(void);   // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVZbound& operator =(const KVZbound& a);    // operateur =

   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   virtual Int_t GetZmin(void) const;
   virtual void SetZmin(Int_t zm);

   virtual void Init(void);

   ClassDef(KVZbound, 1)       // Global variable Sum(Z) for Z >= Zmin and/or Z <= Zmax
};
#endif
