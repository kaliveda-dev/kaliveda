//
// D.Cussol
//
// 17/04/2001:
// Creation d'une classe de tri. Cette classe permet d'effectuer un tri et de
// retourner un nombre entier correspondand a une classe donne. Pour un argument
// reel donne le trieur retourne un
// nombre correspondant a l'intervalle de la tranche.
//

#include "KVTrieur.h"
#include "TVector.h"

//#define DEBUG_KVTrieurTranche

class KVTrieurTranche: public KVTrieur {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;

   TVector xtranches;
   Char_t nomVar[80];

// Methodes
protected:
   void initKVTrieurTranche(void);      // Initialisations
   void SetNomsCases(void);     // Initialisations des noms de cases

public:
   KVTrieurTranche(void);      // constructeur par defaut
   KVTrieurTranche(Char_t* nom);
   KVTrieurTranche(Int_t nbcases, Char_t* nom);
   KVTrieurTranche(const KVTrieurTranche& a);  // constructeur par copie

   virtual ~ KVTrieurTranche(void);    // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   KVTrieurTranche& operator =(const KVTrieurTranche& a);      // operateur =



   virtual Int_t GetNumCase(void* argus ...);   // Pour une situation donnee,
   // retourne le numero de la case
   // correspondante.
   virtual Int_t GetNumCase(Double_t x);        // Pour un x donne,
   // retourne le numero de la case
   // correspondante.

   virtual void SetNomVar(Char_t* x);
   virtual void SetXmin(Int_t i, Double_t x);
   virtual void SetXmax(Int_t i, Double_t x);
   virtual const Char_t* GetNomVar(void);
   virtual Double_t GetXmin(Int_t i);
   virtual Double_t GetXmax(Int_t i);
   virtual void SetNbCases(Int_t n);    // ajuste le nombre de cases.

   ClassDef(KVTrieurTranche, 1)        // Class for sorting with attached cells
};
