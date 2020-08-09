#include "KVSource.h"

ClassImp(KVSource)

void KVSource::init()
{
   // Initialisation called by ctor.
   // Set up correspondance between variable names and index.

   SetNameIndex("Ex", 0);
   SetNameIndex("Z", 1);
   SetNameIndex("A", 2);
   SetNameIndex("Vz", 3);
   SetNameIndex("Vx", 4);
   SetNameIndex("Vy", 5);
   SetNameIndex("Theta", 6);
   SetNameIndex("Mult", 7);
   fMult = 0;
}

Double_t KVSource::getvalue_int(Int_t i) const
{
// returns the i-th value
//
// Index  Meaning
//--------------------------------------
// 0    E*
// 1    Zsource
// 2    Asource
// 3    Vz
// 4    Vx
// 5    Vy
// 6    Theta
// 7    Mult
//
//
   Double_t rval = 0;
   switch (i) {
      case 0:
         rval = GetSumObject().GetExcitEnergy();
         break;

      case 1:
         rval = GetSumObject().GetZ();
         break;

      case 2:
         rval = GetSumObject().GetA();
         break;

      case 3:
         rval = GetSumObject().GetV().Z();
         break;

      case 4:
         rval = GetSumObject().GetV().X();
         break;

      case 5:
         rval = GetSumObject().GetV().Y();
         break;

      case 6:
         rval = GetSumObject().GetTheta();
         break;

      case 7:
         rval = fMult;
         break;

      default :
         rval = 0.;
         break;
   }
   return rval;
}

Char_t KVSource::GetValueType(Int_t i) const
{
   // Returns 'D' for all floating-point values,
   // returns 'I' for source Z and A and Mult
   static Char_t val_type = 'I';
   if ((i > 0 && i < 3) || (i == 7)) return val_type;
   return KVVarGlob::GetValueType(i);
}
