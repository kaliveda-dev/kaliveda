//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des energies cinetiques
//

#include "KVEkin.h"

ClassImp(KVEkin)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the sum of the kinetic energies of fragments
//   in the reference frame chosen by the user with KVVarGlob::SetFrame().
//   Default frame is laboratory.

Int_t KVEkin::nb = 0;
Int_t KVEkin::nb_crea = 0;
Int_t KVEkin::nb_dest = 0;

//_________________________________________________________________
void KVEkin::init_KVEkin(void)
{
//
// Initialisation des champs de KVEkin
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
}

//_________________________________________________________________
KVEkin::KVEkin(void): KVVarGlob1()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVEkin();
   sprintf(nom, "KVEkin_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVEkin
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVEkin::KVEkin(Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVEkin();
#ifdef DEBUG_KVEkin
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVEkin::KVEkin(const KVEkin& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVEkin();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVEkin&) a).Copy(*this);
#endif
#ifdef DEBUG_KVEkin
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVEkin::~KVEkin(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVEkin
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}


//_________________________________________________________________
KVEkin& KVEkin::operator =(const KVEkin& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVEkin
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVEkin&) a).Copy(*this);
#endif
#ifdef DEBUG_KVEkin
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVEkin::Fill(KVNucleus* c)
{
   //Add particle's kinetic energy in frame chosen with SetFrame()
   //to the sum of kinetic energies
   FillVar(c->GetFrame(fFrame.Data())->GetKE());
}
