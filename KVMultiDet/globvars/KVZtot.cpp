//
//Author: Daniel Cussol
//
// 17/02/2004
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Elle retoune la somme des charges de l'evenement
//

#include <math.h>
#include <stdio.h>
#include "KVZtot.h"

ClassImp(KVZtot)
//////////////////////////////////////////////////////////////////////////////////
//  Global variable returning the sum of the fragments charges
//  Ztot
//
// Look at KVVarGlob class to have an example of use.
//
int KVZtot::nb = 0;
int KVZtot::nb_crea = 0;
int KVZtot::nb_dest = 0;

//_________________________________________________________________
void KVZtot::init_KVZtot(void)
{
//
// Initialisation des champs de KVZtot
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
   fValueType = 'I'; //integer values
}

//_________________________________________________________________
KVZtot::KVZtot(void): KVVarGlob1()
{
//
// Createur par default
//
   char* nom = new char[80];

   init_KVZtot();
   sprintf(nom, "KVZtot_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVZtot
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVZtot::KVZtot(const char* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVZtot();
#ifdef DEBUG_KVZtot
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVZtot::KVZtot(const KVZtot& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVZtot();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZtot&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZtot
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVZtot::~KVZtot(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZtot
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}


//_________________________________________________________________
KVZtot& KVZtot::operator =(const KVZtot& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVZtot
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZtot&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZtot
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVZtot::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   FillVar(c->GetZ());
}
