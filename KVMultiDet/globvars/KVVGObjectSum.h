#ifndef __KVVGOBJECTSUM_H
#define __KVVGOBJECTSUM_H

#include "KVVarGlob.h"

/**
 \class KVVGObjectSum
 \brief Global variable calculating sum of objects
 \ingroup GlobalVariables
 \tparam SumObject type of the objects to be summed

This global variable can calculate sums of objects of any type as long as:
   - the type is DefaultConstructible;
   - the type is CopyAssignable;
   - operator+= is defined for the type

 \author John Frankland
 \date Sun Aug  9 16:13:10 2020
*/

template
<class SumObject>
class KVVGObjectSum : public KVVarGlob {
   Int_t fMult;// number of objects summed
   SumObject fResult;// result of summing objects

protected:
   void Add(const SumObject& obj)
   {
      fResult += obj;
      ++fMult;
   }
public:
   KVVGObjectSum()
      : KVVarGlob()
   {}
   KVVGObjectSum(const Char_t* nom)
      : KVVarGlob(nom)
   {}
   ROOT_COPY_CTOR(KVVGObjectSum, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVVGObjectSum)
   virtual ~KVVGObjectSum() {}
   void Copy(TObject& obj) const
   {
      KVVarGlob::Copy(obj);
      dynamic_cast<KVVGObjectSum<SumObject>&>(obj).fResult = fResult;
   }

   void Reset()
   {
      // Reset summed object by calling default ctor
      fResult = SumObject();
      fMult = 0;
   }

   const SumObject& GetSumObject() const
   {
      // return const reference to the summed object
      return fResult;
   }

   Int_t GetMult() const
   {
      // \returns number of objects that were summed
      return fMult;
   }

   ClassDef(KVVGObjectSum, 1) //Global variable calculating sum of objects
};

#endif
