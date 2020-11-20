//Created by KVClassFactory on Thu Jan 30 10:29:29 2014
//Author: John Frankland,,,

#include "KVDataPatch.h"

ClassImp(KVDataPatch)



KVDataPatch::KVDataPatch()
{
   // Default constructor
}

KVDataPatch::KVDataPatch(const Char_t* name, const Char_t* title) : KVBase(name, title)
{
}

KVDataPatch::~KVDataPatch()
{
}

void KVDataPatch::Print(Option_t*) const
{
   // Print infos on patch

   std::cout << "PATCH NAME : [" << GetName() << "]" << std::endl;
   std::cout << "PATCH TITLE: " << GetTitle() << std::endl << std::endl;
   PrintPatchInfo();
   std::cout << std::endl;
}
