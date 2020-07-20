//
//Author: Daniel Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des energies cinetiques
//

#include "KVRiso.h"

ClassImp(KVRiso)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the Riso variable. It is defined as follow:
//
//           Etrans
//   Riso= ----------
//           2*Epar
//
//
//with   Etrans=Sum(etrans(i)) : sum of the transverse energies of the
//                               particules and fragments
//       Epar=Sum(epar(i)) : sum of the parallel energies of the
//                               particules and fragments
//
//By default Riso is calculated from particle momenta in the default (laboratory) frame.
//However, this can be changed by giving the name of the desired frame
//with method SetFrame() :
//      riso->SetFrame( "QP" ); // calculate in "QP" frame
//
// The names of the variables and the corresponding indexes are as follows:
//
//      Name            Index                   Meaning
//----------------------------------------------------------------------------
//      Riso            0       (default)       Riso
//      Epar            1                       Sum of parallel kinetic energies
//      Eperp           2                       Sum of transverse kinetic energies
//
//
// All these values can be obtained by calling the GetValuePtr() method which
// returns an array of Double_t containing the values.
//
// Here are examples on how obtaining values.
//
// KVRiso *priso=new KVRiso();
// KVRiso riso;
// ...
// Double_t r=priso->GetValue();  |
//          r=prsio->GetValue(0); |--> Riso
//          r=priso("Riso");      |
// Double_t epar=riso(1);---------> Sum of parallel kinetic energies
// Double_t etrans=priso->GetValue("Eperp");--> Sum of transverse kinetic
//                                              energies
// Double_t *values=riso.GetValuePtr();
//          r=values[0];      --> Riso
//          epar=values[1];   --> Epar
//          etrans=values[2]; --> Eperp
//
//
// Look at KVVarGlob class to have an example of use.
//
//
/* */
Int_t KVRiso::nb = 0;
Int_t KVRiso::nb_crea = 0;
Int_t KVRiso::nb_dest = 0;

//_________________________________________________________________
void KVRiso::init_KVRiso(void)
{
//
// Initialisation des champs de KVRiso
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;

   Riso = -1;
   Epar = 0.;
   Etrans = 0.;
   SetNameIndex("Riso", 0);
   SetNameIndex("Epar", 1);
   SetNameIndex("Eperp", 2);
}

//_________________________________________________________________
KVRiso::KVRiso(void): KVVarGlob()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVRiso();
   sprintf(nom, "KVRiso_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVRiso
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVRiso::KVRiso(const Char_t* nom): KVVarGlob(nom)
{
//
// Constructeur avec un nom
//
   init_KVRiso();
#ifdef DEBUG_KVRiso
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVRiso::KVRiso(const KVRiso& a): KVVarGlob()
{
//
// Contructeur par Copy
//
   init_KVRiso();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVRiso&) a).Copy(*this);
#endif
#ifdef DEBUG_KVRiso
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVRiso::~KVRiso(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVRiso
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVRiso::Copy(TObject& a) const
#else
void KVRiso::Copy(TObject& a)
#endif
{
// Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.
#ifdef DEBUG_KVRiso
   cout << "Copy de " << GetName() << "..." << endl;
#endif
   KVVarGlob::Copy(a);
   ((KVRiso&) a).Riso = Riso;
   ((KVRiso&) a).Epar = Epar;
   ((KVRiso&) a).Etrans = Etrans;
#ifdef DEBUG_KVRiso
   cout << "Nom de la Copy (resultat) : " << a.GetName() << endl;
#endif
}

//_________________________________________________________________
KVRiso& KVRiso::operator =(const KVRiso& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVRiso
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVRiso&) a).Copy(*this);
#endif
#ifdef DEBUG_KVRiso
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_________________________________________________________________
void KVRiso::Init(void)
{
//
// Initialisations
//
   Riso = -1;
   Epar = 0;
   Etrans = 0;
}


//_________________________________________________________________
void KVRiso::Reset(void)
{
//
// Remise a zero de la variable
//
   Riso = -1;
   Epar = 0;
   Etrans = 0;
}


//_________________________________________________________________

void KVRiso::CalculateRatio(void)
{
   //Calculation of isotropy ratio from sums of parallel and
   //transverse energies
   //Limit to 100

   if (Riso < 0) {
      if (Epar > 0) {
         Riso = TMath::Min(100., Etrans / (2. * Epar));
      }
   }
}

//_________________________________________________________________
Double_t KVRiso::getvalue_void(void) const
{
   // Retourne la valeur de Riso

   //calculate Riso if not already calculated
   const_cast < KVRiso* >(this)->CalculateRatio();
   return Riso;
}

//_________________________________________________________________
Double_t KVRiso::getvalue_int(Int_t i) const
{
//
// Retourne la valeur suivant l'indice
//
//  Index               Meaning
//--------------------------------------------------------
//  0  (default)        Riso
//  1                   Sum of parallel kinetic energies
//  2                   Sum of transverse kinetic energies
//
   switch (i) {
      case 0:
         return getvalue_void();
         break;
      case 1:
         return Epar;
         break;
      case 2:
         return Etrans;
         break;
      default:
         Warning("GetValue(Int_t i)", "Index not valid. Riso returned.");
         return getvalue_void();
         break;
   }
}
//_________________________________________________________________
void KVRiso::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   Riso = -1;
   Double_t ep = c->GetFrame(GetFrame(), kFALSE)->GetKE();
   Double_t et = c->GetFrame(GetFrame(), kFALSE)->GetEtran();

   Epar += (ep - et);
   Etrans += et;
}
