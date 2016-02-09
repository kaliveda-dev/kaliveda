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
#include "KVTrieurBloc.h"

using namespace std;

ClassImp(KVTrieurBloc)
//////////////////////////////////////////////////////////////////////////////////
//          *****     Class for a sorting with detached cells.      ******
//
//   This class returns an index determined via a a research in an array of
// values. The upper limit and the lower limit of a cell are not belonging to
// any of the other cells. The following methods are provided by this class:
//      virtual Int_t GetNumCase(Double_t x)   Gives an index corresponding to
//                                             the value of x. If xmin and xmax
//                                             are the arrays of nb_cells values,
//                                             the returned value i is determined
//                                             following this prescription:
//
//                                             xmin[i-1] < x < xmax[i-1]
//
//                                             if the x value do not correspond
//                                             to any of the cells, -1 is
//                                             returned.
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
//
//  The name of the sorting variable can be set withe the method
//      virtual void SetNomVar(Char_t *s)
//
// The number of indexes can be set with the method. This has to be done first
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
// KVTrieurBloc sortEkin;
// sortEkin.SetNbCases(5);        // 5 indexes
// sortEkin.SetNomVar("E_{kin}"); // name of the sorting variable
// sortEkin.SetXmin(1,0.);        // minimum value of Ekin for cell 1
// sortEkin.SetXmax(1,50.);       // maximum value of Ekin for cell 1
// sortEkin.SetXmin(2,100.);      // minimum value of Ekin for cell 2
// sortEkin.SetXmax(2,150.);      // maximum value of Ekin for cell 2
// sortEkin.SetXmin(3,150.);      // minimum value of Ekin for cell 3
// sortEkin.SetXmax(3,225.);      // maximum value of Ekin for cell 3
// sortEkin.SetXmin(4,300.);      // minimum value of Ekin for cell 4
// sortEkin.SetXmax(4,390.);      // maximum value of Ekin for cell 4
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
   Char_t nomt[80];
   if (noms_cases) {
      for (Int_t i = 0; i < nb_cases; i++) {
         sprintf(nomt, "%f #leq %s #leq %f", xmin(i), nomVar, xmax(i));
         TNamed* nom = (TNamed*) noms_cases->At(i);
         nom->SetTitle(nomt);
      }
   }
}

//_____________________________________________________
KVTrieurBloc::KVTrieurBloc(void): KVTrieur()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   initKVTrieurBloc();
   sprintf(nom, "KVTrieurBloc_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVTrieurBloc
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
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
