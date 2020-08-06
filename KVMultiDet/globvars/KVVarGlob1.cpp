//
//Author: Daniel Cussol
//
// 17/02/2004
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Cette Classe traite le cas ou 1 valeur est attendue (Etrans, Multiplicite,
//                                                      Etot, etc..)
//

#include "TMath.h"
#include "TString.h"
#include "Riostream.h"
#include "KVVarGlob1.h"
#include "KVClassFactory.h"

ClassImp(KVVarGlob1)
//////////////////////////////////////////////////////////////////////////////////
//          *****  Base class for Global variables with 1 value  ******
//
//   This class is a base abstact class for the management of global variable with only
// one value. A global variable class deriving from KVVarGlob1 has to provide the following methods:
//      virtual void Init(void)                 initiates the internal fields
//      virtual void Reset(void)                resets internal fields before the treatment
//      virtual void Fill(KVNucleus *c)         tells how to compute the global variable using a KVNucleus
//
// A call to the protected method FillVar(Double_t v) adds v to the current value.
// A call to the GetValue() method returns the value of the variable.
// Have a look to the KVEkin class to have an example of how to implement a global variable deriving from
// KVVarGlob1. Look at KVVarGlob class to have an example of use.
//
Int_t KVVarGlob1::nb = 0;
Int_t KVVarGlob1::nb_crea = 0;
Int_t KVVarGlob1::nb_dest = 0;

//_________________________________________________________________
void KVVarGlob1::init(void)
{
//
// Initialisation des champs de KVVarGlob1
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;

   var = 0;
}

//_________________________________________________________________
KVVarGlob1::KVVarGlob1(void): KVVarGlob()
{
//
// Createur par default
//
   TString nom;
   init();
   nom.Form("KVVarGlob1_%d", nb_crea);
   SetName(nom.Data());
   SetTitle(nom.Data());
#ifdef DEBUG_KVVarGlob1
   cout << nb << " crees...(defaut) " << endl;
#endif
}

//_________________________________________________________________
KVVarGlob1::KVVarGlob1(const Char_t* nom): KVVarGlob(nom)
{
//
// Constructeur avec un nom
//
   init();
#ifdef DEBUG_KVVarGlob1
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVVarGlob1::KVVarGlob1(const KVVarGlob1& a): KVVarGlob(a)
{
//
// Contructeur par Copy
//
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVVarGlob1&) a).Copy(*this);
#endif
#ifdef DEBUG_KVVarGlob1
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVVarGlob1::~KVVarGlob1(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVVarGlob1
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}

//_________________________________________________________________
KVVarGlob1& KVVarGlob1::operator =(const KVVarGlob1& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVVarGlob1
   cout << "Copy par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVVarGlob1&) a).Copy(*this);
#endif
#ifdef DEBUG_KVVarGlob1
   cout << "Nom de la Copy par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_________________________________________________________________
void KVVarGlob1::Init(void)
{
// methode d'initialisation des
// variables Internes
   var = 0;
}

//_________________________________________________________________
void KVVarGlob1::Reset(void)
{
// Remise a zero avant le
// traitement d'un evenement
   var = 0;
}

//_________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVVarGlob1::Copy(TObject& a) const
#else
void KVVarGlob1::Copy(TObject& a)
#endif
{
   // Methode de Copy
   //In order to be coherent with ROOT Copy functions, "a" becomes a copy of "this",
   //not the other way around as before.
   //"a" is first Reset and then has data copied into it. This means calling KVVarGlob::Copy (which
   //in turn calls KVBase::Copy, copying the name and title/label of the object), then setting the value
   //of "var" in "a".

   ((KVVarGlob1&) a).Reset();
   KVVarGlob::Copy(a);
   ((KVVarGlob1&) a).SetValue(GetValue());
}

//_________________________________________________________________

void KVVarGlob1::MakeClass(const Char_t* classname, const Char_t* classdesc, int type)
{
   //Creates skeleton '.h' and '.cpp' files for a new global variable class which
   //inherits from this class. Give a name for the new class and a short description
   //which will be used to document the class.
   //
   // By default the new class will be of type 1-body. A Fill(KVNucleus*) method
   // will be generated which the user should complete.
   // For a 2-body variable, call MakeClass with type = KVVarGlob::kTwoBody.
   // A skeleton Fill2(KVNucleus*,KVNucleus*) method will be generated.
   // For a N-body variable, call MakeClass with type = KVVarGlob::kNBody.
   // A skeleton FillN(KVEvent*) method will be generated.

   // basic class template
   KVClassFactory cf(classname, classdesc, "KVVarGlob1", kTRUE);

   KVString body;

   // add 'init' method
   KVVarGlob::AddInitMethod(classname, cf, body, type);

   // add 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob::AddFillMethod(cf, type);

   // body of 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob1::FillMethodBody(body, type);

   // add body of method
   KVVarGlob::AddFillMethodBody(cf, body, type);

   cf.GenerateCode();
}

void KVVarGlob1::FillMethodBody(KVString& body, int type)
{
   // PRIVATE method used by MakeClass.
   // body of 'Fill', 'Fill2', or 'FillN' method
   switch (type) {
      case kTwoBody:
         body = "   // Calculation of contribution to 2-body global variable of pair (n1,n2) of nuclei.\n";
         body += "   // NOTE: this method will be called for EVERY pair of nuclei in the event\n";
         body += "   // (i.e. n1-n2 and n2-n1), including pairs of identical nuclei (n1 = n2).\n";
         body += "   // If you want to calculate a global variable using only each non-identical pair once,\n";
         body += "   // then make sure in your implementation that you check n1!=n2 and divide the\n";
         body += "   // contribution to any sum by 2 (or use a weight=0.5) to avoid double-counting.\n";
         body += "   //\n";
         body += "   // Use the FillVar(v) method to increment the quantity of the global variable.\n";
         body += "   // The value, v, is to be evaluated from the properties of the\n";
         body += "   // two KVNucleus pointers passed as argument.\n";
         break;
      case kNBody:
         body = "   // Calculation of contribution to N-body global variable of particles in event e.\n";
         body += "   //\n";
         body += "   // Use the FillVar(v) method to increment the quantity of the global variable.\n";
         body += "   // The value, v, is to be evaluated from the properties of the\n";
         body += "   // KVEvent pointer passed as argument.\n";
         break;
      default:
         body = "   // Calculation of contribution to 1-body global variable of nucleus n1\n";
         body += "   //\n";
         body += "   // Use the FillVar(v) method to increment the quantity of the global variable.\n";
         body += "   // The value, v, is to be evaluated from the properties of the \n";
         body += "   // KVNucleus passed as argument. For example, to evaluate the sum of the charge\n";
         body += "   // of all fragments, you may proceed as follows:\n";
         body += "   //\n";
         body += "   // FillVar(n->GetZ());\n";
   }
}

