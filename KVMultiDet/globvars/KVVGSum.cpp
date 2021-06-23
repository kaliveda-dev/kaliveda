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

