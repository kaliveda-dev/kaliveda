//
// D.Cussol
//
// 20/03/2006:
// Creation de la Variable Globale Ptot
//

#ifndef KVPtot_h
#define KVPtot_h
#include "KVVarGlob.h"

class KVPtot: public KVVarGlob {

protected:
   TVector3 ptot;
   Double_t fNorm;//! normalisation factor

   void init_KVPtot(void);
   Double_t getvalue_int(Int_t) const;
   Double_t getvalue_void(void) const;

public:
   KVPtot(void);               // constructeur par defaut
   KVPtot(const Char_t* nom, const Char_t* frm = "");
   KVPtot(const KVPtot& a);    // constructeur par Copy

   virtual ~ KVPtot(void);     // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVPtot& operator =(const KVPtot& a);        // operateur =

   virtual void Init(void);     // methode d'initialisation des
   // variables Internes
   virtual void Reset(void);    // Remise a zero avant le
   // traitement d'un evenement
   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   std::vector<Double_t> GetValueVector() const; // On retourne le tableau des valeurs

   virtual TVector3 GetTVector3(void) const;    // on retourne le TVector3
   virtual TObject* GetObject(void) const
   {
      return (TObject*)&ptot;
   }

   ClassDef(KVPtot, 1)         // Global variable Ptot=Sum(p(i))
};
#endif
