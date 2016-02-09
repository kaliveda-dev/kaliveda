//
//Author: Daniel Cussol
//
// 18/02/2004:
// Creation d'une classe Variable Globale.
// Elle retourne la valeur moyenne et l'ecart-type des Char_tges
//
#include <math.h>
#include <stdio.h>
#include "KVZmean.h"

ClassImp(KVZmean)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the mean value and the standard deviation
//  of the charge.
//
// Look at KVVarGlobMean class to have an example of use.
//
//
Int_t KVZmean::nb = 0;
Int_t KVZmean::nb_crea = 0;
Int_t KVZmean::nb_dest = 0;

//_________________________________________________________________
void KVZmean::init_KVZmean(void)
{
//
// Initialisation des champs de KVZmean
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
}

//_________________________________________________________________
KVZmean::KVZmean(void): KVVarGlobMean()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVZmean();
   sprintf(nom, "KVZmean_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVZmean
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVZmean::KVZmean(Char_t* nom): KVVarGlobMean(nom)
{
//
// Constructeur avec un nom
//
   init_KVZmean();
#ifdef DEBUG_KVZmean
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVZmean::KVZmean(const KVZmean& a) : KVVarGlobMean()
{
//
// Contructeur par Copy
//
   init_KVZmean();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZmean&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZmean
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVZmean::~KVZmean(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZmean
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
KVZmean& KVZmean::operator =(const KVZmean& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVZmean
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZmean&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZmean
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVZmean::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   FillVar(c->GetZ());
}
