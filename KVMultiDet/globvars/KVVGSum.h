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

/**
  \class KVVGSum
  \ingroup GlobalVariables
 \brief General global variable for calculating sums of various quantities

Most global variables involve a sum of some kind, whether it be
summing up the number of times that particles meets certain
criteria (multiplicity), summing some property of particles such as
transverse energy or charge, or calculating the mean value of such a
property for all particles.

Therefore the actual act of summing up is always the same,
the difference between one global variable and another is the
quantity which is summed, and the information which is finally
extracted from this sum.

A KVVGSum has three modes of operation: mult, sum, mean
In the first, "mult", it is used to count the number of times that a selection
condition is successfully met.
In "sum" mode, it is used to sum a property of all particles which meet
a given selection condition.
In "mean" mode, it calculates the mean value of a property of all particles
which meet a given selection condition.

### SETTING THE MODE OF OPERATION
Assuming a pointer to a KVVGSum, KVVGSum* vgs:

~~~~{.cpp}
    vgs->SetOption("mode", "mult");
    vgs->SetOption("mode", "sum");
    vgs->SetOption("mode", "mean");
~~~~

### SETTING THE PROPERTY TO BE SUMMED
To set the property to be summed, you must give the name of the method
to be called in order to obtain the desired quantity:

~~~~{.cpp}
    vgs->SetOption("method", "GetREtran");
    vgs->SetOption("method", "GetZ");
~~~~

If you need to give arguments for the method call, use option "args", e.g.:

~~~~{.cpp}
    vgs->SetOption("args", "3, \"string argument\", 0");
~~~~

By default, the method used must be defined in class KVNucleus or one of
its base classes. If you need to access methods in some derived class,
use the "class" option:

~~~~{.cpp}
    vgs->SetOption("class", "MyOwnParticleClass");
~~~~

### SETTING THE PARTICLE SELECTION CRITERIA
Use a KVParticleCondition to set some selection criteria, then
pass it as argument to base class method KVVVarGlob::SetSelection().

Example: calculate multiplicity of nuclei with Z<3

~~~~{.cpp}
    KVParticleCondition sel("_NUC_->GetZ()<3");
    vgs->SetSelection(sel);
    vgs->SetOption("mode", "mult");
~~~~

Example: calculate mean transverse energy of IMF with CM parallel
         velocities between -1.5 and +1.5 cm/ns

~~~~{.cpp}
    KVParticleCondition imf("_NUC_->GetZ()>=3 && _NUC_->GetZ()<=20");
    KVParticleCondition vpar("_NUC_->GetFrame(\"CM\")->GetVpar()>=-1.5 && _NUC_->GetFrame(\"CM\")->GetVpar()<=1.5");
    KVParticleCondition sel = imf && vpar;
    vgs->SetSelection(sel);
    vgs->SetOption("mode", "mean");
    vgs->SetOption("method", "GetEtran");
~~~~
    */

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
