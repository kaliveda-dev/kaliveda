//
//Author: Daniel Cussol
//
// 18/02/2004
// Creation d'une classe Variable Globale.
// Retourne le Zmax de l'enenemnt et le pointeur du fragment le plus lourd
#include <math.h>
#include <stdio.h>
#include "KVZmax.h"

ClassImp(KVZmax)
//////////////////////////////////////////////////////////////////////////////////
//  Global variable returning the charge of the (i+1)th heaviest fragment
//  and the pointer to the (i+1)th heaviest fragment, where i=0,1,...,M-1
//  with M = total multiplicity of event.
//
//  All nuclei of the event are sorted in descending order according to their charge
//  (Z(0) > Z(1) > Z(2) > ... > Z(M-1)) .
//
//  Simple interface "GetValue()" returns Z of heaviest fragment
//  and "GetObject" returns pointer to this fragment.
//
//  All other fragments accessible via "GetValue(i)" and "GetZmax(i)", i ranging
//  from 0 to M-1 (M=total multiplicity)
//  (the latter method returns a KVNucleus pointer directly).
//
//  Also GetValue("Zmax1"), GetValue("Zmax2"), ..., GetValue("Zmax50")
//  can be used to obtain Z of 50 heaviest nuclei.
///////////////////////////////////////////////////////////////////////////////////

Int_t KVZmax::nb = 0;
Int_t KVZmax::nb_crea = 0;
Int_t KVZmax::nb_dest = 0;

//_________________________________________________________________
void KVZmax::init_KVZmax(void)
{
//
// Initialisation des champs de KVZmax
// Cette methode privee n'est appelee que par les createurs
//
   nb++;
   nb_crea++;
   heaviest = 0;
   fSorted = kFALSE;
   //set up list of indices
   for (register int i = 1; i <= 50; i++)
      SetNameIndex(Form("Zmax%d", i), i - 1);
   fValueType = 'I'; // integer values
   SetMaxNumBranches(3);
}

//_________________________________________________________________
KVZmax::KVZmax(void): KVVarGlob1()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init_KVZmax();
   sprintf(nom, "KVZmax_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVZmax
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVZmax::KVZmax(Char_t* nom): KVVarGlob1(nom)
{
//
// Constructeur avec un nom
//
   init_KVZmax();
#ifdef DEBUG_KVZmax
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVZmax::KVZmax(const KVZmax& a) : KVVarGlob1()
{
//
// Contructeur par Copy
//
   init_KVZmax();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZmax&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZmax
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVZmax::~KVZmax(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVZmax
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
   if (heaviest) {
      heaviest->Clear("nodelete");
      delete heaviest;
      heaviest = 0;
   }
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVZmax::Copy(TObject& a) const
#else
void KVZmax::Copy(TObject& a)
#endif
{
// Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.

#ifdef DEBUG_KVZmax
   cout << "Copy de " << GetName() << "..." << endl;
#endif
   ((KVZmax&) a).Reset();
   KVVarGlob1::Copy(a);

   //copy list of fragments if it exists
   if (heaviest) {
      KVNucleus* tmp;
      TIter next(heaviest);
      while ((tmp = (KVNucleus*) next()))
         ((KVZmax&) a).Fill(tmp);
   }
#ifdef DEBUG_KVZmax
   cout << "Nom de la Copy (resultat) : " << a.GetName() << endl;
#endif
}

//_________________________________________________________________
KVZmax& KVZmax::operator =(const KVZmax& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVZmax
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVZmax&) a).Copy(*this);
#endif
#ifdef DEBUG_KVZmax
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_________________________________________________________________
void KVZmax::Init(void)
{
//
// Remise a Zero de la variable et du pointeur
//
   KVVarGlob1::Init();
   if (heaviest) {
      heaviest->SetOwner(kFALSE);
      heaviest->Clear("nodelete");
   }
   fSorted = kFALSE;
}

//_________________________________________________________________
void KVZmax::Reset(void)
{
//
// Remise a Zero de la variable et du pointeur
//
   KVVarGlob1::Reset();
   if (heaviest) {
      heaviest->Clear("nodelete");
   }
   fSorted = kFALSE;
}

//_________________________________________________________________
void KVZmax::Fill(KVNucleus* c)
{
   //We use the TList sorting mechanism in order to rank the fragments.
   //Each new nucleus has its pointer added to the TList.

   if (!heaviest) {
      heaviest = new TList;
      heaviest->SetOwner(kFALSE);
   }
   heaviest->Add(c);
}

//_________________________________________________________________
KVNucleus* KVZmax::GetZmax(Int_t i)
{
   //Pointer to (i+1)th heaviest fragment (i=0 : Zmax, i=1 : Zmax2, etc.)

   if (!heaviest)
      return 0;
   if (!fSorted) {
      heaviest->Sort();
      fSorted = kTRUE;
   }
   return (KVNucleus*) heaviest->At(i);
}

//_________________________________________________________________
Double_t KVZmax::getvalue_int(Int_t i)
{
   //returns the Z of the (i+1)th heaviest fragment
   // i=0 : Zmax
   // i=1 : Zmax2
   // etc.

   if (!GetZmax(i))
      return -1.0;
   return (Double_t) GetZmax(i)->GetZ();
}

//_________________________________________________________________
Double_t KVZmax::getvalue_void(void) const
{
   //Returns Z of heaviest fragment
   return (const_cast < KVZmax* >(this)->getvalue_int(0));
}

//_________________________________________________________________
Double_t* KVZmax::GetValuePtr(void)
{
   //Return array containing ordered list of Z (all fragments)
   //
   //  USER MUST DELETE ARRAY AFTER USING !!!

   if (!heaviest)
      return 0;
   UInt_t size_event = heaviest->GetSize();
   if (!fSorted) {
      heaviest->Sort();
      fSorted = kTRUE;
   }
   Double_t* v = new Double_t[size_event];
   for (UInt_t u = 0; u < size_event; u++)
      v[u] = GetValue(u);
   return v;
}
