/*
$Id: KVParticleCondition.cpp,v 1.4 2007/03/26 10:14:56 franklan Exp $
$Revision: 1.4 $
$Date: 2007/03/26 10:14:56 $
*/

//Created by KVClassFactory on Thu Nov 16 14:20:38 2006
//Author: franklan

#include "KVParticleCondition.h"
#include "TROOT.h"
#include "KVNucleus.h"
#include "Riostream.h"
#include "TSystem.h"
#include "KVClassFactory.h"
#include "TPluginManager.h"
#include "TUUID.h"

using namespace std;

ClassImp(KVParticleCondition)

////////////////////////////////////////////////////////////////////////////////
/*
<h2>KVParticleCondition</h2>
<h4>Implements parser of particle selection criteria</h4>

These must be valid C++ expressions using _NUC_ instead and in place of
a pointer to the particle to be tested. Note that the methods used in the selection
do not have to be limited to the KVNucleus class. The 'real' class of the object
passed to Test() will be used to cast the base pointer (KVNucleus*) up to the
required pointer type at execution. In this case, you must call the method
SetParticleClassName() with the name of the class to use in the cast.

Conditions can be combined using Boolean '&&' and '||' operators.

Note that the first time Test() is called we generate and compile a class derived
from KVParticleCondition which implements the required test. For all subsequent
calls it is this compiled code which is used, greatly increasing speed of
execution.

### EXAMPLES

~~~~~~~~~~~~~~~~
// select nuclei with Z>2
KVParticleCondition pc("_NUC_->GetZ()>2");

KVNucleus nuc("56Fe");
pc.Test(&nuc);

Info in <KVParticleCondition::Optimize>: Optimization of KVParticleCondition : _NUC_->GetZ()>2;
<KVClassFactory::WriteClassHeader> : File KVParticleCondition_9594f66c.h generated.
<KVClassFactory::WriteClassImp> : File KVParticleCondition_9594f66c.cpp generated.
Info in <TUnixSystem::ACLiC>: creating shared library /./KVParticleCondition_9594f66c_cpp.so
Info in <KVParticleCondition::Optimize>: fOptimal = 0x55add512d3a0
Info in <KVParticleCondition::Optimize>: Success
(bool) true

pc.Test(&nuc);
(bool) true

// select simulated nuclei with spin>40hbar
KVParticleCondition pc2 = "_NUC_->GetSpin().Mag()<40";

KVSimNucleus p("56Fe");
p.SetSpin(50,0,0);

pc2.Test(&p);

Info in <KVParticleCondition::Optimize>: Optimization of KVParticleCondition : _NUC_->GetSpin().Mag()>40;
<KVClassFactory::WriteClassHeader> : File KVParticleCondition_b04461fe.h generated.
<KVClassFactory::WriteClassImp> : File KVParticleCondition_b04461fe.cpp generated.
Info in <TUnixSystem::ACLiC>: creating shared library /./KVParticleCondition_b04461fe_cpp.so
In file included from input_line_11:9:
././KVParticleCondition_b04461fe.cpp:43:16: error: no member named 'GetSpin' in 'KVNucleus'
   return nuc->GetSpin().Mag()>40;
          ~~~  ^
Error in <ACLiC>: Dictionary generation failed!
Error in <KVParticleCondition::Optimize>:  *** Optimization failed for KVParticleCondition : _NUC_->GetSpin().Mag()>40;
Error in <KVParticleCondition::Optimize>:  *** Use method AddExtraInclude(const Char_t*) to give the names of all necessary header files for compilation of your condition.
Fatal in <KVParticleCondition::Optimize>:  *** THIS CONDITION WILL BE EVALUATED AS kFALSE FOR ALL PARTICLES!!!
aborting

KVParticleCondition pc2 = "_NUC_->GetSpin().Mag()<40";
pc2.SetParticleClassName("KVSimNucleus"); // GetSpin() only defined for KVSimNucleus class
pc2.Test(&p);

Info in <KVParticleCondition::Optimize>: Optimization of KVParticleCondition : _NUC_->GetSpin().Mag()>40;
<KVClassFactory::WriteClassHeader> : File KVParticleCondition_16f09224.h generated.
<KVClassFactory::WriteClassImp> : File KVParticleCondition_16f09224.cpp generated.
Info in <TUnixSystem::ACLiC>: creating shared library /./KVParticleCondition_16f09224_cpp.so
Info in <KVParticleCondition::Optimize>: fOptimal = 0x55cc657e1d90
Info in <KVParticleCondition::Optimize>: Success
(bool) true
~~~~~~~~~~~~~~~~

*/
////////////////////////////////////////////////////////////////////////////////

KVHashList KVParticleCondition::fgOptimized;

KVParticleCondition::KVParticleCondition()
   : KVBase("KVParticleCondition", "Particle selection criteria")
{
   //default ctor
   fOptimal = 0;
   cf = 0;
   fOptOK = kFALSE;
   fNUsing = 0;
}

//_____________________________________________________________________________//

KVParticleCondition::~KVParticleCondition()
{
   //default dtor
   if (fOptimal) {
      // do not delete optimized condition unless we are the last to use it
      --(fOptimal->fNUsing);
      if (!(fOptimal->fNUsing)) {
         fgOptimized.Remove(fOptimal);
         delete fOptimal;
         fOptimal = 0;
      }
   }
   SafeDelete(cf);
}

//_____________________________________________________________________________//

KVParticleCondition::KVParticleCondition(const Char_t* cond)
   : KVBase(cond, "KVParticleCondition")
{
   //Create named object and set condition
   fOptimal = 0;
   Set(cond);
   cf = 0;
   fOptOK = kFALSE;
   fNUsing = 0;
}

//_____________________________________________________________________________//

Bool_t KVParticleCondition::Test(const KVNucleus* nuc)
{
   //Evaluates the condition for the particle in question
   //
   //If no condition has been set (object created with default ctor) this returns
   //kTRUE for all nuclei.
   //
   //If optimisation fails (see method Optimize()), the condition will always
   //be evaluated as 'kFALSE' for all particles

   if (!IsSet()) return kTRUE;

   if (!fOptimal) Optimize();

   return (fOptOK ? fOptimal->Test(nuc) : kFALSE);
}

//_____________________________________________________________________________//

void KVParticleCondition::Set(const Char_t* cond)
{
   //Set particle condition criteria.
   //
   //These must be valid C++ expressions using _NUC_ instead and in place of
   //a pointer to the particle to be tested. Note that the methods used in the selection
   //do not have to be limited to the KVNucleus class. The 'real' class of the object
   //passed to Test() will be used to cast the base (KVNucleus) pointer up to the
   //required pointer type at execution.

   fCondition = cond;
   Ssiz_t ind = fCondition.Index(";");
   if (ind < 0) {
      fCondition_raw = fCondition;
      fCondition += ";";    //we add a ";" if there isn't already
   }
   else {
      fCondition_raw = fCondition.Strip(TString::kTrailing, ';');
   }
   fCondition_brackets = "(" + fCondition_raw + ")";
}

//_____________________________________________________________________________//

void KVParticleCondition::Copy(TObject& obj) const
{
   //Copy this to obj
   KVBase::Copy(obj);
   ((KVParticleCondition&) obj).Set(fCondition.Data());
   ((KVParticleCondition&) obj).fOptOK = fOptOK;
   // force first call to Test to try optimization
   // if existing optimized version exists in static list, pointer will be reset
   ((KVParticleCondition&) obj).fOptimal = nullptr;
   if (fClassName != "")((KVParticleCondition&) obj).SetParticleClassName(fClassName.Data());
   if (cf) {
      ((KVParticleCondition&) obj).SetClassFactory(cf);
   }
}

//_____________________________________________________________________________//

void KVParticleCondition::SetClassFactory(KVClassFactory* CF)
{
   //PRIVATE METHOD
   //Used by Copy
   CreateClassFactory();
   CF->Copy(*cf);
}

//_____________________________________________________________________________//

KVParticleCondition::KVParticleCondition(const KVParticleCondition& obj)
   : KVBase("KVParticleCondition", "Particle selection criteria")
{
   fOptimal = 0;
   cf = 0;
   fOptOK = kFALSE;
   fNUsing = 0;
   //Copy ctor
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVParticleCondition&) obj).Copy(*this);
#endif
}

//_____________________________________________________________________________//

KVParticleCondition& KVParticleCondition::operator=(const KVParticleCondition& obj)
{
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVParticleCondition&) obj).Copy(*this);
#endif
   return (*this);
}

//_____________________________________________________________________________//

KVParticleCondition& KVParticleCondition::operator=(const Char_t* sel)
{
   Set(sel);
   return (*this);
}

//_____________________________________________________________________________//

KVParticleCondition KVParticleCondition::operator&&(const KVParticleCondition& obj)
{
   //Perform boolean AND between the two selection conditions
   //If SetParticleClassName has been called for either of the two conditions,
   //it will be called for the resulting condition with the same value
   KVParticleCondition tmp(fCondition_brackets + " && " + obj.fCondition_brackets);
   if (fClassName != "") tmp.SetParticleClassName(fClassName);
   else if (obj.fClassName != "") tmp.SetParticleClassName(obj.fClassName);
   return tmp;
}

//_____________________________________________________________________________//

KVParticleCondition KVParticleCondition::operator||(const KVParticleCondition& obj)
{
   //Perform boolean OR between the two selection conditions
   //If SetParticleClassName has been called for either of the two conditions,
   //it will be called for the resulting condition with the same value
   KVParticleCondition tmp(fCondition_brackets + " || " + obj.fCondition_brackets);
   if (fClassName != "") tmp.SetParticleClassName(fClassName);
   else if (obj.fClassName != "") tmp.SetParticleClassName(obj.fClassName);
   return tmp;
}

KVParticleCondition& KVParticleCondition::operator|=(const KVParticleCondition& other)
{
   // Replace current condition with a logical 'OR' between itself and other

   KVParticleCondition tmp = *this || other;
   tmp.Copy(*this);
   return *this;
}

KVParticleCondition& KVParticleCondition::operator&=(const KVParticleCondition& other)
{
   // Replace current condition with a logical 'AND' between itself and other

   KVParticleCondition tmp = *this && other;
   tmp.Copy(*this);
   return *this;
}

//_____________________________________________________________________________//

void KVParticleCondition::AddExtraInclude(const Char_t* inc_file)
{
   //Optimisation of KVParticleCondition::Test implies the automatic generation
   //of a new class which implements the selection required by the user (see Optimize).
   //
   //If the user's condition depends on objects of classes other than the family
   //of particle classes (TLorentVector <- KVParticle <- KVNucleus ...) there will
   //not be by default the necessary '#include' directive for the classes in question
   //in the generated class; the required plugin for Test() to function will not
   //load. In this case, the user should call this method with the name of each
   //'#include' file to be added to the class implementation.
   //
   //Example:
   //    KVParticleCondition p("_NUC_->GetVpar()>=gDataAnalyser->GetKinematics()->GetNucleus(1)->GetVpar()");
   //
   //Optimization will not work, as the 'gIndra' pointer is declared in KVINDRA.h, which
   //is not a default '#include' file when the optimised class is generated; similarly,
   //gIndra->GetSystem() returns a pointer to a KVDBSystem : => #include "KVDBSystem.h";
   //and KVDBSystem::GetKinematics() returns a pointer to a KV2Body : => #include "KV2Body.h".
   //Therefore, for this condition to work, the user must first call the methods :
   //
   //    p.AddExtraInclude("KVINDRA.h");
   //    p.AddExtraInclude("KV2Body.h");
   //    p.AddExtraInclude("KVDBSystem.h");
   //
   //before the first call to p.Test() (when optimization occurs).

   CreateClassFactory();
   cf->AddImplIncludeFile(inc_file);
}

//_____________________________________________________________________________//

void KVParticleCondition::CreateClassFactory()
{
   //Initialises KVClassFactory object used for optimization if it doesn't exist

   if (cf) return;

   // unique name for new class
   TUUID unique;
   KVString new_class = unique.AsString();
   // only first 8 characters are unique
   new_class.Remove(8);
   new_class.Prepend("KVParticleCondition_");

   //create new class
   cf = new KVClassFactory(new_class.Data(), "Particle condition to test", "KVParticleCondition");
}

//_____________________________________________________________________________//

void KVParticleCondition::Optimize()
{
   //Generate a new class which inherits from KVParticleCondition but having a Test()
   //method which tests explicitly the condition which is set by the user.
   //
   //If needed, the KVNucleus pointer argument will be upcasted to the type given to SetParticleClassName().
   //
   //The new class is added to the list of plugins of type KVParticleCondition,
   //then an instance of the class is generated and a pointer to it stored in
   //member KVParticleCondition::fOptimal.
   //
   //This object is then used in the Test() method of this object to test the condition.
   //
   //If compilation fails, the condition will evaluate to kFALSE for all subsequent calls.

   /* check that the same condition has not already been optimized */
   fOptimal = (KVParticleCondition*)fgOptimized.FindObject(GetName());
   if (fOptimal) {
      Info("Optimize", "Using existing optimized condition %p", fOptimal);
      fOptimal->fNUsing++;
      fOptOK = kTRUE;
      return;
   }
   Info("Optimize", "Optimization of KVParticleCondition : %s", fCondition.Data());

   CreateClassFactory();
   KVString created_class_name = cf->GetClassName();
   //add Test() method
   cf->AddMethod("Test", "Bool_t");
   cf->AddMethodArgument("Test", "const KVNucleus*", "nuc");
   cf->AddHeaderIncludeFile("KVNucleus.h");

   //write body of method
   KVString body("   //Optimized Test method for particle condition\n");
   KVString pointer = "nuc";
   if (fClassName != "") {
      pointer.Form("((%s*)nuc)", fClassName.Data());
      //upcasting pointer - we need to add corresponding #include to '.cpp' file
      cf->AddImplIncludeFile(Form("%s.h", fClassName.Data()));
   }
   KVString tmp;
   tmp = fCondition;
   tmp.ReplaceAll("_NUC_", pointer.Data());
   body += "   return ";
   body += tmp;

   cf->AddMethodBody("Test", body);

   //generate .cpp and .h for new class
   cf->GenerateCode();

   //add plugin for new class
   gROOT->GetPluginManager()->AddHandler("KVParticleCondition", cf->GetClassName(), cf->GetClassName(),
                                         Form("%s+", cf->GetImpFileName()), Form("%s()", cf->GetClassName()));
   //load plugin
   TPluginHandler* ph;
   if (!(ph = LoadPlugin("KVParticleCondition", cf->GetClassName()))) {
      Error("Optimize", " *** Optimization failed for KVParticleCondition : %s", fCondition.Data());
      Error("Optimize", " *** Use method AddExtraInclude(const Char_t*) to give the names of all necessary header files for compilation of your condition.");
      Fatal("Optimize", " *** THIS CONDITION WILL BE EVALUATED AS kFALSE FOR ALL PARTICLES!!!");
      delete cf;
      cf = 0;
      //we set fOptimal to a non-zero value to avoid calling Optimize
      //every time that Test() is called subsequently.
      fOptimal = this;
      fOptOK = kFALSE;
      return;
   }
   fOptOK = kTRUE;
   delete cf;
   cf = 0;
   //execute constructor
   fOptimal = (KVParticleCondition*) ph->ExecPlugin(0);

   Info("Optimize", "fOptimal = %p", fOptimal);
   if (!fOptimal) {
      Error("Optimize", " *** Optimization failed for KVParticleCondition : %s", fCondition.Data());
      Error("Optimize", " *** Use method AddExtraInclude(const Char_t*) to give the names of all necessary header files for compilation of your condition.");
      Fatal("Optimize", " *** THIS CONDITION WILL BE EVALUATED AS kFALSE FOR ALL PARTICLES!!!");
      //we set fOptimal to a non-zero value to avoid calling Optimize
      //every time that Test() is called subsequently.
      fOptimal = this;
      fOptOK = kFALSE;
   }
   // add to list of optimized conditions
   fOptimal->SetName(GetName());
   fOptimal->fOptimizedClassName = created_class_name;
   fgOptimized.Add(fOptimal);
   fOptimal->fNUsing++;
   Info("Optimize", "Success");
}

void KVParticleCondition::Print(Option_t*) const
{
   //Print informations on object
   Info("Print", "object name = %s, address = %p", GetName(), this);
   cout << " * condition = " << fCondition.Data() << endl;
   cout << " * classname = " << fClassName.Data() << endl;
   cout << " * fOptimal = " << fOptimal << endl;
   cout << " * fNUsing = " << fNUsing << endl;
   if (cf) {
      cout << " * classfactory :" << endl;
      cf->Print();
   }
}

