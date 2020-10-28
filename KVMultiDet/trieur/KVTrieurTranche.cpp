#include <math.h>
#include <stdio.h>
#include "KVTrieurTranche.h"

using namespace std;

ClassImp(KVTrieurTranche)

Int_t KVTrieurTranche::nb = 0;
Int_t KVTrieurTranche::nb_crea = 0;
Int_t KVTrieurTranche::nb_dest = 0;

//_____________________________________________________
void KVTrieurTranche::initKVTrieurTranche(void)
{
//
// Initialisation des champs de KVTrieurTranche
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
   sprintf(nomVar, "Variable");
}


//_____________________________________________________
void KVTrieurTranche::SetNomsCases(void)
{
//
// On affecte les noms des cases
//
   if (noms_cases) {
      for (Int_t i = 0; i < nb_cases; i++) {
         TNamed* nom = (TNamed*) noms_cases->At(i);
         nom->SetTitle(Form("%f #leq %s < %f", xtranches(i), nomVar,
                            xtranches(i + 1)));
      }
   }
}

//_____________________________________________________
KVTrieurTranche::KVTrieurTranche(void): KVTrieur()
{
//
// Createur par default
//
   TString nom;

   initKVTrieurTranche();
   nom.Form("KVTrieurTranche_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurTranche
   cout << nb << " crees...(defaut) " << endl;
#endif
}

//_____________________________________________________
KVTrieurTranche::KVTrieurTranche(Char_t* nom): KVTrieur(nom)
{
//
// Constructeur avec un nom
//
   initKVTrieurTranche();
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurTranche
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_____________________________________________________
KVTrieurTranche::KVTrieurTranche(Int_t nbcases, Char_t* nom): KVTrieur(nbcases,
         nom)
{
//
// Constructeur avec un nombre de cases et un nom
//
   initKVTrieurTranche();
   xtranches.ResizeTo(nb_cases + 1);
#ifdef DEBUG_KVTrieurTranche
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_____________________________________________________
KVTrieurTranche::KVTrieurTranche(const KVTrieurTranche& a) : KVTrieur()
{
//
// Contructeur par copie
//
   initKVTrieurTranche();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieurTranche&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieurTranche
   cout << nb << " crees...(copie) " << endl;
#endif
}

//_____________________________________________________
KVTrieurTranche::~KVTrieurTranche(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVTrieurTranche
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;
   nb_dest++;
}

//_____________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTrieurTranche::Copy(TObject& a) const
{
#else
void KVTrieurTranche::Copy(TObject& a)
{
#endif
//
// Methode de Copie
//
#ifdef DEBUG_KVTrieurTranche
   cout << "Copie de " << a.GetName() << "..." << endl;
#endif
   KVTrieur::Copy(a);
   ((KVTrieurTranche&) a).xtranches.ResizeTo(this->nb_cases + 1);
   ((KVTrieurTranche&) a).xtranches = this->xtranches;
   sprintf(((KVTrieurTranche&) a).nomVar, "%s", this->nomVar);
#ifdef DEBUG_KVTrieurTranche
   cout << "Nom de la copie (arguement): " << this->nomVar << endl;
   cout << "Nom de la copie (resultat) : " << ((KVTrieurTranche&) a).
        GetName() << endl;
#endif
}

//_____________________________________________________
KVTrieurTranche& KVTrieurTranche::operator =(const KVTrieurTranche& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVTrieurTranche
   cout << "Copie par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieurTranche&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieurTranche
   cout << "Nom de la copie par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_____________________________________________________
Int_t KVTrieurTranche::GetNumCase(void* ...)
{
//
// On retourne le numero de case
//
   cout << "Utiliser KVTrieurTranche::GetNumCase(Double_t x)" << endl;
   return -1;
}

//_____________________________________________________
Int_t KVTrieurTranche::GetNumCase(Double_t x)
{
//
// On retourne le numero de case
//
   Int_t i = -1;
   while ((i < nb_cases) && (x >= xtranches(i + 1))) {
      i++;
   }
   if (i >= nb_cases)
      i = -1;
   return i + 1;
}

//_____________________________________________________
void KVTrieurTranche::SetXmin(Int_t i, Double_t x)
{
//
// On met la valeur de Xmin
//
   xtranches(i - 1) = x;
   SetNomsCases();
}

//_____________________________________________________
Double_t KVTrieurTranche::GetXmin(Int_t i)
{
//
// On retourne la valeur de xmin
   return xtranches(i - 1);
}

//_____________________________________________________
void KVTrieurTranche::SetXmax(Int_t i, Double_t x)
{
//
// On met la valeur de Xmin
//
   xtranches(i) = x;
   SetNomsCases();
}

//_____________________________________________________
Double_t KVTrieurTranche::GetXmax(Int_t i)
{
//
// On retourne la vaelur de xmin
   return xtranches(i);
}

//_____________________________________________________
void KVTrieurTranche::SetNomVar(Char_t* x)
{
//
// On met la valeur de Xmin
//
   sprintf(nomVar, "%s", x);
   SetNomsCases();
}

//_____________________________________________________
const Char_t* KVTrieurTranche::GetNomVar(void)
{
//
// On retourne la vaelur de xmin
   return nomVar;
}

//________________________________________________________
void KVTrieurTranche::SetNbCases(Int_t n)
{
//
// On ajuste le nombre de cases
//
   if (n != nb_cases) {
      KVTrieur::SetNbCases(n);
      xtranches.ResizeTo(n + 1);
      SetNomsCases();
   }
}
