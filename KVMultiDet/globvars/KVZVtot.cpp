//
//Author: Daniel Cussol
//
// 20/03/2006:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des moments
//

#include <math.h>
#include <stdio.h>
#include "KVZVtot.h"

ClassImp(KVZVtot)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the ZVtot variable. It is defined as follow:
//
//   ZVtot= Sum(Z(i)*v(i))
//
//
//with   v(i) : velocity of particle i; (in cm/ns)
//       Z(i) : charge of particle i;
//
//The units of ZVtot are therefore [cm/ns].
//
//By default ZVtot is calculated from particle velocities in the default (laboratory) frame.
//However, this can be changed by giving the name of the desired frame
//to either the constructor
//      KVZVtot* zvtot = new KVZVtot("my_ZVtot", "CM"); // calculate in "CM" frame
//or by using KVVarGlob method SetFrame():
//      zvtot->SetFrame( "QP" ); // calculate in "QP" frame
//
// The names of the variables and the corresponding indexes are as follows:
//
//      Name            Index                   Meaning
//----------------------------------------------------------------------------
//      Z               0       (default)       Z component of ZVtot
//      X               1                       X component of ZVtot
//      Y               2                       Y component of ZVtot
//
//
// All these values can be obtained by calling the GetValuePtr() method which
// returns an array of Double_t containing the values.
//
// The vector (TVector3) can be used by calling the GetTVector3() method.
//
// Here are examples on how obtaining values.
//
// KVZVtot *ZVtot_ptr=new KVZVtot();
// KVZVtot ZVtot;
// ...
// Double_t r=zvtot_ptr->GetValue();  |
//          r=zvtot_ptr->GetValue(0); |--> z component of ZVtot
//          r=zvtot("Z");             |
// Double_t zvx=zvtot("X");---------> X component of total Z*v
// Double_t zvtottrans=zvtot_ptr->GetTVector3().GetPerp();--> Sum of Z*v_trans
// Double_t *values=.GetValuePtr();
//          zvz=values[0];   --> z component of Sum(Z*v)
//          zvx=values[1];   --> x component of Sum(Z*v)
//          zvy=values[2];   --> y component of Sum(Z*v)
//
//
// Look at KVVarGlob class to have an example of use.
//
//
/* */
Int_t KVZVtot::nb = 0;
Int_t KVZVtot::nb_crea = 0;
Int_t KVZVtot::nb_dest = 0;

//_________________________________________________________________
void KVZVtot::init_KVZVtot(void)
{
//
// Initialisation des champs de KVZVtot
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
}

//_________________________________________________________________
KVZVtot::KVZVtot(void): KVPtot()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVZVtot();
   sprintf(nom, "KVZVtot_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVZVtot
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVZVtot::KVZVtot(Char_t* nom, const Char_t* frm): KVPtot(nom, frm)
{
//
// Constructeur avec un nom
//
   init_KVZVtot();
#ifdef DEBUG_KVZVtot
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVZVtot::KVZVtot(const KVZVtot& a) : KVPtot()
{
//
// Contructeur par Copy
//
   init_KVZVtot();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZVtot&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZVtot
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVZVtot::~KVZVtot(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZVtot
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVZVtot::Copy(TObject& a) const
#else
void KVZVtot::Copy(TObject& a)
#endif
{
// Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.
#ifdef DEBUG_KVZVtot
   cout << "Copy de " << GetName() << "..." << endl;
#endif
   KVPtot::Copy(a);
#ifdef DEBUG_KVZVtot
   cout << "Nom de la Copy (resultat) : " << a.GetName() << endl;
#endif
}

//_________________________________________________________________
KVZVtot& KVZVtot::operator =(const KVZVtot& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVZVtot
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZVtot&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZVtot
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}


//_________________________________________________________________
void KVZVtot::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   ptot += (c->GetZ() * c->GetFrame(fFrame.Data())->GetVelocity());
}
