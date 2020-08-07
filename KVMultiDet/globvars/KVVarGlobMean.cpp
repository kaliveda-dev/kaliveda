#include "Riostream.h"
#include "KVVarGlobMean.h"
#include "KVClassFactory.h"

ClassImp(KVVarGlobMean)

void KVVarGlobMean::init(void)
{
   SetNameIndex("Mean", 0);
   SetNameIndex("RMS", 1);
   SetNameIndex("SumVar", 2);
   SetNameIndex("SumVarSquared", 3);
   SetNameIndex("SumOfWeights", 4);
   SetNameIndex("Min", 5);
   SetNameIndex("Max", 6);
}

void KVVarGlobMean::FillVar(Double_t v, Double_t w)
{
   svar += w * v;
   svar2 += w * v * v;
   sw += w;
   if (v > max) max = v;
   if (v < min) min = v;
   calc = 0;
}

void KVVarGlobMean::Calculate()
{
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

Double_t KVVarGlobMean::getvalue_int(Int_t i) const
{
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
   KVVarGlob::AddInitMethod(cf, body);
   KVVarGlob::ImplementInitMethod(cf, body, type);

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

   KVVarGlob::FillMethodBody(body, type);
   switch (type) {
      case kTwoBody:
         body += "\n";
         body += "Use the FillVar(v,w) method to increment the quantity of the global variable.\n";
         body += "The value, v, and the weight, w, are to be evaluated from the properties of the\n";
         body += "two KVNucleus pointers passed as argument.\n";
         break;
      case kNBody:
         body += "\n";
         body += "Use the FillVar(v,w) method to increment the quantity of the global variable.\n";
         body += "The value, v, and the weight, w, are to be evaluated from the properties of the\n";
         body += "KVEvent pointer passed as argument.\n";
         break;
      default:
         body += "\n";
         body += "Use the FillVar(v,w) method to increment the quantity of the global variable.\n";
         body += "The value, v, and the weight, w, are to be evaluated from the properties of the \n";
         body += "KVNucleus passed as argument. For example, to evaluate the mean parallel velocity\n";
         body += "weighted by the charge of the nucleus, you may proceed as follows:\n";
         body += "\n";
         body += "FillVar(n->GetVpar(), n->GetZ());\n";
   }
}

