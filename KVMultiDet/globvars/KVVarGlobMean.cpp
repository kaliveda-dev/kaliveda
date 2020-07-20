//
//Author: Daniel Cussol
//
// 18/02/2004:
// Creation d'une classe Variable Globale.
// Elle est utilisee dans les analyses ROOT.
// Cette Classe traite le cas ou 1 moyenne et un ecart type sont attendus
//

#include "Riostream.h"
#include "KVVarGlobMean.h"
#include "KVClassFactory.h"

ClassImp(KVVarGlobMean)
//////////////////////////////////////////////////////////////////////////////////
//          *****  Base class for Global variables with   ******
//                 a mean value and a standard deviation
//
//   This class is a base abstact class for the management of global variable with a mean value and a standard
//deviation. A global variable class deriving from KVVarGlobmean has to provide the following methods:
//      virtual void Init(void)                 initiates the internal fields
//      virtual void Reset(void)                resets internal fields before the treatment
//      virtual void Fill(KVNucleus *c)         tells how to compute the global variable using a KVNucleus
//
// A call to the GetValue() method returns the mean value of the variable. One can also obtain the array which
// contains the values of the internal fields by a call to GetValuePtr(). Individual values can also be
// obtained by a call to GetValue(Int_t index) or by a call to  GetValue(Chat_t *name). Parenthesis can also be
// used. The correspondance between indexes ans names is as follows:
//
//      Name            Index                   Meaning
//----------------------------------------------------------------
//      Mean            0       (default)       Mean value of var = SumVar/SumOfWeights
//      RMS             1                       standard deviation of var = sqrt(SumVarSquared/SumOfWeights-Mean*Mean)
//      SumVar          2                       Sum of weighted var values
//      SumVarSquared   3                       Sum of weighted squared var values
//      SumOfWeights    4                       Sum of weights
//      Min             5                       Minimum value
//      Max             6                       Maximum value
//
//
// Here are examples on how obtaining values.
//
// KVZmean *pzmean=new KVZmean();
// KVZmean zmean;
// ...
// Double_t mean=pzmean->GetValue();        |
//          mean=pzmean->GetValue("Mean");  |---> Mean value
//          mean=zmean();                   |
//          mean=zmean("Mean");             |
// Double_t ect=zmean("RMS");---------------> Standard Deviation
// Double_t sw=pzmean->GetValue(4) ---------> Sum of weights
// Double_t *pvalues=zmean.GetValuePtr();---> Array of values
//
// The internal fields can be set by a call to the protected methods FillVar(Double_t v, Double_t w) or
// FillVar(Double_t v). In the latter case, the weight is set to 1.
//
//      Method                                  Action
//----------------------------------------------------------------
//      FillVar(Double_t v, Double_t w)         SumVar+=w*v;
//                                              SumVarSquared+=w*v*v;
//                                              SumOfWeights+=w;
//
//      FillVar(Double_t v)                     SumVar+=v;
//                                              SumVarSquared+=v*v;
//                                              SumOfWeights+=1;
//
// Mean value and standard deviation are computed by the protected CalcVar() method.
// Have a look to the KVZmean class for an example of a global variable deriving from KVVarGlobMean.
// Look at KVVarGlob class to have an example of use.
//

//_________________________________________________________________
void KVVarGlobMean::init(void)
{
// Initialisation des champs de KVVarGlobMean
// Cette methode privee n'est appelee par les createurs
//
   SetNameIndex("Mean", 0);
   SetNameIndex("RMS", 1);
   SetNameIndex("SumVar", 2);
   SetNameIndex("SumVarSquared", 3);
   SetNameIndex("SumOfWeights", 4);
   SetNameIndex("Min", 5);
   SetNameIndex("Max", 6);
}

//_________________________________________________________________
void KVVarGlobMean::FillVar(Double_t v, Double_t w)
{
// Routine privee de remplissage
//
   svar += w * v;
   svar2 += w * v * v;
   sw += w;
   if (v > max) max = v;
   if (v < min) min = v;
   calc = 0;
}

//_________________________________________________________________
void KVVarGlobMean::Calculate(void)
{
// Routine de calcul des valeurs moyennes
//
   if (!calc) {
      if (sw != 0) {
         var = svar / sw;
         if (svar2 / sw >= var * var) {
            ect = TMath::Sqrt(svar2 / sw - var * var);
         }
         else {
            ect = -1.;
         }
      }
      else {
         var = 0.;
         ect = -1.;
      }
      calc = 1;
   }
}

//_________________________________________________________________
void KVVarGlobMean::FillVar(Double_t v)
{
//
// Routine privee de remplissage
//
   FillVar(v, 1.);
}

void KVVarGlobMean::Copy(TObject& a) const
{
   // Copy this to a

   KVVarGlob::Copy(a);
   KVVarGlobMean& _a = dynamic_cast<KVVarGlobMean&>(a);
   _a.ect = ect;
   _a.svar = svar;
   _a.svar2 = svar2;
   _a.sw = sw;
   _a.min = min;
   _a.max = max;
   _a.calc = calc;
}

//_________________________________________________________________
void KVVarGlobMean::Reset()
{
   var = 0;
   ect = 0;
   svar = 0;
   svar2 = 0;
   sw = 0;
   min = 1e6;
   max = -1e6;
   calc = 0;
}

void KVVarGlobMean::Init()
{
   Reset();
}

//_________________________________________________________________
Double_t KVVarGlobMean::getvalue_void() const
{
   // Returns mean value of variable
   return getvalue_int(0);
}

//_________________________________________________________________
Double_t KVVarGlobMean::getvalue_int(Int_t i) const
{
// on retourne la ieme valeur du tableau
//
// Index  Meaning
//--------------------------
// 0      Mean value of var = SumVar/SumOfWeights
// 1      standard deviation of var = sqrt(SumVarSquared/SumOfWeights-Mean*Mean)
// 2      Sum of weighted var values
// 3      Sum of weighted squared var values
// 4      Sum of weights
// 5      Minimum value
// 6      Maximum value
//
   Double_t rval = 0;
   switch (i) {
      case 0:
         rval = var;
         break;

      case 1:
         rval = ect;
         break;

      case 2:
         rval = svar;
         break;

      case 3:
         rval = svar2;
         break;

      case 4:
         rval = sw;
         break;

      case 5:
         rval = min;
         break;

      case 6:
         rval = max;
         break;

      default:
         rval = 0.;
         break;
   }
   return rval;
}

//_________________________________________________________________

void KVVarGlobMean::MakeClass(const Char_t* classname, const Char_t* classdesc, int type)
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
   KVClassFactory cf(classname, classdesc, "KVVarGlobMean", kTRUE);

   KVString body;

   // add 'init' method
   KVVarGlob::AddInitMethod(classname, cf, body, type);

   // add 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob::AddFillMethod(cf, type);

   // body of 'Fill', 'Fill2', or 'FillN' method
   KVVarGlobMean::FillMethodBody(body, type);

   // add body of method
   KVVarGlob::AddFillMethodBody(cf, body, type);

   cf.GenerateCode();
}

void KVVarGlobMean::FillMethodBody(KVString& body, int type)
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
         body += "   // Use the FillVar(v,w) method to increment the quantity of the global variable.\n";
         body += "   // The value, v, and the weight, w, are to be evaluated from the properties of the\n";
         body += "   // two KVNucleus pointers passed as argument.\n";
         break;
      case kNBody:
         body = "   // Calculation of contribution to N-body global variable of particles in event e.\n";
         body += "   //\n";
         body += "   // Use the FillVar(v,w) method to increment the quantity of the global variable.\n";
         body += "   // The value, v, and the weight, w, are to be evaluated from the properties of the\n";
         body += "   // KVEvent pointer passed as argument.\n";
         break;
      default:
         body = "   // Calculation of contribution to 1-body global variable of nucleus n1\n";
         body += "   //\n";
         body += "   // Use the FillVar(v,w) method to increment the quantity of the global variable.\n";
         body += "   // The value, v, and the weight, w, are to be evaluated from the properties of the \n";
         body += "   // KVNucleus passed as argument. For example, to evaluate the mean parallel velocity\n";
         body += "   // weighted by the charge of the nucleus, you may proceed as follows:\n";
         body += "   //\n";
         body += "   // FillVar(n->GetVpar(), n->GetZ());\n";
   }
}

