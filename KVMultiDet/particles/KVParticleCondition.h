/*
$Id: KVParticleCondition.h,v 1.3 2007/03/26 10:14:56 franklan Exp $
$Revision: 1.3 $
$Date: 2007/03/26 10:14:56 $
*/

//Created by KVClassFactory on Thu Nov 16 14:20:38 2006
//Author: franklan

#ifndef __KVPARTICLECONDITION_H
#define __KVPARTICLECONDITION_H

#include "KVBase.h"
#include "KVString.h"
#include "KVHashList.h"
#include "KVNucleus.h"

#ifdef USING_ROOT6
// can use with lambda functions from ROOT v6.00 onwards
// (ROOT6 is necessarily compiled in C++11 mode and rootcling can handle
// C++11 syntax and objects in headers)
#include <vector>
#include <functional>
#endif

class KVClassFactory;

/**
  \class KVParticleCondition
\brief Handles particle selection criteria for data analysis classes
\ingroup Analysis,NucEvents

A KVParticleCondition object can be used to select nuclei to include in data analysis
(see KVEventSelector::SetParticleConditions()) or in the calculation of global
variables (see KVVarGlob::SetSelection()). The Test() method returns true or false
for a given nucleus depending on whether or not the condition is fulfilled. Combinations of
selections can be performed using Boolean logic operations `&&` and `||`.

There are two ways to define and use KVParticleCondition objects:
  1. Using strings of pseudo-code
  2. Using lambda expressions (only with ROOT6 or later)

### Using strings of pseudo-code
These must be valid C++ expressions using `_NUC_` instead and in place of
a `const KVNucleus*` pointer to the particle to be tested, for example
~~~~{.cpp}
KVParticleCondition c1("_NUC_->GetZ()>2");
KVParticleCondition c2("_NUC_->GetVpar()>0");
~~~~
Note that the methods used in the selection
do not have to be limited to the methods of the KVNucleus class.
The 'real' class of the object
passed to Test() can be used to cast the base pointer up (or is it down?) to the
required pointer type at execution. In this case, you must call the method
SetParticleClassName() with the name of the class to use in the cast.

Note that the first call to Test() automatically causes the 'optimization' of the
KVParticleCondition, which means that a class implementing the required condition is generated
and compiled on the fly before continuing (see method Optimize()).

### Using lambda expressions (only with ROOT6 or later)
Lambda expressions were introduced in C++11 and provide an easy way to define small functions
on the fly inside code. The lambda must take a `const KVNucleus*` pointer as argument and return
a boolean:
~~~~~~{.cpp}
KVParticleCondition l1("Z>2", [](const KVNucleus* nuc){ return nuc->GetZ()>2; });
KVParticleCondition l2("Vpar>0", [](const KVNucleus* nuc){ return nuc->GetVpar()>0; });
~~~~~~
Note the first argument to the constructor is a name which the user is free to define
in order to remember what the condition does.

Like any lambda expressions, variables can be 'captured' from the surrounding scope, which
can be useful in some situations. For example, given the following definitions:
~~~~~~{.cpp}
int zmin = 3;
KVParticleCondition l3("Z>zmin", [&](const KVNucleus* nuc){ return nuc->GetZ()>=zmin; });
~~~~~~
then the limit for the selection can be changed dynamically like so:
~~~~~~{.cpp}
KVNucleus N("7Li");
l3.Test(&N);      ==> returns true
zmin=5;
l3.Test(&N);      ==> returns false
~~~~~~
*/

class KVParticleCondition : public KVBase {
   static KVHashList fgOptimized;// list of optimized particle conditions
   mutable Int_t fNUsing;//! number of classes using this as an optimized condition
#ifdef USING_ROOT6
   using LambdaFunc = std::function<bool(const KVNucleus*)>;
   mutable LambdaFunc fLambdaCondition;
   LambdaFunc fSavedLambda1, fSavedLambda2;// used by || and &&
   enum class LogOp { AND, OR } fOpType;

   void logical_operator_lambda_condition_test() const
   {
      // check if we were created by && or || and haven't been initialized yet
      if (IsLambda() && !IsSet()) {
         switch (fOpType) {
            case LogOp::AND:
               fLambdaCondition = [this](const KVNucleus * nuc) {
                  return (fSavedLambda1(nuc) && fSavedLambda2(nuc));
               };
               break;
            case LogOp::OR:
               fLambdaCondition = [this](const KVNucleus * nuc) {
                  return (fSavedLambda1(nuc) || fSavedLambda2(nuc));
               };
               break;
         }
      }
   }
#endif

protected:

   KVString fCondition;//string containing selection criteria with ";" at end
   KVString fCondition_raw;//'raw' condition, i.e. no ';'
   KVString fCondition_brackets;//condition with '(' and ')' around it

   mutable const KVParticleCondition* fOptimal;//!
   KVString fClassName;//!
   mutable KVClassFactory* cf;//! used to generate code for optimisation
   KVString fOptimizedClassName;//! name of generated class used for optimisation
   mutable Bool_t fOptOK;//!false if optimisation failed (can't load generated code)

   void Optimize() const;
   void CreateClassFactory() const;
   void SetClassFactory(KVClassFactory* CF);

public:

   KVParticleCondition();
   KVParticleCondition(const KVString& cond);
   KVParticleCondition(const Char_t* cond);
   KVParticleCondition(const KVParticleCondition&);
#ifdef USING_ROOT6
   KVParticleCondition(const KVString& name, const LambdaFunc& F)
      : KVBase(name, "KVParticleCondition"), fLambdaCondition(F)
   {
      // Create named object using lambda expression for condition
      // The lambda must take a `const KVNucleus*` pointer as argument and return
      // a boolean:
      // ~~~~~~{.cpp}
      // KVParticleCondition l1("Z>2", [](const KVNucleus* nuc){ return nuc->GetZ()>2; });
      // KVParticleCondition l2("Vpar>0", [](const KVNucleus* nuc){ return nuc->GetVpar()>0; });
      // ~~~~~~
      // Note the first argument to the constructor is a name which the user is free to define
      // in order to remember what the condition does.
      //
      // Like any lambda expressions, variables can be 'captured' from the surrounding scope, which
      // can be useful in some situations. For example, given the following definitions:
      // ~~~~~~{.cpp}
      // int zmin = 3;
      // KVParticleCondition l3("Z>zmin", [&](const KVNucleus* nuc){ return nuc->GetZ()>=zmin; });
      // ~~~~~~
      // then the limit for the selection can be changed dynamically like so:
      // ~~~~~~{.cpp}
      // KVNucleus N("7Li");
      // l3.Test(&N);      ==> returns true
      // zmin=5;
      // l3.Test(&N);      ==> returns false
      // ~~~~~~
      fOptimal = nullptr;
      cf = nullptr;
      fOptOK = kFALSE;
      fNUsing = 0;
   }
   bool IsLambda() const
   {
      // \returns true if this condition is defined using a lambda expression
      return (bool)fLambdaCondition || ((bool)fSavedLambda1 && (bool)fSavedLambda2);
   }
   KVParticleCondition(KVParticleCondition&& other) noexcept;
   KVParticleCondition& operator=(KVParticleCondition&& other) noexcept;
#endif
   virtual ~KVParticleCondition();
#ifdef USING_ROOT6
   void Set(const KVString& name, const LambdaFunc& F)
   {
      // Set condition using lambda expression (replace any existing definition).
      //
      // The lambda must take a `const KVNucleus*` pointer as argument and return
      // a boolean:
      // ~~~~~~{.cpp}
      // KVParticleCondition l1("Z>2", [](const KVNucleus* nuc){ return nuc->GetZ()>2; });
      // KVParticleCondition l2("Vpar>0", [](const KVNucleus* nuc){ return nuc->GetVpar()>0; });
      // ~~~~~~
      // Note the first argument to the constructor is a name which the user is free to define
      // in order to remember what the condition does.
      fLambdaCondition = F;
      SetName(name);
   }
#endif
   void Set(const KVString&);

   Bool_t Test(const KVNucleus* nuc) const
   {
      //Evaluates the condition for the particle in question
      //
      //If no condition has been set (object created with default ctor) this returns
      //kTRUE for all nuclei.
      //
      //If optimisation fails (see method Optimize()), the condition will always
      //be evaluated as 'kFALSE' for all particles

#ifdef USING_ROOT6
      logical_operator_lambda_condition_test();
#endif
      if (!IsSet()) return kTRUE;
#ifdef USING_ROOT6
      if (IsLambda()) return fLambdaCondition(nuc);
#endif
      if (!fOptimal) Optimize();

      return (fOptOK ? fOptimal->Test(nuc) : kFALSE);
   }

   Bool_t Test(const KVNucleus& nuc) const
   {
      //Evaluates the condition for the particle in question
      //
      //If no condition has been set (object created with default ctor) this returns
      //kTRUE for all nuclei.
      //
      //If optimisation fails (see method Optimize()), the condition will always
      //be evaluated as 'kFALSE' for all particles

      return Test(&nuc);
   }

   void SetParticleClassName(const Char_t* cl)
   {
      fClassName = cl;
   }
   void AddExtraInclude(const Char_t* inc_file);

   void Copy(TObject&) const;

   KVParticleCondition& operator=(const KVParticleCondition&);
   KVParticleCondition& operator=(const KVString&);
#ifdef USING_ROOT6
   KVParticleCondition& operator=(const LambdaFunc&);
#endif
   friend KVParticleCondition operator&&(const KVParticleCondition&, const KVParticleCondition&);
   friend KVParticleCondition operator||(const KVParticleCondition&, const KVParticleCondition&);
   KVParticleCondition& operator|=(const KVParticleCondition&);
   KVParticleCondition& operator&=(const KVParticleCondition&);

   void Print(Option_t* opt = "") const;
   static void PrintOptimizedList()
   {
      fgOptimized.Print();
   }
   Bool_t IsSet() const
   {
      // Return kTRUE if a condition/selection has been defined
      //
      // If this is not true, Test() will return true for all and any nuclei.
#ifdef USING_ROOT6
      return (fLambdaCondition || fCondition != "");
#else
      return fCondition != "";
#endif
   }

   ClassDef(KVParticleCondition, 2) //Implements parser of particle selection criteria
};

#endif
