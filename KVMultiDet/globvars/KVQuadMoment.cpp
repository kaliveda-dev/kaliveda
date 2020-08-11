//Created by KVClassFactory on Wed Jan 23 16:46:57 2013
//Author: John Frankland,,,

#include "KVQuadMoment.h"

ClassImp(KVQuadMoment)

KVQuadMoment::KVQuadMoment()
{
   // Default constructor
   init_KVQuadMoment();
}

//________________________________________________________________

KVQuadMoment::KVQuadMoment(const Char_t* nom) : KVVarGlob(nom)
{
   // Write your code here
   init_KVQuadMoment();
}

//________________________________________________________________

void KVQuadMoment::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVQuadMoment::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVVarGlob::Copy(obj);
   KVQuadMoment& CastedObj = (KVQuadMoment&)obj;
   for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
         CastedObj.matrix[i][j] = matrix[i][j];
      }
   }
}

void KVQuadMoment::init_KVQuadMoment(void)
{
   SetNameIndex("Qzz", 0);
   SetNameIndex("Qxx", 1);
   SetNameIndex("Qyy", 2);
}
void KVQuadMoment::Init(void)
{
   Reset();
}

//_________________________________________________________________
void KVQuadMoment::Reset(void)
{
   for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) matrix[i][j] = 0.;
}

void KVQuadMoment::fill(const KVNucleus* c)
{
   //Add the particle's contribution to the momentum tensor.

   TVector3 P = c->GetMomentum();
   Double_t P2 = P.Mag2();
   for (int i = 0; i < 3; i++) {
      for (int j = i; j < 3; j++) {
         if (i == j) matrix[i][j] += 3.*P(i) * P(j) - P2;
         else {
            Double_t a = 3.*P(i) * P(j);
            matrix[i][j] += a;
            matrix[j][i] += a;
         }
      }
   }
}

//_________________________________________________________________
Double_t KVQuadMoment::getvalue_int(Int_t i) const
{
   // i=0 : Qzz
   // i=1 : Qxx
   // i=2 : Qyy

   switch (i) {
      case 0:
         return matrix[2][2];
      case 1:
         return matrix[0][0];
      case 2:
         return matrix[1][1];
      default:
         break;
   }
   return 0;
}


