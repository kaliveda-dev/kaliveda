//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle retourne la somme des energies transverses
//

#include "KVEtrans.h"

ClassImp(KVEtrans)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the sum of the tranverse energies of fragments
//   in the reference frame chosen by the user with KVVarGlob::SetFrame().
//   Default frame is laboratory.

Int_t KVEtrans::nb = 0;
Int_t KVEtrans::nb_crea = 0;
Int_t KVEtrans::nb_dest = 0;

//_________________________________________________________________
void KVEtrans::init_KVEtrans(void)
{
//
// Initialisation des champs de KVEtrans
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
}

//_________________________________________________________________
KVEtrans::KVEtrans(void): KVVarGlob1()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVEtrans();
   sprintf(nom, "KVEtrans_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVEtrans
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVEtrans::KVEtrans(const Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVEtrans();
#ifdef DEBUG_KVEtrans
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVEtrans::KVEtrans(const KVEtrans& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVEtrans();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVEtrans&) a).Copy(*this);
#endif
#ifdef DEBUG_KVEtrans
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVEtrans::~KVEtrans(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVEtrans
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}


//_________________________________________________________________
KVEtrans& KVEtrans::operator =(const KVEtrans& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVEtrans
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVEtrans&) a).Copy(*this);
#endif
#ifdef DEBUG_KVEtrans
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVEtrans::Fill(KVNucleus* c)
{
   //Add particle's transverse kinetic energy in frame chosen with SetFrame()
   //to the sum of transverse energies
   FillVar(c->GetFrame(fFrame.Data(), kFALSE)->GetEtran());
}
