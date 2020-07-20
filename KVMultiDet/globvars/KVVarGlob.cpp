#include "KVVarGlob.h"
#include "KVClassFactory.h"
#include "TObjString.h"

ClassImp(KVVarGlob)
/////////////////////////////////////////////////////////////////////////////////
/*

*/
////////////////////////////////////////////////////////////////////////////////////

//_________________________________________________________________
void KVVarGlob::init(void)
{
   nameList.Clear();
   // one-body variable by default
   fType = kOneBody;
   fValueType = 'D';
   fMaxNumBranches = -1;
   conditioned_fill = kFALSE;
   fIsInitialized = kFALSE;
   fNormalization = 1.0;
   fOptions.SetName("Options");
   fParameters.SetName("Parameters");
}

void KVVarGlob::MakeClass(const Char_t* classname, const Char_t* classdesc, int type)
{
   // Creates skeleton '.h' and '.cpp' files for a new global variable class which
   // inherits from this class. Give a name for the new class and a short description
   // which will be used to document the class.
   //
   // By default the new class will be of type 1-body. A Fill(KVNucleus*) method
   // will be generated which the user should complete.
   // For a 2-body variable, call MakeClass with type = KVVarGlob::kTwoBody.
   // A skeleton Fill2(KVNucleus*,KVNucleus*) method will be generated.
   // For a N-body variable, call MakeClass with type = KVVarGlob::kNBody.
   // A skeleton FillN(KVEvent*) method will be generated.

   // basic class template
   KVClassFactory cf(classname, classdesc, "KVVarGlob", kTRUE);

   KVString body;

   // add 'init' method
   KVVarGlob::AddInitMethod(classname, cf, body, type);

   // add 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob::AddFillMethod(cf, type);

   // body of 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob::FillMethodBody(body, type);

   // add body of method
   KVVarGlob::AddFillMethodBody(cf, body, type);

   cf.GenerateCode();
}

void KVVarGlob::FillMethodBody(KVString& body, int type)
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
         body += "   // contribution to any sum by 2 to avoid double-counting.\n";
         break;
      case kNBody:
         body = "   // Calculation of contribution to N-body global variable of particles in event e.\n";
         break;
      default:
         body = "   // Calculation of contribution to 1-body global variable of nucleus n\n";
   }
}

void KVVarGlob::AddInitMethod(const Char_t* classname, KVClassFactory& cf, KVString& body, int type)
{
   // PRIVATE method used by MakeClass.
   // add 'init' method
   cf.AddMethod(Form("init_%s", classname), "void", "private");
   body = "   // Private initialisation method called by all constructors.\n";
   body += "   // All member initialisations should be done here.\n";
   body += "   //\n";
   body += "   // You should also (if your variable calculates several different quantities)\n";
   body += "   // set up a correspondance between named values and index number\n";
   body += "   // using method SetNameIndex(const Char_t*,Int_t)\n";
   body += "   // in order for GetValue(const Char_t*) to work correctly.\n";
   body += "   // The index numbers should be the same as in your getvalue_int(Int_t) method.\n";
   body += "\n";
   switch (type) {
      case kTwoBody:
         body += "   fType = KVVarGlob::kTwoBody; // this is a 2-body variable\n";
         break;
      case kNBody:
         body += "   fType = KVVarGlob::kNBody; // this is a N-body variable\n";
         break;
      default:
         body += "   fType = KVVarGlob::kOneBody; // this is a 1-body variable\n";
   }
   cf.AddMethodBody(Form("init_%s", classname), body);
}

void KVVarGlob::AddFillMethod(KVClassFactory& cf, int type)
{
   // PRIVATE method used by MakeClass.
   // add 'Fill', 'Fill2', or 'FillN' method
   switch (type) {
      case kTwoBody:
         cf.AddMethod("Fill2", "void");
         cf.AddMethodArgument("Fill2", "KVNucleus*", "n1");
         cf.AddMethodArgument("Fill2", "KVNucleus*", "n2");
         break;
      case kNBody:
         cf.AddMethod("FillN", "void");
         cf.AddMethodArgument("FillN", "KVEvent*", "e");
         cf.AddHeaderIncludeFile("KVEvent.h");
         break;
      default:
         cf.AddMethod("Fill", "void");
         cf.AddMethodArgument("Fill", "KVNucleus*", "n");
   }
}

void KVVarGlob::AddFillMethodBody(KVClassFactory& cf, KVString& body, int type)
{
   // PRIVATE method used by MakeClass.
   // add body of fill method
   switch (type) {
      case kTwoBody:
         cf.AddMethodBody("Fill2", body);
         break;
      case kNBody:
         cf.AddMethodBody("FillN", body);
         break;
      default:
         cf.AddMethodBody("Fill", body);
   }
}

//_________________________________________________________________
void KVVarGlob::SetNameIndex(const Char_t* name, Int_t index)
{
   // Make the link between a variable name and an index
   if (!(nameList.HasParameter(name))) {
      nameList.SetValue(name, index);
   }
   else {
      Warning("SetNameIndex(const Char_t *name,Int_t index)",
              "No link between \"%s\" and the index %d: the name already exist.",
              name, index);
   }
}

Int_t KVVarGlob::GetNameIndex(const Char_t* name) const
{
   // return the index corresponding to name
   Int_t index = 0;
   if (nameList.HasParameter(name)) {
      index = nameList.GetIntValue(name);
   }
   else {
      Warning("GetNameIndex(const Char_t *name)",
              "The parameter \"%s\" does not exist fot the Class %s.\n 0 returned.",
              name, ClassName());

   }
   return index;
}

void KVVarGlob::Print(Option_t*) const
{
   printf("GLOBVAR: %s     class=%s [", GetName(), ClassName());
   switch (fType) {
      case kOneBody:
         printf("One-body variable]\n");
         break;
      case kTwoBody:
         printf("Two-body variable]\n");
         break;
      case kNBody:
         printf("N-body variable]\n");
         break;
      default:
         printf("Variable type unknown!]\n");
   }
   if (GetNumberOfValues() > 1)
      printf("- Multi-valued variable:\n");
   else
      printf("- Single-valued variable:\n");
   printf("- Kinematics calculated in ");
   if (fFrame != "") printf("%s", fFrame.Data());
   else printf("default");
   printf(" frame\n");
   if (fOptions.GetNpar()) {
      printf("- List of options:\n");
      fOptions.Print();
   }
   if (fParameters.GetNpar()) {
      printf("- List of parameters:\n");
      fParameters.Print();
   }
   if (fSelection.IsSet()) {
      printf("- Particle selection criteria:\n");
      fSelection.Print();
   }
   printf("- Number of branches in TTree: %d\n", GetNumberOfBranches());
   printf("- Available values:\n");
   printf(" IND.   NAME      VALUE\n");
   for (int i = 0; i < GetNumberOfValues(); ++i) {
      KVNamedParameter* np = nameList.GetParameter(i);
      printf("  %d      %s     %f\n", np->GetInt(), np->GetName(), GetValue(np->GetName()));
   }
   printf("\n");
}

