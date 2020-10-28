#include <math.h>
#include <stdio.h>
#include "KVTrieurLin.h"

using namespace std;

ClassImp(KVTrieurLin)

Int_t KVTrieurLin::nb = 0;
Int_t KVTrieurLin::nb_crea = 0;
Int_t KVTrieurLin::nb_dest = 0;

//_____________________________________________________
void KVTrieurLin::initKVTrieurLin(void)
{
//
// Initialisation des champs de KVTrieurLin
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;
   xmin = 0.;
   xmax = 0.;
   sprintf(nom_var, "Variable");
}


//_____________________________________________________
void KVTrieurLin::SetNomsCases(void)
{
//
// On affecte les noms aux cases
//
   TString nomt;
   if (noms_cases) {
      Double_t xpas = (xmax - xmin) / nb_cases;
      for (Int_t i = 0; i < nb_cases; i++) {
         if (i == 0) {
            nomt.Form("%s < %f", nom_var, xmin + (i + 1) * xpas);
         }
         else if (i == nb_cases - 1) {
            nomt.Form("%f #leq %s", xmin + i * xpas, nom_var);
         }
         else {
            nomt.Form("%f #leq %s < %f", xmin + i * xpas, nom_var,
                      xmin + (i + 1) * xpas);
         }
         TNamed* nom = (TNamed*) noms_cases->At(i);
         nom->SetTitle(nomt);
      }
   }
}

//_____________________________________________________
KVTrieurLin::KVTrieurLin(void): KVTrieur()
{
//
// Createur par default
//
   TString nom;

   initKVTrieurLin();
   nom.Form("KVTrieurLin_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurLin
   cout << nb << " crees...(defaut) " << endl;
#endif
}

//_____________________________________________________
KVTrieurLin::KVTrieurLin(Char_t* nom): KVTrieur(nom)
{
//
// Constructeur avec un nom
//
   initKVTrieurLin();
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurLin
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_____________________________________________________
KVTrieurLin::KVTrieurLin(Int_t nbcases, Char_t* nom): KVTrieur(nbcases, nom)
{
//
// Constructeur avec un nombre de cases et un nom
//
   initKVTrieurLin();
#ifdef DEBUG_KVTrieurLin
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_____________________________________________________
KVTrieurLin::KVTrieurLin(const KVTrieurLin& a) : KVTrieur()
{
//
// Contructeur par copie
//
   initKVTrieurLin();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieurLin&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieurLin
   cout << nb << " crees...(copie) " << endl;
#endif
}

//_____________________________________________________
KVTrieurLin::~KVTrieurLin(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVTrieurLin
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;
   nb_dest++;
}

//_____________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTrieurLin::Copy(TObject& a) const
{
#else
void KVTrieurLin::Copy(TObject& a)
{
#endif
//
// Methode de copie
//
#ifdef DEBUG_KVTrieurLin
   cout << "Copie de " << this->GetName() << "..." << endl;
#endif
   KVTrieur::Copy(a);
   ((KVTrieurLin&) a).xmin = this->xmin;
   ((KVTrieurLin&) a).xmax = this->xmax;
   sprintf(((KVTrieurLin&) a).nom_var, "%s", this->nom_var);
#ifdef DEBUG_KVTrieurLin
   cout << "Nom de la copie (arguement): " << ((KVTrieurLin&) a).
        nom_var << endl;
   cout << "Nom de la copie (resultat) : " << ((KVTrieurLin&) a).
        GetName() << endl;
#endif
}

//_____________________________________________________
KVTrieurLin& KVTrieurLin::operator =(const KVTrieurLin& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVTrieurLin
   cout << "Copie par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieurLin&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieurLin
   cout << "Nom de la copie par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_____________________________________________________
//
// On retourne le numero de case
//
Int_t KVTrieurLin::GetNumCase(void* ...)
{
   cout << "Utiliser KVTrieurLin::GetNum_Case(Double_t x)" << endl;
   return -1;
}

//_____________________________________________________
Int_t KVTrieurLin::GetNumCase(Double_t x)
{
//
// On retourne le numero de case
//
   Int_t i = 0;
   if (xmax > xmin) {
      i = (Int_t)((x - xmin) / (xmax - xmin) * nb_cases) + 1;
      if (i < 1)
         i = 1;
      if (i > nb_cases)
         i = nb_cases;
   }
   else {
      Warning("GetNumCase(Double_t x)", "Xmin >= Xmax !");
      i = 0;
   }
   return i;
}

//_____________________________________________________
void KVTrieurLin::SetXmin(Double_t x)
{
//
// On met la valeur de Xmin
//
   xmin = x;
   SetNomsCases();
}

//_____________________________________________________
Double_t KVTrieurLin::GetXmin(void)
{
//
// On retourne la vaelur de xmin
   return xmin;
}

//_____________________________________________________
void KVTrieurLin::SetXmax(Double_t x)
{
//
// On met la valeur de Xmin
//
   xmax = x;
   SetNomsCases();
}

//_____________________________________________________
Double_t KVTrieurLin::GetXmax(void)
{
//
// On retourne la vaelur de xmin
   return xmax;
}

//_____________________________________________________
void KVTrieurLin::SetNomVar(Char_t* x)
{
//
// On met la valeur de Xmin
//
   sprintf(nom_var, "%s", x);
   SetNomsCases();
}

//_____________________________________________________
const Char_t* KVTrieurLin::GetNomVar(void)
{
//
// On retourne la vaelur de xmin
   return nom_var;
}

//________________________________________________________
void KVTrieurLin::SetNbCases(Int_t n)
{
//
// On ajuste le nombre de cases
//
   if (n != nb_cases) {
      KVTrieur::SetNbCases(n);
      SetNomsCases();
   }
}
