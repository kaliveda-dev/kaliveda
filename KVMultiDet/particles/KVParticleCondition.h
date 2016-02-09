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
class KVNucleus;
class KVClassFactory;

class KVParticleCondition : public KVBase {
   static KVHashList fgOptimized;// list of optimized particle conditions
   Int_t fNUsing;//! number of classes using this as an optimized condition

protected:

   KVString fCondition;//string containing selection criteria with ";" at end
   KVString fCondition_raw;//'raw' condition, i.e. no ';'
   KVString fCondition_brackets;//condition with '(' and ')' around it

   KVParticleCondition* fOptimal;//!
   KVString fClassName;//!
   KVClassFactory* cf;//! used to generate code for optimisation
   Bool_t fOptOK;//!false if optimisation failed (can't load generated code)

   void Optimize();
   void CreateClassFactory();
   void SetClassFactory(KVClassFactory* CF);

public:

   KVParticleCondition();
   KVParticleCondition(const KVParticleCondition&);
   KVParticleCondition(const Char_t* cond);
   virtual ~KVParticleCondition();
   virtual void Set(const Char_t*);
   virtual Bool_t Test(KVNucleus*);

   void SetParticleClassName(const Char_t* cl)
   {
      fClassName = cl;
   };
   void AddExtraInclude(const Char_t* inc_file);

   virtual void Copy(TObject&) const;

   KVParticleCondition& operator=(const KVParticleCondition&);
   KVParticleCondition& operator=(const Char_t*);
   KVParticleCondition operator&&(const KVParticleCondition&);
   KVParticleCondition operator||(const KVParticleCondition&);

   void Print(Option_t* opt = "") const;
   static void PrintOptimizedList()
   {
      fgOptimized.Print();
   }

   ClassDef(KVParticleCondition, 1) //Implements parser of particle selection criteria
};

#endif
