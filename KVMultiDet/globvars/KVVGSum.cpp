/*
$Id: KVVGSum.cpp,v 1.2 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.2 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Thu Nov 16 10:42:38 2006
//Author: John Frankland

#include "KVVGSum.h"
#include "TClass.h"
#include "TROOT.h"

ClassImp(KVVGSum)

////////////////////////////////////////////////////////////////////////////////
//  KVVGSum
// General global variable for calculating sums of various quantities
//
//Most global variables involve a sum of some kind, whether it be
//summing up the number of times that particles meets certain
//criteria (multiplicity), summing some property of particles such as
//transverse energy or charge, or calculating the mean value of such a
//property for all particles.
//
//Therefore the actual act of summing up is always the same,
//the difference between one global variable and another is the
//quantity which is summed, and the information which is finally
//extracted from this sum.
//
//A KVVGSum has three modes of operation: mult, sum, mean
//In the first, "mult", it is used to count the number of times that a selection
//condition is successfully met.
//In "sum" mode, it is used to sum a property of all particles which meet
//a given selection condition.
//In "mean" mode, it calculates the mean value of a property of all particles
//which meet a given selection condition.
//
//SETTING THE MODE OF OPERATION
//Assuming a pointer to a KVVGSum, KVVGSum* vgs:
//
//    vgs->SetOption("mode", "mult");
//    vgs->SetOption("mode", "sum");
//    vgs->SetOption("mode", "mean");
//
//SETTING THE PROPERTY TO BE SUMMED
//To set the property to be summed, you must give the name of the method
//to be called in order to obtain the desired quantity:
//
//    vgs->SetOption("method", "GetREtran");
//    vgs->SetOption("method", "GetZ");
//
//If you need to give arguments for the method call, use option "args", e.g.:
//
//    vgs->SetOption("args", "3, \"string argument\", 0");
//
//By default, the method used must be defined in class KVNucleus or one of
//its base classes. If you need to access methods in some derived class,
//use the "class" option:
//
//    vgs->SetOption("class", "MyOwnParticleClass");
//
//SETTING THE PARTICLE SELECTION CRITERIA
//Use a KVParticleCondition to set some selection criteria, then
//pass it as argument to base class method KVVVarGlob::SetSelection().
//
//Example: calculate multiplicity of nuclei with Z<3
//    KVParticleCondition sel("_NUC_->GetZ()<3");
//    vgs->SetSelection(sel);
//    vgs->SetOption("mode", "mult");
//
//Example: calculate mean transverse energy of IMF with CM parallel
//         velocities between -1.5 and +1.5 cm/ns
//    KVParticleCondition imf("_NUC_->GetZ()>=3 && _NUC_->GetZ()<=20");
//    KVParticleCondition vpar("_NUC_->GetFrame(\"CM\")->GetVpar()>=-1.5 && _NUC_->GetFrame(\"CM\")->GetVpar()<=1.5");
//    KVParticleCondition sel = imf && vpar;
//    vgs->SetSelection(sel);
//    vgs->SetOption("mode", "mean");
//    vgs->SetOption("method", "GetEtran");
////////////////////////////////////////////////////////////////////////////////

void KVVGSum::init(void)
{
   ClearNameIndex();
   SetMaxNumBranches(-1);
   fClass = nullptr;
   fVal = 0;
}

//_________________________________________________________________

void KVVGSum::fill(const KVNucleus* c)
{
   if (fMethod.get()) {
      fMethod->Execute(const_cast<KVNucleus*>(c), fVal);
      FillVar(fVal, 1);
   }
   else
      FillVar(1, 1);
}

//_________________________________________________________________

void KVVGSum::Init()
{
   //Must be called at least once before beginning calculation in order to
   //set mode of operation, get pointer to method used to calculate variable,
   //etc. etc.

   KVVarGlobMean::Init();

   if (TestBit(kInitDone)) return;

   SetBit(kInitDone);

   //Analyse options and set internal flags
   //Info("Init", "Called for %s", GetName());

   //SET MODE OF OPERATION
   if (GetOptionString("mode") == "mult") {
      SetBit(kMult);
      fValueType = 'I'; // integer type for automatic TTree branch
      SetNameIndex("Mult", 4);
   }
   else if (GetOptionString("mode") == "sum") {
      SetBit(kSum);
      SetNameIndex("Sum", 2);
   }
   else if (GetOptionString("mode") == "mean") {
      SetBit(kMean);
      // redefine "Mean" and "RMS" indices
      SetNameIndex("Mean", 0);
      SetNameIndex("RMS", 1);
   }
   else SetBit(kSum); //sum by default if unknown mode given

   //Info("Init", "mode=%s", GetOptionString("mode").Data());

   //SET UP METHOD CALL
   if (IsOptionGiven("class")) fClass = TClass::GetClass(GetOptionString("class"));
   else fClass = TClass::GetClass("KVNucleus");

   if (!fClass) {
      Fatal("Init", "Failed to load class requested as option: %s", GetOptionString("class").Data());
   }
   if (IsOptionGiven("method")) {
      if (IsOptionGiven("args"))
         fMethod.reset(new TMethodCall(fClass, GetOptionString("method").Data(), GetOptionString("args").Data()));
      else
         fMethod.reset(new TMethodCall(fClass, GetOptionString("method").Data(), ""));
      //Info("Init", "Method = %s Params = %s", fMethod->GetMethodName(),
      //     fMethod->GetParams());
      // if we are summing an integer quantity, make automatic TTree branch with integer type
      if (fMethod->ReturnType() == TMethodCall::kLong && TestBit(kSum)) fValueType = 'I';
   }
}

