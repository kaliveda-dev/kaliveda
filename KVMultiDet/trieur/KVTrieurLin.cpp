//
// D.Cussol
//
// 17/04/2001:
// Creation d'une classe de tri. Cette classe permet d'effectuer un tri et de
// retourner un nombre entier correspondand a une classe donne. Pour un argument
// reel donne entre une valeur min et une valeur max, le trieur retourne un
// nombre compris entre 1 et nb_cases.
//

#include <math.h>
#include <stdio.h>
#include "KVTrieurLin.h"

using namespace std;

ClassImp(KVTrieurLin)
//////////////////////////////////////////////////////////////////////////////////
//          *****       Class for a linear sorting.      ******
//
//   This class returns an index determined via a linear interpolation between
// two values Xmin and Xmax. The following methods are provided by this class:
//      virtual Int_t GetNumCase(Double_t x)   Gives an index corresponding to
//                                             the value of x. nb_cells being
//                                             the total number of indexes, the
//                                             returned value is
//
//                                                       (x-xmin)*nb_cells
//                                             1+(Int_t)-------------------
//                                                          xmax-xmin
//
//                                             if x<xmin it returns 1
//                                             if x>xmax it returns nb_cells
//
//      virtual Int_t GetNumCase(void *,...)   return -1,not to use.
//
// BEWARE! : the index value ranges between 1 and nb_cells.
//
//  The Xmin and Xmax values can be adjusted with the methods
//      virtual void SetXmin(Double_t x)
//      virtual void SetXmax(Double_t x)
//
//  The name of the sorting variable can be set withe the method
//      virtual void SetNomVar(Char_t *s)
//
// The number of indexes can be set with the method
//      virtual void SetNbCases(Int_t n)
//
//  Setting the number of indexes, the name of the sorting variable or the Xmin
// or Xmax values automatically generates the names for each index.
//
//  In a treatment program, this can be used to set histogram titles and to
// manage efficiently arrays of histograms. Here is an example where the sorting
// variable is Ekin (calculated using KVEkin class).
//== Example ==================================================================================================
// // Declarations and initialisations
// ...
// KVEkin Sekin;
// Sekin.Init();
// KVZmax Zmax;
// Zmax.Init();
// ...
// KVTrieurLin sortEkin;
// sortEkin.SetNbCases(10);       // 10 indexes
// sortEkin.SetNomVar("E_{kin}"); // name of the sorting variable
// sortEkin.SetXmin(0.);          // minimum value for Ekin
// sortEkin.SetXmax(800.);        // maximum value for Ekin
// ...
// // Declaration of histograms
// TList *lekin=new TList();      // list to store histograms
// for(Int_t i=0;i<sortEkin.GetNbCases();i++)
//  {
//  TString sname("histo"); // TString for the histogram name
//  sname+=i;
//  TString stitle("Z_{max} for "); // TString for the histogram title
//  stitle+=sortEkin.GetNomCase(i+1);
//  TH1F *h1=new TH1F(sname.Data(),stitle.Data(),100,0.5,100.5);
//  lekin->Add(h1); // Add the histogram to the list
//  }
// ...
// // Treatment loop for each event called for each event
// ...
// Sekin.Reset();
// Zmax.Reset();
// KVINDRAReconNuc *part = 0;
// while( (part = GetEvent()->GetNextParticle("ok")) ){//loop over particles with correct codes
//  Sekin.Fill(part);
//  Zmax.Fill(part);
// }
// // Filling the appropriate histogram
// Int_t index=sortEkin(Sekin());       // Determine the index value according to Sekin()
// TH1F *h1=(TH1F *)lekin->At(index-1); // retrieve the histogram in the list
// h1->Fill(Zmax());                    // fill it!
// ...
//
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
   Char_t nomt[80];
   if (noms_cases) {
      Double_t xpas = (xmax - xmin) / nb_cases;
      for (Int_t i = 0; i < nb_cases; i++) {
         if (i == 0) {
            sprintf(nomt, "%s < %f", nom_var, xmin + (i + 1) * xpas);
         } else if (i == nb_cases - 1) {
            sprintf(nomt, "%f #leq %s", xmin + i * xpas, nom_var);
         } else {
            sprintf(nomt, "%f #leq %s < %f", xmin + i * xpas, nom_var,
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
   Char_t* nom = new Char_t[80];

   initKVTrieurLin();
   sprintf(nom, "KVTrieurLin_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurLin
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
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
   } else {
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
