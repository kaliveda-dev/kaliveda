#include "KVVarGlob1.h"
#include "TMath.h"
#include "TString.h"
#include "Riostream.h"
#include "KVClassFactory.h"

ClassImp(KVVarGlob1)

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

