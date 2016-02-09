//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des energies cinetiques
//
#include "KVMultIMF.h"

ClassImp(KVMultIMF)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the number of fragments with a charge greater than
// or equal to zmin. The zmin value can be set by calling the SetZmin(z) method.
// The default value of zmin is 3.
//
// Look at KVVarGlob class to have an example of use.
//
/* *///
Int_t KVMultIMF::nb = 0;
Int_t KVMultIMF::nb_crea = 0;
Int_t KVMultIMF::nb_dest = 0;

//_________________________________________________________________
void KVMultIMF::init_KVMultIMF(void)
{
//
// Initialisation des champs de KVMultIMF
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;

   SetZmin(3);
}

//_________________________________________________________________
KVMultIMF::KVMultIMF(void): KVZbound()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVMultIMF();
   sprintf(nom, "KVMultIMF_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVMultIMF
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVMultIMF::KVMultIMF(Char_t* nom): KVZbound(nom)
{
//
// Constructeur avec un nom
//
   init_KVMultIMF();
#ifdef DEBUG_KVMultIMF
   cout << nb << " crees...(nom) " << endl;
#endif
}


//_________________________________________________________________
KVMultIMF::~KVMultIMF(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVMultIMF
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
void KVMultIMF::Fill(KVNucleus* c)
{
   //The multiplicity is incremented if the Z of nucleus *c is >=zmin
   if (c->GetZ() >= zmin)
      FillVar(1.);
}
