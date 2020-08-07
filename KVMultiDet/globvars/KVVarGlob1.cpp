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
   KVVarGlob::AddInitMethod(cf, body);
   KVVarGlob::ImplementInitMethod(cf, body, type);

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

   KVVarGlob::FillMethodBody(body, type);
   switch (type) {
      case kTwoBody:
         body += "\n";
         body += "Use the FillVar(v) method to increment the quantity of the global variable.\n";
         body += "The value, v, is to be evaluated from the properties of the\n";
         body += "two KVNucleus pointers passed as arguments.\n";
         break;
      case kNBody:
         body += "\n";
         body += "Use the FillVar(v) method to increment the quantity of the global variable.\n";
         body += "The value, v, is to be evaluated from the properties of the\n";
         body += "KVEvent pointer passed as argument.\n";
         break;
      default:
         body += "\n";
         body += "Use the FillVar(v) method to increment the quantity of the global variable.\n";
         body += "The value, v, is to be evaluated from the properties of the \n";
         body += "KVNucleus passed as argument. For example, to evaluate the sum of the charge\n";
         body += "of all fragments, you may proceed as follows:\n";
         body += "\n";
         body += "FillVar(n->GetZ());\n";
   }
}
