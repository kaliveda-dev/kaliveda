#include "KVZmax.h"

ClassImp(KVZmax)

void KVZmax::init()
{
   heaviest.SetOwner(kFALSE);
   fSorted = kFALSE;
   //set up list of indices
   for (int i = 1; i <= 50; i++)
      SetNameIndex(Form("Zmax%d", i), i - 1);
   fValueType = 'I'; // integer values
   SetMaxNumBranches(3);
}

void KVZmax::Copy(TObject& a) const
{
   ((KVZmax&) a).Reset();
   KVVarGlob::Copy(a);
   KVZmax& _a = dynamic_cast<KVZmax&>(a);
   heaviest.Copy(_a.heaviest);
   _a.fSorted = fSorted;
}


//_________________________________________________________________
std::vector<Double_t> KVZmax::GetValuePtr(void) const
{
   //Return vector containing ordered list of Z (all fragments)

   UInt_t size_event = heaviest.GetEntries();
   std::vector<Double_t> tmp;
   for (UInt_t u = 0; u < size_event; u++)
      tmp.push_back(GetValue(u));
   return tmp;
}

/**
 \example globvars_kvzmax.C
# Examples of use of the KVZmax global variable

 KVZmax sorts nuclei in an event as a function of decreasing Z.

 To execute this function, do:

    $ kaliveda
    kaliveda[0] .L globvars_kvzmax.C+
    kaliveda[1] zmax_example1()
    kaliveda[2] zmax_example2()

*/
