/**
\class KVTrieur
\brief Base class for Sorting purposes
\ingroup Analysis

   This class is a base abstact class for the management of sortings. A
  sorting class deriving from KVTrieur has to provide the following methods:
~~~~{.cpp}
      virtual Int_t GetNumCase(Double_t x)   Gives an index corresponding to
                                             the value of x.
      virtual Int_t GetNumCase(void *,...)   Gives an index corresponding to
                                             the list of arguments.
~~~~
  Child classes KVTrieurLin, KVTrieurTranche and KVTrieurBloc provide
 these methods. A name can be associated to each index value with the method
~~~~{.cpp}
      virtual void SetNomCase(Int_t i,Char_t *s)
~~~~
 and can be retrieved withe the method
~~~~{.cpp}
      virtual const Char_t *GetNomCase(Int_t i)
~~~~
 The number of indexes can be set with the method
~~~~{.cpp}
      virtual void SetNbCases(Int_t n)
~~~~
*/

#ifndef KVTrieur_h
#define KVTrieur_h

#include "Riostream.h"
#include "KVBase.h"
#include "TClonesArray.h"

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
