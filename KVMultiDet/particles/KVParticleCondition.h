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

class KVParticleCondition : public KVBase {
   static KVHashList fgOptimized;// list of optimized particle conditions
   mutable Int_t fNUsing;//! number of classes using this as an optimized condition
#ifdef USING_ROOT6
   using LambdaFunc = std::function<bool(const KVNucleus*)>;
   mutable LambdaFunc fLambdaCondition;
   LambdaFunc fSavedLambda1, fSavedLambda2;// used by || and &&
   enum class LogOp { AND, OR } fOpType;
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
   KVParticleCondition(const char* cond);
   KVParticleCondition(const KVString& cond);
   KVParticleCondition(const KVParticleCondition&);
#ifdef USING_ROOT6
   KVParticleCondition(const KVString& name, const LambdaFunc& F)
      : KVBase(name, "KVParticleCondition"), fLambdaCondition(F)
   {
      // Create named object using lambda capture for condition
      fOptimal = nullptr;
      cf = nullptr;
      fOptOK = kFALSE;
      fNUsing = 0;
   }
   bool IsLambda() const
   {
      return (bool)fLambdaCondition || ((bool)fSavedLambda1 && (bool)fSavedLambda2);
   }
#endif
   virtual ~KVParticleCondition();
#ifdef USING_ROOT6
   void Set(const KVString& name, const LambdaFunc& F)
   {
      // set condition using lambda capture
      fLambdaCondition = F;
      SetName(name);
   }
#endif
   void Set(const KVString&);

   Bool_t Test(const KVNucleus*) const;

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
   KVParticleCondition operator&&(const KVParticleCondition&) const;
   KVParticleCondition operator||(const KVParticleCondition&) const;
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
#ifdef USING_ROOT6
      return (fLambdaCondition || fCondition != "");
#else
      return fCondition != "";
#endif
   }

   ClassDef(KVParticleCondition, 2) //Implements parser of particle selection criteria
};

#endif
