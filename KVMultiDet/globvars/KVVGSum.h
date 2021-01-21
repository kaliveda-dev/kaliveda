/*
$Id: KVVGSum.h,v 1.2 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.2 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Thu Nov 16 10:42:38 2006
//Author: franklan

#ifndef __KVVGSUM_H
#define __KVVGSUM_H

#include "KVVarGlobMean.h"
#include "TMethodCall.h"
#include "TROOT.h"
#include "TClass.h"

class KVVGSum: public KVVarGlobMean {

   TClass* fClass; //class used to represent particles
   unique_ptr<TMethodCall> fMethod; //method used to extract property of interest from particles
   Double_t fVal; //used to retrieve value of property for each particle

   enum {
      kMult = BIT(14), //set in "mult" mode
      kSum = BIT(15), //set in "sum" mode
      kMean = BIT(16), //set in "mean" mode
      kNoFrame = BIT(17), //set if property to be calculated is independent of reference frame
      kInitDone = BIT(18) //set if Init() has been called
   };
   void init(void);

protected:
   Double_t getvalue_int(Int_t index) const
   {
      // Returns value based on position in name-index list, not the actual index:
      //~~~~{.cpp}
      // i.e. mode = mult: getvalue_int(0) returns value of "Mult" (index:4)
      // i.e. mode = sum:  getvalue_int(0) returns value of "Sum" (index:2)
      // i.e. mode = mean: getvalue_int(0) returns value of "Mean" (index:0), getvalue_int(1) returns value of "RMS" (index:1)
      //~~~~
      return KVVarGlobMean::getvalue_int(GetIndexAtListPosition(index));
   }

   Double_t getvalue_char(const Char_t* name) const
   {
      return KVVarGlobMean::getvalue_int(GetNameIndex(name));
   }

public:
   ROOT_FULL_SET_WITH_INIT(KVVGSum, KVVarGlobMean)

   void Init();
   void fill(const KVNucleus* c);    // Filling method

   virtual TString GetValueName(Int_t i) const
   {
      // Returns name of value associated with index 'i':
      //~~~~{.cpp}
      // i.e. mode = mult: GetValueName(0) returns "Mult"
      // i.e. mode = sum:  GetValueName(0) returns "Sum"
      // i.e. mode = mean: GetValueName(0) returns "Mean", GetValueName(1) returns "RMS"
      //~~~~
      if (i < GetNumberOfValues()) {
         return GetNameAtListPosition(i);
      }
      return TString("unknown");
   }

   ClassDef(KVVGSum, 0) //General global variable for calculating sums of various quantities
};
#endif
