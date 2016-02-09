//
// D.Cussol
//
// 17/04/2001:
// Creation d'une classe de tri. Cette classe permet d'effectuer un tri et de
// retourner un nombre entier correspondand a une classe donne. Ceci est une
// classe virtuelle. La classe contient une methode virtuelle qui devra etre
// fournie par l'utilisateur lors de son implementation.
//

#ifndef KVTrieur_h
#define KVTrieur_h

#include "Riostream.h"
#include "KVBase.h"
#include "TClonesArray.h"

//#define DEBUG_KVTrieur

class KVTrieur: public KVBase {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;

// Champs publics:
   Int_t nb_cases;
   TClonesArray* noms_cases;

// Methodes
protected:
   void initKVTrieur(void);     // Initialisations

public:
   KVTrieur(void);             // constructeur par defaut
   KVTrieur(Char_t* nom);
   KVTrieur(Int_t nbcases, Char_t* nom);
   KVTrieur(const KVTrieur& a);        // constructeur par copie

   virtual ~ KVTrieur(void);   // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   KVTrieur& operator =(const KVTrieur& a);    // operateur =

   virtual const Char_t* GetNomCase(Int_t i);
   // Retourne le nom d'une case.
   virtual void SetNomCase(Int_t i, Char_t* s);
   // Retourne le nom d'une case.
   virtual void SetNbCases(Int_t n);
   // ajuste le nombre de cases.
   virtual Int_t GetNbCases(void);
   // Retourne le nombre de cases.
   virtual Int_t GetNumCase(Double_t x) = 0;
   // Retourne le numero de la case correspondant a x
   virtual Int_t GetNumCase(void*, ...) = 0;
   // Pour une situation donnee, retourne le numero de la case correspondante.

   ClassDef(KVTrieur, 1)       // Base class for sortings
};
#endif
