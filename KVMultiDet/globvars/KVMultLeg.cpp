//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Calculates multiplicity of LCP (Z=1,2)
//

#include "KVMultLeg.h"

ClassImp(KVMultLeg)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the number of particules with a charge lower than
// or equal to zmax. The zmax value can be set by calling the SetZmax(z) method.
// The default value of zmax is 2.
//
// Look at KVVarGlob class to have an example of use.
//
/* *///
Int_t KVMultLeg::nb = 0;
Int_t KVMultLeg::nb_crea = 0;
Int_t KVMultLeg::nb_dest = 0;

//_________________________________________________________________
void KVMultLeg::init_KVMultLeg(void)
{
//
// Initialisation des champs de KVMultLeg
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;

   zmax = 2;
   fValueType = 'I'; // integer values
}

//_________________________________________________________________
KVMultLeg::KVMultLeg(void): KVVarGlob1()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVMultLeg();
   sprintf(nom, "KVMultLeg_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVMultLeg
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVMultLeg::KVMultLeg(const Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVMultLeg();
#ifdef DEBUG_KVMultLeg
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVMultLeg::KVMultLeg(const KVMultLeg& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVMultLeg();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVMultLeg&) a).Copy(*this);
#endif
#ifdef DEBUG_KVMultLeg
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVMultLeg::~KVMultLeg(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVMultLeg
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVMultLeg::Copy(TObject& a) const
#else
void KVMultLeg::Copy(TObject& a)
#endif
{
// Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.

#ifdef DEBUG_KVMultLeg
   cout << "Copy de " << GetName() << "..." << endl;
#endif
   KVVarGlob1::Copy(a);
   ((KVMultLeg&) a).SetZmax(GetZmax());
#ifdef DEBUG_KVMultLeg
   cout << "Nom de la Copy (resultat) : " << a.GetName() << endl;
#endif
}

//_________________________________________________________________
KVMultLeg& KVMultLeg::operator =(const KVMultLeg& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVMultLeg
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVMultLeg&) a).Copy(*this);
#endif
#ifdef DEBUG_KVMultLeg
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVMultLeg::Fill(KVNucleus* c)
{
   //The multiplicity is incremented if the Z of nucleus *c is <=zmax
   if (c->GetZ() <= zmax)
      FillVar(1.);
}

//_________________________________________________________________
Int_t KVMultLeg::GetZmax(void) const
{
// Retourne la valeur de Zmax
   return zmax;
}

//_________________________________________________________________
void KVMultLeg::SetZmax(Int_t zm)
{
// Ajuste la valeur de Zmax
   zmax = zm;
}
