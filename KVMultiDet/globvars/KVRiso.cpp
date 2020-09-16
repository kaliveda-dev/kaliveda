#include "KVRiso.h"

ClassImp(KVRiso)

void KVRiso::init()
{
   Riso = -1;
   Epar = 0.;
   Etrans = 0.;
   Mult = 0;
   SetNameIndex("Riso", 0);
   SetNameIndex("Epar", 1);
   SetNameIndex("Eperp", 2);
   SetNameIndex("Mult", 3);
}

void KVRiso::Copy(TObject& a) const
{
   KVVarGlob::Copy(a);
   ((KVRiso&) a).Riso = Riso;
   ((KVRiso&) a).Epar = Epar;
   ((KVRiso&) a).Etrans = Etrans;
   ((KVRiso&) a).Mult = Mult;
}

void KVRiso::Calculate()
{
   //Calculation of isotropy ratio from sums of parallel and
   //transverse energies
   //Limit to 100

   if (TMath::Abs(Epar) > 0) {
      Riso = TMath::Min(100., Etrans / (2. * Epar));
   }
   else Riso = -1;
}

Double_t KVRiso::getvalue_int(Int_t i) const
{
//
// Retourne la valeur suivant l'indice
//
//  Index               Meaning
//--------------------------------------------------------
//  0  (default)        Riso
//  1                   Sum of parallel kinetic energies
//  2                   Sum of transverse kinetic energies
//  3                   Number of nuclei included in calculation
   switch (i) {
      case 0:
         return Riso;
         break;
      case 1:
         return Epar;
         break;
      case 2:
         return Etrans;
         break;
      case 3:
         return Mult;
         break;
      default:
         Warning("GetValue(Int_t i)", "Index %d not valid. Riso returned.", i);
         return Riso;
         break;
   }
}

void KVRiso::fill(const KVNucleus* c)
{
   Double_t ep = c->GetKE();
   Double_t et = c->GetEtran();
   Epar += (ep - et);
   Etrans += et;
   ++Mult;
}
