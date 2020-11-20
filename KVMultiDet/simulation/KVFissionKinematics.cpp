//Created by KVClassFactory on Mon May  2 14:00:47 2016
//Author: John Frankland,,,

#include "KVFissionKinematics.h"

#include <KVPosition.h>

ClassImp(KVFissionKinematics)

KVFissionKinematics::KVFissionKinematics(const KVNucleus& fis, Int_t Z1, Int_t A1)
   : KVBase(), fFission(2), fMother(fis), fDaughter(Z1, A1)
{
   // Initialize fission kinematics calculation
   //
   // 'fis' is the fissioning nucleus, we take into account its kinematics
   // KVFissionKinematics(fis);                // symmetric fission
   // KVFissionKinematics(fis, Z1);            // specify Z of one fission fragment, masses calculated from N/Z
   // KVFissionKinematics(fis, Z1, A1);        // specify Z & A of one fission fragment

   if (!Z1) {
      // set daughter to half of mother
      fDaughter.SetZandA(fis.GetZ() / 2, fis.GetA() / 2);
   }
   else if (!A1) {
      // mass of daughter not given. use N/Z ratio of mother.
      fDaughter.SetN(Z1 * fMother.GetNsurZ());
   }
}

const KVSimEvent* KVFissionKinematics::Generate()
{
   // Generate a fission event

   fFission.Clear();

   KVNucleus* ff1, *ff2;
   (ff1 = fFission.AddParticle())->SetZandA(fDaughter.GetZ(), fDaughter.GetA());
   (ff2 = fFission.AddParticle())->SetZandA(fMother.GetZ() - fDaughter.GetZ(), fMother.GetA() - fDaughter.GetA());

   Double_t vrel = ff1->GetFissionVelocity(ff2);
   Double_t mu   = ff1->GetMass() * ff2->GetMass() / (ff1->GetMass() + ff2->GetMass());

   KVPosition pp(0, 180, 0, 360);
   TVector3 fis_axis = pp.GetRandomDirection("isotropic");
   ff1->SetVelocity((mu * vrel / ff1->GetMass())*fis_axis);
   ff2->SetVelocity((-mu * vrel / ff2->GetMass())*fis_axis);

   if (fMother.GetVelocity().Mag2() > 0.) {
      // boost to frame of fissioning nucleus
      fFission.SetFrame("lab", -fMother.GetVelocity());
      ff1->SetVelocity(ff1->GetFrame("lab")->GetVelocity());
      ff2->SetVelocity(ff2->GetFrame("lab")->GetVelocity());
   }

   return &fFission;
}

