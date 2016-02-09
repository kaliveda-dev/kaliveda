//
// D.Cussol
//
// 17/04/2001:
// Creation d'une classe de tri. Cette classe permet d'effectuer un tri et de
// retourner un nombre entier correspondand a une classe donne. Pour un argument
// reel donne le trieur retourne un
// nombre correspondant a l'intervalle de la tranche.
//

#include <math.h>
#include <stdio.h>
#include "KVTrieurTranche.h"

using namespace std;

ClassImp(KVTrieurTranche)
//////////////////////////////////////////////////////////////////////////////////
//          *****       Class for a sorting with attached cells.      ******
//
//   This class returns an index determined via a a research in an array of
// values. The upper limit of a cell in the array is the lower limit of the next
// cell. The following methods are provided by this class:
//      virtual Int_t GetNumCase(Double_t x)   Gives an index corresponding to
//                                             the value of x. If xarray is the
//                                             array of nb_cells values, the
//                                             returned value i is determined
//                                             following this prescription:
//
//                                             xarray[i-1] < x < xarray[i]
//
//                                             if x<x[0] it returns -1
//                                             if x>x[nb_cells] it returns -1
//                                             -1 means "out of range"
//
//      virtual Int_t GetNumCase(void *,...)   return -1,not to use.
//
// BEWARE! : the index value ranges between 1 and nb_cells.
//
//  For each index, the Xmin and Xmax values can be adjusted with the methods
//      virtual void SetXmin(Int_t i,Double_t x)
//      virtual void SetXmax(Int_t i,Double_t x)
//
// BEWARE! : the index value ranges between 1 and nb_cells.
//  Remember that calling SetXmax(i,x) is equivalent to call  SetXmin(i+1,x), or
//  that calling SetXmin(i,x) is equivalent to call  SetXmax(i-1,x).
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
// KVTrieurTranche sortEkin;
// sortEkin.SetNbCases(5);        // 5 indexes
// sortEkin.SetNomVar("E_{kin}"); // name of the sorting variable
// sortEkin.SetXmin(1,0.);        // minimum value of Ekin for cell 1
// sortEkin.SetXmin(2,100.);      // minimum value of Ekin for cell 2
// sortEkin.SetXmin(3,150.);      // minimum value of Ekin for cell 3
// sortEkin.SetXmin(4,300.);      // minimum value of Ekin for cell 4
// sortEkin.SetXmin(5,500.);      // minimum value of Ekin for cell 5
// sortEkin.SetXmax(5,800.);      // maximum value of Ekin for cell 5
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
// if(index > 0)     // Check if SEkin() is not out of range (<0 or >800)
//  {
//  TH1F *h1=(TH1F *)lekin->At(index-1); // retrieve the histogram in the list
//  h1->Fill(Zmax());                    // fill it!
//  }
// ...
//
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
   Char_t nomt[80];
   if (noms_cases) {
      for (Int_t i = 0; i < nb_cases; i++) {
         sprintf(nomt, "%f #leq %s < %f", xtranches(i), nomVar,
                 xtranches(i + 1));
         TNamed* nom = (TNamed*) noms_cases->At(i);
         nom->SetTitle(nomt);
      }
   }
}

//_____________________________________________________
KVTrieurTranche::KVTrieurTranche(void): KVTrieur()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   initKVTrieurTranche();
   sprintf(nom, "KVTrieurTranche_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurTranche
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
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
