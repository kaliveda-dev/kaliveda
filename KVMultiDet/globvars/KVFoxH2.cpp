#include "KVFoxH2.h"

ClassImp(KVFoxH2)

void KVFoxH2::init(void)
{
   // Initialisation of the fields of the KVFoxH2 class
   // Set reference frame to "CM" by default

   den = 0;
   num = 0;
   SetFrame("CM");
   fType = kTwoBody;
}

void KVFoxH2::Copy(TObject& a) const
{
   KVVarGlob1::Copy(a);
   ((KVFoxH2&)a).den = den;
   ((KVFoxH2&)a).num = num;
}

void KVFoxH2::fill2(const KVNucleus* n1, const KVNucleus* n2)
{
   // Calculate contribution to H(2) for the given pair of nuclei.
   // By default, momenta in "CM" frame are used. (change with SetFrame()).
   // NOTE: if one of the nuclei has zero momentum (strange, but could happen)
   // it is excluded from the sum.

   Double_t p1, p2, cos_th_rel;

   if (n1 == n2) {
      p1 = p2 = n1->GetMomentum().Mag();
      cos_th_rel = 1.;
   }
   else {
      p1 = n1->GetMomentum().Mag();
      p2 = n2->GetMomentum().Mag();
      if ((p1 * p2) > 0.0) cos_th_rel = n1->GetMomentum().Dot(n2->GetMomentum()) / (p1 * p2);
      else cos_th_rel = 1.;
   }
   if ((p1 * p2) > 0.) {
      num += p1 * p2 * (3.*cos_th_rel * cos_th_rel - 1.);
      den += 2.*p1 * p2;
   }
}

void KVFoxH2::Init(void)
{
   KVVarGlob1::Init();
   den = 0;
   num = 0;
}

void KVFoxH2::Reset(void)
{
   KVVarGlob1::Init();
   den = 0;
   num = 0;
}

void KVFoxH2::Calculate()
{
   if (den > 0)
      SetValue(num / den);
   else
      SetValue(-99);
}

