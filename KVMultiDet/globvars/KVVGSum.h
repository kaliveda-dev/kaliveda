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
   virtual Double_t getvalue_void(void) const;

public:
   ROOT_FULL_SET_WITH_INIT(KVVGSum, KVVarGlobMean)

   void Init();
   void fill(const KVNucleus* c);    // Filling method

   ClassDef(KVVGSum, 0) //General global variable for calculating sums of various quantities
};
#endif
