//
// D.Cussol
//
// 17/04/2001:
// Creation d'une classe de tri. Cette classe permet d'effectuer un tri et de
// retourner un nombre entier correspondand a une classe donne. Pour un argument
// reel donne entre une valeur min et une valeur max, le trieur retourne un
// nombre compris entre 1 et nb_cases.
//

#include "KVTrieur.h"

//#define DEBUG_KVTrieurLin

class KVTrieurLin: public KVTrieur {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;

   Double_t xmin;
   Double_t xmax;
   Char_t nom_var[80];

// Methodes
protected:
   void initKVTrieurLin(void);  // Initialisations
   void SetNomsCases(void);     // Initialisations des noms de cases

public:
   KVTrieurLin(void);          // constructeur par defaut
   KVTrieurLin(Char_t* nom);
   KVTrieurLin(Int_t nbcases, Char_t* nom);
   KVTrieurLin(const KVTrieurLin& a);  // constructeur par copie

   virtual ~ KVTrieurLin(void);        // destructeur


#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVTrieurLin& operator =(const KVTrieurLin& a);      // operateur =



   virtual Int_t GetNumCase(void* argus ...);   // Pour une situation donnee,
   // retourne le numero de la case
   // correspondante.
   virtual Int_t GetNumCase(Double_t x);        // Pour un x donne,
   // retourne le numero de la case
   // correspondante.

   virtual void SetNbCases(Int_t n);    // ajuste le nombre de cases.
   virtual void SetNomVar(Char_t* x);
   virtual void SetXmin(Double_t x);
   virtual void SetXmax(Double_t x);
   virtual const Char_t* GetNomVar(void);
   virtual Double_t GetXmin(void);
   virtual Double_t GetXmax(void);

   ClassDef(KVTrieurLin, 1)    // Class for linear sorting
};
