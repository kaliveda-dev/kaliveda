/*
$Id: KVParticleCondition.cpp,v 1.4 2007/03/26 10:14:56 franklan Exp $
$Revision: 1.4 $
$Date: 2007/03/26 10:14:56 $
*/

//Created by KVClassFactory on Thu Nov 16 14:20:38 2006
//Author: franklan

#include "KVParticleCondition.h"
#include "TROOT.h"
#include "Riostream.h"
#include "TSystem.h"
#include "KVClassFactory.h"
#include "TPluginManager.h"
#include "TUUID.h"
#include <utility>

using namespace std;

ClassImp(KVParticleCondition)

KVHashList KVParticleCondition::fgOptimized;

void KVParticleCondition::Set(const KVString& cond)
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

KVParticleCondition::KVParticleCondition(const KVString& cond)
   : KVBase(cond, "KVParticleCondition")
{
   //Create named object and set condition.
   //   This must be a valid C++ expression using `_NUC_` instead and in place of
   //   a `const KVNucleus*` pointer to the particle to be tested, for example
   //   ~~~~{.cpp}
   //   KVParticleCondition c1("_NUC_->GetZ()>2");
   //   KVParticleCondition c2("_NUC_->GetVpar()>0");
   //   ~~~~
   //   Note that the methods used in the selection
   //   do not have to be limited to the methods of the KVNucleus class.
   //   The 'real' class of the object
   //   passed to Test() can be used to cast the base pointer up (or is it down?) to the
   //   required pointer type at execution. In this case, you must call the method
   //   SetParticleClassName() with the name of the class to use in the cast.
   //
   //   Note that the first call to Test() automatically causes the 'optimization' of the
   //   KVParticleCondition, which means that a class implementing the required condition is generated
   //   and compiled on the fly before continuing (see method Optimize()).
   fOptimal = nullptr;
   Set(cond);
   cf = nullptr;
   fOptOK = kFALSE;
   fNUsing = 0;
}

KVParticleCondition::KVParticleCondition(const Char_t* cond)
   : KVBase(cond, "KVParticleCondition")
{
   //Create named object and set condition.
   //   This must be a valid C++ expression using `_NUC_` instead and in place of
   //   a `const KVNucleus*` pointer to the particle to be tested, for example
   //   ~~~~{.cpp}
   //   KVParticleCondition c1("_NUC_->GetZ()>2");
   //   KVParticleCondition c2("_NUC_->GetVpar()>0");
   //   ~~~~
   //   Note that the methods used in the selection
   //   do not have to be limited to the methods of the KVNucleus class.
   //   The 'real' class of the object
   //   passed to Test() can be used to cast the base pointer up (or is it down?) to the
   //   required pointer type at execution. In this case, you must call the method
   //   SetParticleClassName() with the name of the class to use in the cast.
   //
   //   Note that the first call to Test() automatically causes the 'optimization' of the
   //   KVParticleCondition, which means that a class implementing the required condition is generated
   //   and compiled on the fly before continuing (see method Optimize()).
   fOptimal = nullptr;
   Set(cond);
   cf = nullptr;
   fOptOK = kFALSE;
   fNUsing = 0;
}

KVParticleCondition::KVParticleCondition()
   : KVBase("KVParticleCondition", "Particle selection criteria")
{
   //default ctor
   fOptimal = nullptr;
   cf = nullptr;
   fOptOK = kFALSE;
   fNUsing = 0;
}

KVParticleCondition::~KVParticleCondition()
{
   //default dtor
   if (fOptimal) {
      // do not delete optimized condition unless we are the last to use it
      --(fOptimal->fNUsing);
      if (!(fOptimal->fNUsing)) {
         fgOptimized.Remove(const_cast<KVParticleCondition*>(fOptimal));
         delete fOptimal;
         fOptimal = nullptr;
      }
   }
   SafeDelete(cf);
}

//_____________________________________________________________________________//


//_____________________________________________________________________________//

void KVParticleCondition::Copy(TObject& obj) const
{
   //Copy this to obj
   KVBase::Copy(obj);
   ((KVParticleCondition&) obj).fCondition = fCondition;
   ((KVParticleCondition&) obj).fCondition_raw = fCondition_raw;
   ((KVParticleCondition&) obj).fCondition_brackets = fCondition_brackets;
#ifdef USING_ROOT6
   ((KVParticleCondition&) obj).fLambdaCondition = fLambdaCondition;
   ((KVParticleCondition&) obj).fSavedLambda1 = fSavedLambda1;
   ((KVParticleCondition&) obj).fSavedLambda2 = fSavedLambda2;
   ((KVParticleCondition&) obj).fOpType = fOpType;
#endif
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
   // Copy constructor. Create new condition which is a copy of existing condition, obj.
   fOptimal = nullptr;
   cf = nullptr;
   fOptOK = kFALSE;
   fNUsing = 0;
   obj.Copy(*this);
}

//_____________________________________________________________________________//

KVParticleCondition& KVParticleCondition::operator=(const KVParticleCondition& obj)
{
   // Set condition to be same as for existing KVParticleCondition object
   if (&obj != this) obj.Copy(*this);
   return (*this);
}

//_____________________________________________________________________________//

KVParticleCondition& KVParticleCondition::operator=(const KVString& sel)
{
   // Set condition using pseudo-code in string (replacing any previous definition).
   //
   //   This must be a valid C++ expression using `_NUC_` instead and in place of
   //   a `const KVNucleus*` pointer to the particle to be tested, for example
   //   ~~~~{.cpp}
   //   KVParticleCondition c1("_NUC_->GetZ()>2");
   //   KVParticleCondition c2("_NUC_->GetVpar()>0");
   //   ~~~~
   //   Note that the methods used in the selection
   //   do not have to be limited to the methods of the KVNucleus class.
   //   The 'real' class of the object
   //   passed to Test() can be used to cast the base pointer up (or is it down?) to the
   //   required pointer type at execution. In this case, you must call the method
   //   SetParticleClassName() with the name of the class to use in the cast.
   Set(sel);
   return (*this);
}

#ifdef USING_ROOT6
KVParticleCondition::KVParticleCondition(KVParticleCondition&& other) noexcept :
   KVBase(other),
#ifdef WITH_CPP14
   fNUsing(std::exchange(other.fNUsing, 0)),
#endif
   fLambdaCondition(std::move(other.fLambdaCondition)),
   fSavedLambda1(std::move(other.fSavedLambda1)),
   fSavedLambda2(std::move(other.fSavedLambda2)),
   fOpType(other.fOpType),
   fCondition(std::move(other.fCondition)),
   fCondition_brackets(std::move(other.fCondition_brackets)),
#ifdef WITH_CPP14
   fOptimal(std::exchange(other.fOptimal, nullptr)),
#endif
   fClassName(std::move(other.fClassName)),
#ifdef WITH_CPP14
   cf(std::exchange(other.cf, nullptr)),
#endif
   fOptimizedClassName(std::move(other.fOptimizedClassName))
#ifdef WITH_CPP14
   , fOptOK(std::exchange(other.fOptOK, false))
#endif
{
   // Move constructor
#ifndef WITH_CPP14
   fNUsing = other.fNUsing;
   other.fNUsing = 0;
   fOptimal = other.fOptimal;
   other.fOptimal = nullptr;
   cf = other.cf;
   other.cf = nullptr;
   fOptOK = other.fOptOK;
   other.fOptOK = false;
#endif
}

KVParticleCondition& KVParticleCondition::operator=(const LambdaFunc& f)
{
   // Set condition using lambda expression (replacing any previous definition).
   fLambdaCondition = f;
   return (*this);
}

KVParticleCondition& KVParticleCondition::operator=(KVParticleCondition&& other) noexcept
{
   // Move assignment operator
   KVBase::operator=(other);
#ifdef WITH_CPP14
   fNUsing = std::exchange(other.fNUsing, 0);
#else
   fNUsing = other.fNUsing;
   other.fNUsing = 0;
#endif
   fLambdaCondition = std::move(other.fLambdaCondition);
   fSavedLambda1 = std::move(other.fSavedLambda1);
   fSavedLambda2 = std::move(other.fSavedLambda2);
   fOpType = other.fOpType;
   fCondition = std::move(other.fCondition);
   fCondition_brackets = std::move(other.fCondition_brackets);
#ifdef WITH_CPP14
   fOptimal = std::exchange(other.fOptimal, nullptr);
#else
   fOptimal = other.fOptimal;
   other.fOptimal = nullptr;
#endif
   fClassName = std::move(other.fClassName);
#ifdef WITH_CPP14
   cf = std::exchange(other.cf, nullptr);
#else
   cf = other.cf;
   other.cf = nullptr;
#endif
   fOptimizedClassName = std::move(other.fOptimizedClassName);
#ifdef WITH_CPP14
   fOptOK = std::exchange(other.fOptOK, false);
#else
   fOptOK = other.fOptOK;
   other.fOptOK = false;
#endif
   return (*this);
}
#endif

//_____________________________________________________________________________//

KVParticleCondition operator&&(const KVParticleCondition& A, const KVParticleCondition& B)
{
   //Perform boolean AND between the two selection conditions
   //
   //If SetParticleClassName() has been called for either of the two conditions,
   //it will be called for the resulting condition with the same value
   //
   // Both conditions must be of same type, i.e. if one uses a lambda expression, the other
   // must also use a lambda expression.
   //
   // If one or other of the conditions is not set, we just return the condition which has been set.

#ifdef USING_ROOT6
   A.logical_operator_lambda_condition_test();
   B.logical_operator_lambda_condition_test();
#endif

   if (!(A.IsSet() && B.IsSet())) {
      // one or both conditions is/are not set
      if (!(A.IsSet() || B.IsSet())) {
         // neither is set: return blank (unset) condition
         return KVParticleCondition();
      }
      else if (A.IsSet()) return KVParticleCondition(A);
      else return KVParticleCondition(B);
   }
#ifdef USING_ROOT6
   // if lambdas are used (error if not both ?)
   if (A.IsLambda() || B.IsLambda()) {
      if (A.IsLambda() && B.IsLambda()) {
         KVParticleCondition tmp;
         tmp.fSavedLambda1 = A.fLambdaCondition;
         tmp.fSavedLambda2 = B.fLambdaCondition;
         tmp.fOpType = KVParticleCondition::LogOp::AND;
         tmp.SetName(Form("(%s) && (%s)", A.GetName(), B.GetName()));
         return tmp;
      }
      else {
         Error("operator&&", "Both KVParticleCondition objects must use lambda captures in order to do this");
         return KVParticleCondition();
      }
   }
#endif
   KVParticleCondition tmp;
   tmp.Set(A.fCondition_brackets + " && " + B.fCondition_brackets);
   if (A.fClassName != "") tmp.SetParticleClassName(A.fClassName);
   else if (B.fClassName != "") tmp.SetParticleClassName(B.fClassName);
   return tmp;
}

//_____________________________________________________________________________//

KVParticleCondition operator||(const KVParticleCondition& A, const KVParticleCondition& B)
{
   //Perform boolean OR between the two selection conditions
   //
   //If SetParticleClassName has been called for either of the two conditions,
   //it will be called for the resulting condition with the same value
   //
   // Both conditions must be of same type, i.e. if one uses a lambda expression, the other
   // must also use a lambda expression.
   //
   // If one or other of the conditions is not set, we just return the condition which has been set.
#ifdef USING_ROOT6
   A.logical_operator_lambda_condition_test();
   B.logical_operator_lambda_condition_test();
#endif

   if (!(A.IsSet() && B.IsSet())) {
      // one or both conditions is/are not set
      if (!(A.IsSet() || B.IsSet())) {
         // neither is set: return blank (unset) condition
         return KVParticleCondition();
      }
      else if (A.IsSet()) return KVParticleCondition(A);
      else return KVParticleCondition(B);
   }
#ifdef USING_ROOT6
   // if lambdas are used (error if not both ?)
   if (A.IsLambda() || B.IsLambda()) {
      if (A.IsLambda() && B.IsLambda()) {
         KVParticleCondition tmp;
         tmp.fSavedLambda1 = A.fLambdaCondition;
         tmp.fSavedLambda2 = B.fLambdaCondition;
         tmp.fOpType = KVParticleCondition::LogOp::OR;
         tmp.SetName(Form("(%s) || (%s)", A.GetName(), B.GetName()));
         return tmp;
      }
      else {
         Error("operator&&", "Both KVParticleCondition objects must use lambda captures in order to do this");
         return KVParticleCondition();
      }
   }
#endif
   KVParticleCondition tmp;
   tmp.Set(A.fCondition_brackets + " || " + B.fCondition_brackets);
   if (A.fClassName != "") tmp.SetParticleClassName(A.fClassName);
   else if (B.fClassName != "") tmp.SetParticleClassName(B.fClassName);
   return tmp;
}

KVParticleCondition& KVParticleCondition::operator|=(const KVParticleCondition& other)
{
   // Replace current condition with a logical 'OR' between itself and other
   //
   // Both conditions must be of same type, i.e. if one uses a lambda expression, the other
   // must also use a lambda expression.

   KVParticleCondition tmp = *this || other;
   tmp.Copy(*this);
   return *this;
}

KVParticleCondition& KVParticleCondition::operator&=(const KVParticleCondition& other)
{
   // Replace current condition with a logical 'AND' between itself and other
   //
   // Both conditions must be of same type, i.e. if one uses a lambda expression, the other
   // must also use a lambda expression.

   KVParticleCondition tmp = *this && other;
   tmp.Copy(*this);
   return *this;
}

//_____________________________________________________________________________//

void KVParticleCondition::AddExtraInclude(const Char_t* inc_file)
{
   //Optimisation of KVParticleCondition::Test() implies the automatic generation
   //of a new class which implements the selection required by the user (see Optimize()).
   //
   //If the user's condition depends on objects of classes other than the family
   //of particle classes (TLorentVector <- KVParticle <- KVNucleus ...) there will
   //not be by default the necessary '#include' directive for the classes in question
   //in the generated class; the required plugin for Test() to function will not
   //load. In this case, the user should call this method with the name of each
   //'#include' file to be added to the class implementation.
   //
   //Example:
   //~~~~~{.cpp}
   //    KVParticleCondition p("_NUC_->GetVpar()>=gDataAnalyser->GetKinematics()->GetNucleus(1)->GetVpar()");
   //~~~~~
   //Optimization will not work, as:
   //  - gDataAnalyser pointer to current analysis manager (KVDataAnalyser object), defined in KVDataAnalyser.h
   //  - gDataAnalyser->GetKinematics() returns a pointer to a KV2Body object, defined in KV2Body.h
   //
   //Therefore, for this condition to work, the user must first call the methods :
   //
   //~~~~~{.cpp}
   //    p.AddExtraInclude("KVDataAnalyser.h");
   //    p.AddExtraInclude("KV2Body.h");
   //~~~~~
   //
   //before the first call to p.Test() (when optimization occurs).

   CreateClassFactory();
   cf->AddImplIncludeFile(inc_file);
}

//_____________________________________________________________________________//

void KVParticleCondition::CreateClassFactory() const
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

void KVParticleCondition::Optimize() const
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

   fOptimal = (KVParticleCondition*)fgOptimized.FindObject(GetName());
   if (fOptimal) {  /* check that the same condition has not already been optimized */
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
   const_cast<KVParticleCondition*>(fOptimal)->SetName(GetName());
   const_cast<KVParticleCondition*>(fOptimal)->fOptimizedClassName = created_class_name;
   fgOptimized.Add(const_cast<KVParticleCondition*>(fOptimal));
   fOptimal->fNUsing++;
   Info("Optimize", "Success");
}

void KVParticleCondition::Print(Option_t*) const
{
   //Print informations on object
   if (fCondition != "") {
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
   else {
      cout << GetName() << endl;
   }
}

