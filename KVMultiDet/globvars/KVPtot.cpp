//
//Author: Daniel Cussol
//
// 20/03/2006:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Retourne la somme des moments
//

#include "KVPtot.h"

ClassImp(KVPtot)
//////////////////////////////////////////////////////////////////////////////////
//   Global variable returning the Ptot variable. It is defined as follow:
//
//   Ptot= Sum(p(i))
//
//
//with   p(i) : momentum of particle i;
//
//By default Ptot is calculated from particle momenta
//in the default (laboratory) frame.
//However, this can be changed by giving the name of the desired frame
//to either the constructor
//      KVPtot* ptot = new KVPtot("my_Ptot", "CM"); // calculate in "CM" frame
//or by using KVVarGlob method SetFrame()
//      ptot->SetFrame( "QP" ); // calculate in "QP" frame
//
// The names of the variables and the corresponding indexes are as follows:
//
//      Name            Index                   Meaning
//----------------------------------------------------------------------------
//      Z               0       (default)       Z component of Ptot
//      X               1                       X component of Ptot
//      Y               2                       Y component of Ptot
//
//
// All these values can be obtained by calling the GetValuePtr() method which
// returns an array of Double_t containing the values.
//
// The vector (TVector3) can be used by calling the GetTVector3() method.
//
// If you want to normalize the values e.g. to the momentum of the projectile
// in the laboratory frame, set the parameter "Normalization"

//_________________________________________________________________
void KVPtot::init_KVPtot(void)
{
//
// Initialisation des champs de KVPtot
// Cette methode privee n'est appelee par les createurs
//

   ptot.SetXYZ(0, 0, 0);
   SetNameIndex("Z", 0);
   SetNameIndex("X", 1);
   SetNameIndex("Y", 2);
}

//_________________________________________________________________
KVPtot::KVPtot(void): KVVarGlob()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVPtot();
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVPtot
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVPtot::KVPtot(const Char_t* nom, const Char_t* frm): KVVarGlob(nom)
{
   // Create KVPtot with name "nom" and using Lorentz frame whose name is "frame" to calculate
   // the sum of momenta. If "frame" is not given, the laboratory reference frame is assumed.

   init_KVPtot();
#ifdef DEBUG_KVPtot
   cout << nb << " crees...(nom) " << endl;
#endif
   if (strcmp(frm, ""))
      SetFrame(frm);
}

//_________________________________________________________________
KVPtot::KVPtot(const KVPtot& a) : KVVarGlob()
{
//
// Contructeur par Copy
//
   init_KVPtot();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVPtot&) a).Copy(*this);
#endif
#ifdef DEBUG_KVPtot
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVPtot::~KVPtot(void)
{}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVPtot::Copy(TObject& a) const
#else
void KVPtot::Copy(TObject& a)
#endif
{
// Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.
#ifdef DEBUG_KVPtot
   cout << "Copy de " << GetName() << "..." << endl;
#endif
   KVVarGlob::Copy(a);
   ((KVPtot&) a).ptot = ptot;
#ifdef DEBUG_KVPtot
   cout << "Nom de la Copy (resultat) : " << a.GetName() << endl;
#endif
}

//_________________________________________________________________
KVPtot& KVPtot::operator =(const KVPtot& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVPtot
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVPtot&) a).Copy(*this);
#endif
#ifdef DEBUG_KVPtot
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_________________________________________________________________
void KVPtot::Init(void)
{
//
// Initialisations
//
   Info("Init", "called");
   ptot.SetXYZ(0., 0., 0.);
   if (IsParameterSet("Normalization")) {
      fNorm = GetParameter("Normalization");
      Info("Init", "Normalization factor = %g", fNorm);
   }
   else fNorm = 1;
}


//_________________________________________________________________
void KVPtot::Reset(void)
{
//
// Remise a zero de la variable
//
   ptot.SetXYZ(0., 0., 0.);
}

//_________________________________________________________________

Double_t KVPtot::getvalue_void() const
{
   // Retourne la valeur du Z component of Ptot
   return ptot.Z();
}

//_________________________________________________________________
Double_t KVPtot::getvalue_int(Int_t i) const
{
//
// Retourne la valeur suivant l'indice
//
//   Index                   Meaning
//---------------------------------------------------------
//   0       (default)       Z component of Ptot
//   1                       X component of Ptot
//   2                       Y component of Ptot
//
   switch (i) {
      case 0:
         return ptot.Z();
         break;
      case 1:
         return ptot.X();
         break;
      case 2:
         return ptot.Y();
         break;
      default:
         Warning("GetValue(Int_t i)", "Index not valid. Pz returned.");
         return ptot.Z();
         break;
   }
}

//_________________________________________________________________
std::vector<Double_t> KVPtot::GetValuePtr() const
{
// On retourne un tableau de valeurs. il est organise comme suit
//
//   Index    Meaning
//------------------------------------------
//   0        Z component of Ptot
//   1        X component of Ptot
//   2        Y component of Ptot
//
   std::vector<Double_t> tmp;
   for (Int_t i = 0; i < 3; i++) {
      tmp.push_back(GetValue(i));
   }
   return tmp;
}

//_________________________________________________________________
void KVPtot::Fill(KVNucleus* c)
{
//
// Routine de remplissage
//
   //ptot += ((1. / fNorm) * c->GetFrame(fFrame.Data(), kFALSE)->GetMomentum());
}

//_________________________________________________________________
TVector3 KVPtot::GetTVector3(void) const
{
//
// Return total momentum vector (TVector3 object)
//
   return ptot;
}
