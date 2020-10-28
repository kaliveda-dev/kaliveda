//Created by KVClassFactory on Tue Feb 19 17:16:59 2013
//Author: John Frankland,,,

#include "KVWilckeReactionParameters.h"

#include <TF1Derivative.h>

ClassImp(KVWilckeReactionParameters)

Double_t KVWilckeReactionParameters::e2_Wilcke = 1.438; //e**2
Double_t KVWilckeReactionParameters::mu_Wilcke = 931.5; //atomic mass unit
/* Numerical constants from W.D. Myers, Phys. Lett. B 30, 451 (1969) */
Double_t KVWilckeReactionParameters::a1_Myers = 15.677; //volume energy coefficient
Double_t KVWilckeReactionParameters::a2_Myers = 22.0; //surface energy coefficient
Double_t KVWilckeReactionParameters::J_Myers = 35.0; //symmetry energy coefficient
Double_t KVWilckeReactionParameters::K_Myers = 300.0; //compressibility coefficient
Double_t KVWilckeReactionParameters::L_Myers = 99.0; //density-symmetry coefficient
Double_t KVWilckeReactionParameters::M_Myers = 4.5; //symmetry anharmonicity coefficient
Double_t KVWilckeReactionParameters::Q_Myers = 25.0; //effective surface stiffness
Double_t KVWilckeReactionParameters::r0_Myers = 1.16; //nuclear radius constant
Double_t KVWilckeReactionParameters::c1_Myers = 0.745; //Coulomb energy coefficient

void KVWilckeReactionParameters::init()
{
   ZC = ZP + ZT;
   NP = AP - ZP;
   NT = AT - ZT;
   NC = NP + NT;
   AC = AP + AT;
   MU = AP * AT / (1.*AC);
   RINT = InteractionRadius(AP, AT);
   R0 = r0_Wilcke(AP, AT);
   RP = SharpRadius(AP);
   RT = SharpRadius(AT);
   CP = MatterHalfDensityRadius(AP);
   CT = MatterHalfDensityRadius(AT);
   CBAR = CP * CT / (CP + CT);
   RCP = ChargeRadius_Myers(ZP, AP);
   RCT = ChargeRadius_Myers(ZT, AT);
   RCTOTAL = RCP + RCT;
   V0 = BSS_V0(ZP, AP, ZT, AT);
   BSS_K = V0 - e2_Wilcke * ZP * ZT / RCTOTAL;
   BSS_N = e2_Wilcke * ZP * ZT / RCTOTAL / BSS_K;
   BSS_K /= pow(RCTOTAL, BSS_N);

   FISSIONTKE = TKESymFiss(ZC, AC);
   ASYMMFISSIONTKE = FISSIONTKE * 4.*ZP * ZT / (1.*ZC * ZC);
   GAMMA = NLDSurfaceTensionCoefficient(ZC, AC);
   // Factor to convert the dimensionless proximity potential function of
   // J. Blocki et al., Ann. Phys. (N. Y.) 105, 427 (1977)
   // into a nuclear potential V_N
   PROXFACTOR = 4 * TMath::Pi() * GAMMA * CBAR;
   RBARRIER = SWaveFusionBarrierRadius(ZP, AP, ZT, AT);
   // Maximum critical angular momentum for fusion
   Double_t S = CT + CP + 0.3;
   Double_t PHI = -0.96;
   LCRIT = PROXFACTOR * PHI + e2_Wilcke * ZP * ZT / S / S;
   LCRIT *= (-MU * mu_Wilcke * S * S * S / KVNucleus::hbar / KVNucleus::hbar);
   LCRIT = 7. / 5.*pow(LCRIT, 0.5);

   if (!fBSS) fBSS = new TF1("BSS", this, &KVWilckeReactionParameters::VC, 0, 25, 0, "KVWilckeReactionParameters", "VC");
   VC_RINT = fBSS->Eval(RINT);
   if (!fCMThetaQuart) fCMThetaQuart = new TF1("ThetaQuart", this, &KVWilckeReactionParameters::QuarterPointAngle, 0, 100, 0, "KVWilckeReactionParameters", "QuarterPointAngle");
   if (!fLmax) fLmax = new TF1("LMAX", this, &KVWilckeReactionParameters::Lmax, 0, 100, 0, "KVWilckeReactionParameters", "Lmax");
   if (!fSigmaR) fSigmaR = new TF1("SIGMA-R", this, &KVWilckeReactionParameters::SigmaR, 0, 100, 0, "KVWilckeReactionParameters", "SigmaR");
   if (!fProx) fProx = new TF1("PROX", this, &KVWilckeReactionParameters::ProxPot, 0, 25, 0, "KVWilckeReactionParameters", "ProxPot");
   if (!fPotential) fPotential = new TF1("HIPOT", this, &KVWilckeReactionParameters::Potential, 0, 25, 0, "KVWilckeReactionParameters", "Potential");
   if (!fCentPot) {
      fCentPot = new TF1("CENTPOT", this, &KVWilckeReactionParameters::CentrifugalPotential, 0, 25, 1, "KVWilckeReactionParameters", "CentrifugalPotential");
      fCentPot->SetNpx(1000);
      fCentPot->SetParName(0, "l [hbar]");
   }
   VRB = fPotential->Eval(RBARRIER);

   if (!fSigmaFus) fSigmaFus = new TF1("SIGMA-FUS", this, &KVWilckeReactionParameters::SigmaFus, 0, 1.e+02, 0, "KVWilckeReactionParameters", "SigmaFus");
}

KVWilckeReactionParameters::KVWilckeReactionParameters()
   : fBSS(0), fProx(0), fPotential(0), fCMThetaQuart(0), fLmax(0), fSigmaR(0), fSigmaFus(0), fCentPot(0)
{
   // Default constructor
}

KVWilckeReactionParameters::KVWilckeReactionParameters(const KVNucleus& proj, const KVNucleus& targ)
   : fBSS(0), fProx(0), fPotential(0), fCMThetaQuart(0), fLmax(0), fSigmaR(0), fSigmaFus(0), fCentPot(0)
{
   SetEntranceChannel(proj, targ);
}

KVWilckeReactionParameters::~KVWilckeReactionParameters()
{
   // Destructor
}

void KVWilckeReactionParameters::SetEntranceChannel(const KVNucleus& proj, const KVNucleus& targ)
{
   // (Re)set entrance channel to calculate
   ZP = proj.GetZ();
   AP = proj.GetA();
   ZT = targ.GetZ();
   AT = targ.GetA();
   init();
}


Double_t KVWilckeReactionParameters::epsilon_bar_Myers(Int_t Z, Int_t A)
{
   // epsilon_bar, Eq.(7) in W.D. Myers, Phys. Lett. B 30, 451 (1969)
   Double_t eps = (-2.*a2_Myers * pow(A, -THIRD) +
                   L_Myers * pow(delta_bar_Myers(Z, A), 2) +
                   c1_Myers * Z * Z * pow(A, -4. / 3.)) / K_Myers;
   return eps;
}

Double_t KVWilckeReactionParameters::delta_bar_Myers(Int_t Z, Int_t A)
{
   // delta_bar, Eq.(8) in W.D. Myers, Phys. Lett. B 30, 451 (1969)
   Double_t I = (A - 2.*Z) / (1.*A);
   Double_t delta = (I + (3 * c1_Myers / 8. / Q_Myers) * Z * Z * pow(A, -5. / 3.)) /
                    (1. + (9.*J_Myers / 4. / Q_Myers) * pow(A, -THIRD));
   return delta;
}

Double_t KVWilckeReactionParameters::RLDCriticalAngularMomentum(Int_t z, Int_t a)
{
   // S. Cohen, F. Plasil and W.J. Swiatecki, Ann. Phys. (N. Y.) 82, 557 (1974)
   // Rotating Liquid Drop model critical angular momentum
   // Only y_I is returned (normally valid for fissility parameter > 0.81)
   // as no parameterisation for y_II is given in the paper ???

   Double_t Esurf = (17.9439 / 0.9517) * NLDSurfaceTensionCoefficient(z, a) * pow(a, 2. / 3.);
   Double_t Ecoul = 0.7053 * z * z / pow(a, THIRD);
   Double_t x = Ecoul / 2. / Esurf; // fissility parameter
   Double_t y_I;
   if (x < 0.75) y_I = 0.2829 - 0.3475 * x - 0.0016 * x * x + 0.0501 * x * x * x;
   else y_I = (7. / 5.) * pow((1. - x), 2) - 4.5660 * pow(1. - x, 3.) + 6.7443 * pow(1 - x, 4.);
   return y_I;
}

void KVWilckeReactionParameters::Print() const
{
   printf("-------------------------------------------\n");
   printf("PARAMETERS INDEPENDENT OF BOMBARDING ENERGY\n");
   printf("-------------------------------------------\n\n");
   KVNucleus fus(ZC, AC);
   printf("ATOMIC NUMBERS:   ZP=%3d  ZT=%3d  ZC=%3d (%s)\n", ZP, ZT, ZC, fus.GetSymbol());
   printf("NEUTRON NUMBERS:  NP=%3d  NT=%3d  NC=%3d\n\n", NP, NT, NC);
   printf("AP**1/3=%7.3f AT**1/3=%7.3f\n", pow(AP, THIRD), pow(AT, THIRD));
   printf("REDUCED MASS NUMBER=%6.2f AP+AT=AC=%3d\n\n", MU, AC);
   printf("INTERACTION RADIUS  RINT=%5.2f fm   R0=%5.2f fm\n\n", RINT, R0);
   printf("MATTER HALF-DENSITY RADII [fm]:\n");
   printf("CP=%5.2f CT=%5.2f CT+CP=%5.2f CBAR=%5.2f\n\n", CP, CT, (CP + CT), CBAR);
   printf("EQUIVALENT SHARP SURFACE RADII [fm]:\n");
   printf("RP=%5.2f RT=%5.2f\n\n", RP, RT);
   printf("COULOMB RADII [fm]:\n");
   printf("RCP=%5.2f RCT=%5.2f RC=RCP+RCT=%5.2f\n\n", RCP, RCT, RCTOTAL);
   printf("BSS-COULOMB POTENTIAL [MeV]:\n");
   printf("VC(r)=%5.3f*ZP*ZT/r for r>RC\n", e2_Wilcke);
   printf("VC(r)=V0-K*r**n     for r<RC\n");
   printf("V0=%7.2f MeV  K=%7.5f  n=%5.3f\n", V0, BSS_K, BSS_N);
   printf("VC(RINT)=%6.1f MeV\n\n", VC_RINT);
   printf("FISSION-TKE=%5.0f MeV\n", FISSIONTKE);
   printf("ASYMM. FISSION-TKE=%5.0f MeV\n\n", ASYMMFISSIONTKE);
   printf("LIQUID DROP PARAMETERS:\n");
   printf("GAMMA=%6.3f MeV/fm**2  PROX-FACTOR=%6.2f MeV\n\n", GAMMA, PROXFACTOR);
   printf("FUSION RELATED PARAMETERS:\n");
   printf("R-BARRIER=%5.2f fm  V(RB)=%6.1f MeV\n", RBARRIER, VRB);
   printf("L-CRITICAL=%4.0f HBAR\n", LCRIT);

   printf("-------------------------------------------------------------------------\n");
   printf("EL/u   ELAB  ECM  ECM/VC   k   ETA    LMAX SGMAR SGFUS QP-CM QP-LP  EP-QP\n");
   printf("-------------------------------------------------------------------------\n");
   Double_t e = 1;
   Int_t nlines = 0;
   while (e < 51) {
      printf(" %4.1f  %4.0f  %4.0f %5.2f  %4.1f  %5.1f  %4.0f  %4.0f  %4.0f %5.1f %5.1f %4.0f\n",
             e, AP * e, ECM(e), ECM(e) / VC_RINT, k(e), Eta(e), Lmax(&e, 0),
             SigmaR(&e, 0), SigmaFus(&e, 0), TMath::RadToDeg()*QuarterPointAngle(&e, 0),
             TMath::RadToDeg()*ProjectileLabQP(e), ProjectileLabEQP(e));
      nlines++;
      if (e < 4) e += 1;
      else if (e < 12) e += 0.5;
      else if (e < 20) e += 1;
      else e += 5;
      if (!(nlines % 5)) printf("\n");
   }
   printf("************************************************************************\n");
}

Double_t KVWilckeReactionParameters::PotentialPocketRadius(Double_t l)
{
   // Find position (radial distance between centres) at which total potential has a
   // minimum for the given angular momentum. Returns -1.0 if no pocket.

   TF1Derivative df(GetCentrifugalPotential(l));
   TF1Derivative df2(GetCentrifugalPotential(l), 2);

   // look for extrema
   Double_t rmax = 20.;
   Double_t rmin = -1.0;
   for (int i = 2; i; --i) {
      rmin = df.GetX(0., 1., rmax);
      if (rmin == rmax) return -1.0; // no extrema
      // Test min/max ?
      if (df2.Eval(rmin) > 0) return rmin; // found minimum
      // That was a maximum. Try again.
      rmax = rmin - 0.1;
   }
   return -1.0;
}

Double_t KVWilckeReactionParameters::GetMaximumAngularMomentumWithPocket()
{
   // Retuns maximum angular momentum for which a pocket exists in the interaction potential
   // which corresponds to a lower potential energy than the maximum

//   Double_t lmin, lmax;
//   lmin = 0;
//   lmax = 200;
//   Double_t l = lmin;
//   while (lmax > lmin) {
//      if (PotentialPocketRadius(l) > 0.) {
//         if (lmax - lmin == 2.0) return l;
//         lmin = l;
//      } else {
//         if (l < 1) return 0;
//         lmax = l;
//      }
//      if (lmax - lmin == 1.0) return lmin;
//      l = TMath::Nint((lmin + lmax) / 2.);
//   }
//   return 0;
   Double_t l_last_pock = -1;
   Double_t l = 0;
   while (1) {
      Double_t Rmax = PotentialMaximumRadius(l);
      if (Rmax < 0) break;
      Double_t Rmin = PotentialPocketRadius(l);
      if (Rmin < 0) break;
      if (GetCentrifugalPotential(l)->Eval(Rmin) >= GetCentrifugalPotential(l)->Eval(Rmax)) break;
      else l_last_pock = l;
      ++l;
   }
   return l_last_pock;
}

Double_t KVWilckeReactionParameters::PotentialMaximumRadius(Double_t l)
{
   // Find position (radial distance between centres) at which total potential has a
   // maximum for the given angular momentum. Returns -1.0 if no maximum.

   TF1Derivative df(GetCentrifugalPotential(l));
   TF1Derivative df2(GetCentrifugalPotential(l), 2);

   // look for extrema
   Double_t rmax = 20.;
   Double_t rmin = 5.0;
   for (int i = 2; i; --i) {
      rmin = df.GetX(0., rmin, rmax);
      if (rmin == rmax) return -1.0; // no extrema
      // Test min/max ?
      if (df2.Eval(rmin) < 0) return rmin; // found minimum
      // That was a minimum. Try again.
      rmin += 0.1;
   }
   return -1.0;
}

Double_t KVWilckeReactionParameters::GetBassReactionCrossSection(Double_t e_sur_a)
{
   // Bass reaction cross-section [mb] for incident energy [MeV/nucleon]
   Double_t R1 = 1.12 * pow(AP, 1. / 3.) - 0.94 * pow(AP, -1. / 3.);
   Double_t R2 = 1.12 * pow(AT, 1. / 3.) - 0.94 * pow(AT, -1. / 3.);
   Double_t Rint = R1 + R2 + 3.2;
   Double_t VRINT = 1.44 * ZP * ZT / Rint - R1 * R2 / (R1 + R2);
   return 10 * TMath::Pi() * pow(Rint, 2) * (1. - VRINT / ECM(e_sur_a));
}
