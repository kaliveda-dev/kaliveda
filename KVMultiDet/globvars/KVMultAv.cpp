//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des energies cinetiques
//

#include "KVMultAv.h"

ClassImp(KVMultAv)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the multiplicity of particules and fragments with
//  Vz > 0 in whatever frame the user chooses with KVVarGlob::SetFrame().
//   By default the "CM" frame is used.
//
// Look at KVVarGlob class to have an example of use.
//
//
/* *///
Int_t KVMultAv::nb = 0;
Int_t KVMultAv::nb_crea = 0;
Int_t KVMultAv::nb_dest = 0;

//_________________________________________________________________
void KVMultAv::init_KVMultAv(void)
{
//
// Initialisation des champs de KVMultAv
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
   SetFrame("CM");
   fValueType = 'I'; //integer values
}

//_________________________________________________________________
KVMultAv::KVMultAv(void): KVVarGlob1()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVMultAv();
   sprintf(nom, "KVMultAv_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVMultAv
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVMultAv::KVMultAv(Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVMultAv();
#ifdef DEBUG_KVMultAv
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVMultAv::KVMultAv(const KVMultAv& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVMultAv();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVMultAv&) a).Copy(*this);
#endif
#ifdef DEBUG_KVMultAv
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVMultAv::~KVMultAv(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVMultAv
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
KVMultAv& KVMultAv::operator =(const KVMultAv& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVMultAv
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVMultAv&) a).Copy(*this);
#endif
#ifdef DEBUG_KVMultAv
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVMultAv::Fill(KVNucleus* c)
{
   //The multiplicity is incremented if the projection of the particle's velocity
   //along the 'z'-direction in the currently defined frame is >0.
   if (c->GetFrame(fFrame.Data())->GetVpar() > 0) FillVar(1.);
}
