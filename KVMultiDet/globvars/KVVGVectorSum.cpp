#include "KVVGVectorSum.h"

ClassImp(KVVGVectorSum)

Double_t KVVGVectorSum::getvalue_int(Int_t i) const
{
   switch (i) {
      case 0:
      default:
         return GetSumObject().Z();
         break;
      case 1:
         return GetSumObject().X();
         break;
      case 2:
         return GetSumObject().Y();
         break;
   }
}

void KVVGVectorSum::init()
{
   SetNameIndex("Z", 0);
   SetNameIndex("X", 1);
   SetNameIndex("Y", 2);
}
