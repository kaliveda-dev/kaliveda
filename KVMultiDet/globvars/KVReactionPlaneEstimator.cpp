#include "KVReactionPlaneEstimator.h"

void KVReactionPlaneEstimator::init()
{
   // Default initialization.
   //
   // Default frame for all calculations is "CM".
   //
   // Define the following values which can be retrieved with GetValue():
   //
   // Name  |  Index  |  Meaning
   // ------|---------|------------------------------------------
   // Q     |    0    | Magnitude of \f$\vec{Q}\f$
   // Phi   |    1    | Azimuthal angle of \f$\vec{Q}\f$
   // X     |    2    | \f$x\f$-component of \f$\vec{Q}\f$
   // Y     |    3    | \f$y\f$-component of \f$\vec{Q}\f$

   SetFrame("CM");
   ClearNameIndex();
   SetNameIndex("Q", 0);
   SetNameIndex("Phi", 1);
   SetNameIndex("X", 2);
   SetNameIndex("Y", 3);
}

Double_t KVReactionPlaneEstimator::getvalue_int(Int_t i) const
{
   // \returns the following values according to the given index:
   //
   // Name  |  Index  |  Meaning
   // ------|---------|------------------------------------------
   // Q     |    0    | Magnitude of \f$\vec{Q}\f$
   // Phi   |    1    | Azimuthal angle of \f$\vec{Q}\f$
   // X     |    2    | \f$x\f$-component of \f$\vec{Q}\f$
   // Y     |    3    | \f$y\f$-component of \f$\vec{Q}\f$

   switch (i) {
      case 0:
         return GetQ().Mag();
      case 1: {
            double phi = GetQ().Phi() * TMath::RadToDeg();
            return (phi < 0) * 360 + phi;
         }
      case 2:
         return GetQ().X();
      case 3:
         return GetQ().Y();
   }
   return -1;
}

TVector3 KVReactionPlaneEstimator::GetQForParticle(const KVNucleus* n)
{
   // \return \f$\vec{Q}\f$ to use with particle
   //
   // To avoid autocorrelation, the particle is not used in the determination of \f$\vec{Q}\f$,
   // i.e. this is the "one plane per particle" approach.

   const KVNucleus* N = dynamic_cast<const KVNucleus*>(n->GetFrame(GetFrame(), false));

   return GetSumObject() - weight_function(N) * N->GetTransverseMomentum();
}

Double_t delta_phiR_distri(Double_t* x, Double_t* par)
{
   // Eqn.(12) of Ollitrault, "Reconstructing azimuthal distributions in nucleus-nucleus collisions",
   // nucl-ex/9711003.
   //
   // Use to fit distribution of \f$\Delta\phi_R\f$, the difference in azimuthal angle between the reaction
   // planes determined for two random subevents containing half of all particles in the event,
   // in order to determine the \f$\chi\f$ parameter
   // which can be used to correct azimuthal distributions for the uncertainty in the determination
   // of the reaction plane.
   //
   // ~~~~{.cpp}
   // x[0] = delta phi
   // par[0] = chi
   // par[1] = normalisation factor
   // ~~~~{.cpp}

   if (par[0] < 0) return 0;
   double dPhi = x[0];
   double chiI = par[0] / TMath::Sqrt2();
   double z = TMath::Power(chiI, 2) * TMath::Cos(TMath::DegToRad() * dPhi);
   double distri = 2. / TMath::Pi() * (1 + TMath::Power(chiI, 2));
   distri += z * (TMath::BesselI0(z) + TMath::StruveL0(z));
   distri += TMath::Power(chiI, 2) * (TMath::BesselI1(z) + TMath::StruveL1(z));
   distri *= par[1] * TMath::Exp(-TMath::Power(chiI, 2)) / 2.;
   return distri;
}

TF1* KVReactionPlaneEstimator::GetDeltaPhiRFitFunction()
{
   // Creates and returns a TF1 corresponding to Eqn.(12) of Ollitrault, "Reconstructing azimuthal distributions in nucleus-nucleus collisions",
   // nucl-ex/9711003.
   //
   // Use to fit distribution of \f$\Delta\phi_R\f$, the difference in azimuthal angle between the reaction
   // planes determined for two random subevents containing half of all particles in the event,
   // in order to determine the \f$\chi\f$ parameter
   // which can be used to correct azimuthal distributions for the uncertainty in the determination
   // of the reaction plane.
   //
   // Parameters:
   // Index |  Meaning
   // ------|-----------------
   // 0     |   \f$\chi\f$
   // 1     |   Normalisation

   auto f = new TF1("DeltaPhiRFitFunction", delta_phiR_distri, 0, 180, 2);
   f->SetParNames("#chi", "Norm.");
   f->SetParLimits(0, 0, 5);
   f->SetParLimits(1, 1, 10000);
   f->SetParameters(1, 1);
   return f;
}
ClassImp(KVReactionPlaneEstimator)


