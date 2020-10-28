#include <math.h>
#include <stdio.h>
#include "KVTrieurBloc.h"

using namespace std;

ClassImp(KVTrieurBloc)

Int_t KVTrieurBloc::nb = 0;
Int_t KVTrieurBloc::nb_crea = 0;
Int_t KVTrieurBloc::nb_dest = 0;

//_____________________________________________________
void KVTrieurBloc::initKVTrieurBloc(void)
{
//
// Initialisation des champs de KVTrieurBloc
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
   sprintf(nomVar, "Variable");
}


//_____________________________________________________
void KVTrieurBloc::SetNomsCases(void)
{
//
// On affecte les noms des cases
//
   if (noms_cases) {
      for (Int_t i = 0; i < nb_cases; i++) {
         TNamed* nom = (TNamed*) noms_cases->At(i);
         nom->SetTitle(Form("%f #leq %s #leq %f", xmin(i), nomVar, xmax(i)));
      }
   }
}

//_____________________________________________________
KVTrieurBloc::KVTrieurBloc(void): KVTrieur()
{
//
// Createur par default
//
   TString nom;

   initKVTrieurBloc();
   nom.Form("KVTrieurBloc_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurBloc
   cout << nb << " crees...(defaut) " << endl;
#endif
}

//_____________________________________________________
KVTrieurBloc::KVTrieurBloc(Char_t* nom): KVTrieur(nom)
{
//
// Constructeur avec un nom
//
   initKVTrieurBloc();
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurBloc
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_____________________________________________________
KVTrieurBloc::KVTrieurBloc(Int_t nbcases, Char_t* nom): KVTrieur(nbcases,
         nom)
{
//
// Constructeur avec un nombre de cases et un nom
//
   initKVTrieurBloc();
   xmin.ResizeTo(nbcases);
   xmax.ResizeTo(nbcases);
#ifdef DEBUG_KVTrieurBloc
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_____________________________________________________
KVTrieurBloc::KVTrieurBloc(const KVTrieurBloc& a) : KVTrieur()
{
//
// Contructeur par copie
//
   initKVTrieurBloc();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieurBloc&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieurBloc
   cout << nb << " crees...(copie) " << endl;
#endif
}

//_____________________________________________________
KVTrieurBloc::~KVTrieurBloc(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVTrieurBloc
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;
   nb_dest++;
}

//_____________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTrieurBloc::Copy(TObject& a) const
{
#else
void KVTrieurBloc::Copy(TObject& a)
{
#endif
//
// Methode de copie
//
#ifdef DEBUG_KVTrieurBloc
   cout << "Copie de " << this->GetName() << "..." << endl;
#endif
   KVTrieur::Copy(a);
   ((KVTrieurBloc&) a).xmin.ResizeTo(this->nb_cases);
   ((KVTrieurBloc&) a).xmax.ResizeTo(this->nb_cases);
   ((KVTrieurBloc&) a).xmin = this->xmin;
   ((KVTrieurBloc&) a).xmax = this->xmax;
   sprintf(((KVTrieurBloc&) a).nomVar, "%s", this->nomVar);
#ifdef DEBUG_KVTrieurBloc
   cout << "Nom de la copie (arguement): " << this->nomVar << endl;
   cout << "Nom de la copie (resultat) : " << ((KVTrieurBloc&)
         a.)GetName() << endl;
#endif
}

//_____________________________________________________
KVTrieurBloc& KVTrieurBloc::operator =(const KVTrieurBloc& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVTrieurBloc
   cout << "Copie par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieurBloc&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieurBloc
   cout << "Nom de la copie par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_____________________________________________________
Int_t KVTrieurBloc::GetNumCase(void* ...)
{
//
// On retourne le numero de case
//
   cout << "Utiliser KVTrieurBloc::GetNumCase(Double_t x)" << endl;
   return -1;
}

//_____________________________________________________
Int_t KVTrieurBloc::GetNumCase(Double_t x)
{
//
// On retourne le numero de case
//
   Int_t i = 0;
   while ((i < nb_cases) && (x < xmin(i) || x > xmax(i)))
      i++;
   if (i >= nb_cases)
      i = -1;
   return i + 1;
}

//_____________________________________________________
void KVTrieurBloc::SetXmin(Int_t i, Double_t x)
{
//
// On met la valeur de Xmin
//
   xmin(i - 1) = x;
   SetNomsCases();
}

//_____________________________________________________
Double_t KVTrieurBloc::GetXmin(Int_t i)
{
//
// On retourne la vaelur de xmin
   return xmin(i - 1);
}

//_____________________________________________________
void KVTrieurBloc::SetXmax(Int_t i, Double_t x)
{
//
// On met la valeur de Xmin
//
   xmax(i - 1) = x;
   SetNomsCases();
}

//_____________________________________________________
Double_t KVTrieurBloc::GetXmax(Int_t i)
{
//
// On retourne la vaelur de xmin
   return xmax(i - 1);
}

//_____________________________________________________
void KVTrieurBloc::SetNomVar(Char_t* x)
{
//
// On met la valeur de Xmin
//
   sprintf(nomVar, "%s", x);
   SetNomsCases();
}

//_____________________________________________________
const Char_t* KVTrieurBloc::GetNomVar(void)
{
//
// On retourne la vaelur de xmin
   return nomVar;
}

//________________________________________________________
void KVTrieurBloc::SetNbCases(Int_t n)
{
//
// On ajuste le nombre de cases
//
   if (n != nb_cases) {
      KVTrieur::SetNbCases(n);
      xmin.ResizeTo(n);
      xmax.ResizeTo(n);
      SetNomsCases();
   }
}
