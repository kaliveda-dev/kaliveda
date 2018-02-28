//
// D.Cussol
//
// 17/04/2001:
// Creation d'une classe de tri. Cette classe permet d'effectuer un tri et de
// retourner un nombre entier correspondand a une classe donne. Ceci est une
// classe virtuelle. La classe contient une methode virtuelle qui devra etre
// fournie par l'utilisateur lors de son implementation.
//

#include <math.h>
#include <stdio.h>
#include "KVTrieur.h"

using namespace std;

ClassImp(KVTrieur)
//////////////////////////////////////////////////////////////////////////////////
//          *****       Base class for Sorting purposes.      ******
//
//   This class is a base abstact class for the management of sortings. A
//  sorting class deriving from KVTrieur has to provide the following methods:
//      virtual Int_t GetNumCase(Double_t x)   Gives an index corresponding to
//                                             the value of x.
//      virtual Int_t GetNumCase(void *,...)   Gives an index corresponding to
//                                             the list of arguments.
//
//  Child classes KVTrieurLin, KVTrieurTranche and KVTrieurBloc provide
// these methods. A name can be associated to each index value with the method
//      virtual void SetNomCase(Int_t i,Char_t *s)
// and can be retrieved withe the method
//      virtual const Char_t *GetNomCase(Int_t i)
//
// The number of indexes can be set with the method
//      virtual void SetNbCases(Int_t n)
//
//
Int_t KVTrieur::nb = 0;
Int_t KVTrieur::nb_crea = 0;
Int_t KVTrieur::nb_dest = 0;

//________________________________________________________
void KVTrieur::initKVTrieur(void)
{
//
// Initialisation des champs de KVTrieur
// Cette methode privee n'est appelee par les createurs
//

   nb++;
   nb_crea++;
   nb_cases = 0;
   noms_cases = 0;
}

//________________________________________________________
KVTrieur::KVTrieur(void): KVBase()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   initKVTrieur();
   sprintf(nom, "KVTrieur_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieur
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//________________________________________________________
KVTrieur::KVTrieur(Char_t* nom)
{
//
// Constructeur avec un nom
//
   initKVTrieur();
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieur
   cout << nb << " crees...(nom) " << endl;
#endif
}

//________________________________________________________
KVTrieur::KVTrieur(Int_t nbcases, Char_t* nom)
{
//
// Constructeur avec un nombre de cases et un nom
//
   initKVTrieur();
   SetName(nom);
   SetTitle(nom);
   if (nbcases > 0) {
      Char_t nomc[80];
      Char_t nomt[80];
      nb_cases = nbcases;
      noms_cases = new TClonesArray("TNamed", nbcases);
      for (Int_t i = 0; i < nbcases; i++) {
         sprintf(nomc, "%s_C%d", this->GetName(), i + 1);
         sprintf(nomt, "Case numero %d", i + 1);
         new ((*noms_cases)[i]) TNamed(nomc, nomt);
      }
   }
#ifdef DEBUG_KVTrieur
   cout << nb << " crees...(nom) " << endl;
#endif
}

//________________________________________________________
KVTrieur::KVTrieur(const KVTrieur& a) : KVBase()
{
//
// Contructeur par Copie
//
   initKVTrieur();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieur&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieur
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//________________________________________________________
KVTrieur::~KVTrieur(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVTrieur
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   if (noms_cases) {            // Destuction du TClonesArray contenant les noms de cases
      noms_cases->Clear();
      delete noms_cases;
   }

   nb_dest++;
}


//________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVTrieur::Copy(TObject& a) const
{
#else
void KVTrieur::Copy(TObject& a)
{
#endif
//
// Methode de Copy
//
   Char_t nom[80];

#ifdef DEBUG_KVTrieur
   cout << "Copy de " << a.GetName() << "..." << endl;
#endif
#ifdef DEBUG_KVTrieur
   sprintf(nom, "Copy de %s", a.GetName());
#else
   sprintf(nom, "Copy_%s", a.GetName());
#endif
   ((KVTrieur&) a).SetName(nom);
   ((KVTrieur&) a).SetTitle(nom);

   ((KVTrieur&) a).nb_cases = this->nb_cases;
   if (this->noms_cases) {
      ((KVTrieur&) a).noms_cases =
         new TClonesArray("TNamed", this->nb_cases);
      TClonesArray* tca = ((KVTrieur&) a).noms_cases;
      Char_t nomc[80];
      for (Int_t i = 0; i < this->nb_cases; i++) {
         TNamed* c = (TNamed*) this->noms_cases->At(i);
         sprintf(nomc, "%s_C%d", this->GetName(), i + 1);
         new ((*tca)[i]) TNamed(nomc, c->GetTitle());
      }
   } else {
      ((KVTrieur&) a).noms_cases = 0;
   }
#ifdef DEBUG_KVTrieur
   cout << "Nom de la Copy (arguement): " << nom << endl;
   cout << "Nom de la Copy (resultat) : " << GetName() << endl;
#endif
}

//________________________________________________________
KVTrieur& KVTrieur::operator =(const KVTrieur& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVTrieur
   cout << "Copie par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVTrieur&) a).Copy(*this);
#endif
#ifdef DEBUG_KVTrieur
   cout << "Nom de la Copie par egalite: " << GetName() << endl;
#endif
   return *this;
}

//________________________________________________________
const Char_t* KVTrieur::GetNomCase(Int_t i)
{
//
// On retourne le nom d'une case
//
   if (noms_cases) {
      if (i > 0 && i <= nb_cases) {
         return ((TNamed*) noms_cases->At(i - 1))->GetTitle();
      } else {
         cout << "Le numero de case est incorrect..." << endl;
         return 0;
      }
   } else {
      cout << "Les noms de cases n'ont pas etes definis..." << endl;
      return 0;
   }
}

//________________________________________________________
void KVTrieur::SetNomCase(Int_t i, Char_t* s)
{
//
// On impose le nom d'une case
//
   if (noms_cases) {
      if (i > 0 && i <= nb_cases) {
         ((TNamed*) noms_cases->At(i - 1))->SetTitle(s);
      } else {
         cout << "Le numero de case est incorrect..." << endl;
      }
   } else {
      cout << "Les noms de cases n'ont pas etes definis..." << endl;
   }
}

//________________________________________________________
Int_t KVTrieur::GetNbCases(void)
{
//
// On retourne le nombre de cases
//
   return nb_cases;
}

//________________________________________________________
void KVTrieur::SetNbCases(Int_t n)
{
//
// On ajuste le nombre de cases
//
   if (n != nb_cases) {
      Char_t nomc[80];
      Char_t nomt[80];
      nb_cases = n;
      if (noms_cases) {
         noms_cases->Clear();
         delete noms_cases;
      }
      noms_cases = new TClonesArray("TNamed", n);
      for (Int_t i = 0; i < nb_cases; i++) {
         sprintf(nomc, "%s_C%d", this->GetName(), i + 1);
         sprintf(nomt, "Case numero %d", i + 1);
         new ((*noms_cases)[i]) TNamed(nomc, nomt);
      }
   }
}

//________________________________________________________
Int_t KVTrieur::GetNumCase(void* ...)
{
//
// On retourne le numero de case
//
   cout << "Classe virtuelle, pas d'argument valables..." << endl;
   return -1;
}

//________________________________________________________
Int_t KVTrieur::GetNumCase(Double_t)
{
//
// On retourne le numero de case
//
   cout << "Classe virtuelle, pas d'argument valables..." << endl;
   return -1;
}
