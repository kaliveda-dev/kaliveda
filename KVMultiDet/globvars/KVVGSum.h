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
private:

   TClass* fClass; //class used to represent particles
   TMethodCall* fMethod; //method used to extract property of interest from particles
   Double_t fVal; //used to retrieve value of property for each particle

   enum {
      kMult = BIT(14), //set in "mult" mode
      kSum = BIT(15), //set in "sum" mode
      kMean = BIT(16), //set in "mean" mode
      kNoFrame = BIT(17), //set if property to be calculated is independent of reference frame
      kInitDone = BIT(18) //set if Init() has been called
   };

protected:
   void init_KVVGSum(void);
   virtual Double_t getvalue_void(void) const;

public:
   KVVGSum(void);       // default constructor
   KVVGSum(Char_t* nom);      // constructor with a name
   KVVGSum(const KVVGSum& a);// copy constructor

   virtual ~KVVGSum(void);    // destructor

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   void Init();

   KVVGSum& operator = (const KVVGSum& a); // operator =

   virtual void Fill(KVNucleus* c);    // Filling method

   ClassDef(KVVGSum, 0) //General global variable for calculating sums of various quantities
};
#endif
