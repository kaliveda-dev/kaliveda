//Created by KVClassFactory on Tue Feb 19 17:16:59 2013
//Author: John Frankland,,,

#ifndef __KVWILCKEREACTIONPARAMETERS_H
#define __KVWILCKEREACTIONPARAMETERS_H

#define THIRD (1./3.)
#include "KVNucleus.h"
#include "TF1.h"
#include "TH1.h"
#include "TPad.h"
#include <TF2.h>

class KVWilckeReactionParameters {
   /* The following variables are exactly copied from pp. 395-400 of Wilcke et al. */
   Int_t ZP, ZT; //Atomic number of the projectile, target
   Int_t NP, NT; //Neutron number of the projectile, target
   Int_t AP, AT; //Mass number of the projectile, target
   Int_t ZC;
   Int_t NC;
   Int_t AC;

   Double_t MU;
   Double_t RINT;
   Double_t R0;
   Double_t RP, RT; //Sharp radius
   Double_t CP, CT; //Matter half-density radii
   Double_t CBAR;
   Double_t RCP, RCT, RCTOTAL; //Coulomb radius
   static Double_t e2_Wilcke;//e**2 = 1.438 is value used by Wilcke et al.
   static Double_t mu_Wilcke;//mu = 931.5 is value used by Wilcke et al.
   Double_t V0;//BSS potential at r=0
   Double_t BSS_K, BSS_N;
   Double_t VC_RINT;//BSS Coulomb potential at Rint
   Double_t FISSIONTKE;//TKE for symmetric fission of combined system
   Double_t ASYMMFISSIONTKE;//TKE of completely relaxed events in strongly damped collisions
   Double_t GAMMA;//Nuclear liquid drop surface-tension coefficient
   Double_t PROXFACTOR;//Proximity potential factor
   Double_t RBARRIER;//Fusion barrier radius RB for s-waves
   Double_t LCRIT;//The maximum critical angular momentum for fusion
   Double_t VRB;//The total conservative potential at r=RB for s-waves

   /* Numerical constants from W.D. Myers, Phys. Lett. B 30, 451 (1969) */
   static Double_t a1_Myers;//volume energy coefficient
   static Double_t a2_Myers;//surface energy coefficient
   static Double_t J_Myers;//symmetry energy coefficient
   static Double_t K_Myers;//compressibility coefficient
   static Double_t L_Myers;//density-symmetry coefficient
   static Double_t M_Myers;//symmetry anharmonicity coefficient
   static Double_t Q_Myers;//effective surface stiffness
   static Double_t c1_Myers;//Coulomb energy coefficient
   static Double_t r0_Myers;//nuclear radius constant

   TF1* fBSS;//BSS Coulomb potential for heavy-ions
   TF1* fProx;//Nuclear proximity potential for heavy-ions
   TF1* fPotential;//total (nuclear+coulomb) potential for heavy-ions
   TF1* fCMThetaQuart;//CM quarter point angle
   TF1* fLmax;//Grazing angular momentum
   TF1* fSigmaR;//Reaction cross section
   TF1* fSigmaFus;//Fusion cross section
   TF1* fCentPot;

   void init();

public:
   KVWilckeReactionParameters();
   KVWilckeReactionParameters(const KVNucleus& proj, const KVNucleus& targ);
   virtual ~KVWilckeReactionParameters();

   static Double_t InteractionRadius(Int_t aproj, Int_t atarg)
   {
      // Interaction radius [fm]
      return MatterHalfDensityRadius(atarg) + MatterHalfDensityRadius(aproj) + 4.49 -
             (MatterHalfDensityRadius(atarg) + MatterHalfDensityRadius(aproj)) / 6.35;
   }
   static Double_t r0_Wilcke(Int_t aproj, Int_t atarg)
   {
      // R0 [fm]
      return InteractionRadius(aproj, atarg) / (pow(aproj, THIRD) + pow(atarg, THIRD));
   }
   static Double_t SharpRadius(Int_t A)
   {
      // Equivalent sharp radius of nucleus mass A
      return (1.28 * pow(A, THIRD) - 0.76 + 0.8 * pow(A, -THIRD));
   }
   static Double_t MatterHalfDensityRadius(Int_t A)
   {
      // Matter half-density radius for nucleus mass A
      // with width parameter b=1fm
      return (SharpRadius(A) * (1. - pow(SharpRadius(A), -2.)));
   }
   static Double_t epsilon_bar_Myers(Int_t Z, Int_t A);
   static Double_t delta_bar_Myers(Int_t Z, Int_t A);
   static Double_t ChargeRadius_Myers(Int_t Z, Int_t A)
   {
      // Charge (Coulomb) radius for nucleus charge Z mass A
      return r0_Myers * pow(2.*Z / (1. - 3.*epsilon_bar_Myers(Z, A)) / (1. - delta_bar_Myers(Z, A)), THIRD);
   }
   static Double_t BSS_V0(Int_t zp, Int_t ap, Int_t zt, Int_t at)
   {
      // Corrected V0 formula (2.24) from Bondorf et al. Phys. Rep. C15, 83 (1974)
      // In Wilcke et al. there is a mistake: (RCT**1/3 +  RCP**1/3)**3
      // instead of (RCT**3+RCP**3)**1/3
      // The Wilcke formula gives negative V0 values and an incorrect form of potential.
      // The actual values in Wilcke correspond to the correct Bondorf formula
      Double_t v0  = pow((zp + zt), 2.) / pow(pow(ChargeRadius_Myers(zt, at), 3.) + pow(ChargeRadius_Myers(zp, ap), 3.), THIRD);
      v0 -= (pow(zt, 2.) / ChargeRadius_Myers(zt, at) + pow(zp, 2.) / ChargeRadius_Myers(zp, ap));
      v0 *= 3.*e2_Wilcke / 5.;
      return v0;
   }
   Double_t VC(Double_t* r, Double_t*)
   {
      // BSS Coulomb potential for heavy ions with separation r
      if (*r >= RCTOTAL) {
         return (ZP * ZT * e2_Wilcke / (*r));
      }
      if (*r == 0) return V0;
      return (V0 - BSS_K * pow(*r, BSS_N));
   }
   Double_t ProxPot(Double_t* r, Double_t*)
   {
      // Nuclear proximity potential for nuclei separated by r
      // J. Blocki, J. Randrup, W.J. Swiatecki, and C.F. Tsang
      // Ann. Phys. (N. Y.) 105, 427-462 (1977)
      static Double_t zeta1 = 1.2511;
      static Double_t zeta0 = 2.54;
      static Double_t Kprox = 0.0852;

      Double_t Phi;
      Double_t zeta = *r - CP - CT;//surface separation
      Double_t dzeta = zeta - zeta0;
      if (zeta <= zeta1) {
         Phi = -0.5 * pow(dzeta, 2) - Kprox * pow(dzeta, 3);
      } else {
         Phi = -3.437 * exp(-zeta / 0.75);
      }
      return PROXFACTOR * Phi;
   }
   Double_t Potential(Double_t* r , Double_t*)
   {
      // Total heavy-ion potential (nuclear proximity+coulomb) at distance r
      return ProxPot(r, 0) + VC(r, 0);
   }
   Double_t CentrifugalPotential(Double_t* x , Double_t* l)
   {
      // Total heavy-ion potential (nuclear proximity+coulomb+centrifugal) at distance r
      // x[0] = r
      // l[0] = angular momentum

      Double_t R = TMath::Max(0.1, x[0]);
      Double_t Vcent = l[0] * (l[0] + 1.) * pow(KVNucleus::hbar, 2) / (2.*MU * mu_Wilcke * R * R);
      return ProxPot(&x[0], 0) + VC(&x[0], 0) + Vcent;
   }
   TF1* GetCentrifugalPotential(Double_t e_sur_a, Double_t b) const
   {
      // Total (nuclear+coulomb+centrifugal) potential for given beam energy (E/A)
      // and impact parameter (fm)
      Double_t l = b * k(e_sur_a);
      fCentPot->SetParameter(0, l);
      return fCentPot;
   }
   TF1* GetCentrifugalPotential(Double_t l) const
   {
      // Total (nuclear+coulomb+centrifugal) potential for given angular momentum (hbar)
      fCentPot->SetParameter(0, l);
      return fCentPot;
   }
   TF1* GetBSSCoulombPotential() const
   {
      return fBSS;
   }
   TF1* GetNuclearProximityPotential() const
   {
      return fProx;
   }
   TF1* GetTotalPotential() const
   {
      return fPotential;
   }
   static Double_t TKESymFiss(Int_t Z, Int_t A)
   {
      // Total kinetic energy for symmetric fission
      // V.E. Viola, Jr., Nucl. Data Sect. A1, 391 (1966)
      return (0.1071 * Z * Z / pow(A, THIRD) + 22.3);
   }
   static Double_t NLDSurfaceTensionCoefficient(Int_t Z, Int_t A)
   {
      // Nuclear liquid-drop surface-tension coefficient
      // J. Blocki, J. Randrup, W.J. Swiatecki, and C.F. Tsang, Ann. Phys. (N. Y.) 105, 427 (1977)
      Double_t I = (A - 2.*Z) / (1.*A);
      return 0.9517 * (1. - 1.7826 * I * I);
   }
   static Double_t SWaveFusionBarrierRadius(Int_t zp, Int_t ap, Int_t zt, Int_t at)
   {
      // Fusion barrier radius for s-waves
      Double_t zpzt = zp * zt;
      Double_t D;
      if (zpzt < 500) {
         D = 0.3117 * pow(zpzt, 0.2122);
      } else {
         D = 1.096 + 1.391e-04 * zpzt;
      }
      return InteractionRadius(ap, at) - D;
   }
   static Double_t RLDCriticalAngularMomentum(Int_t z, Int_t a);
   Double_t Eta(Double_t e_sur_a) const
   {
      // Coulomb parameter
      return 0.15746 * ZP * ZT / pow(e_sur_a, 0.5);
   }
   Double_t k(Double_t e_sur_a) const
   {
      // Asymptotic wave number in the centre-of-mass system
      return 0.2187 * AT * AP * pow(e_sur_a, 0.5) / (1.*AC);
   }
   Double_t QuarterPointAngle(Double_t* x, Double_t*) const
   {
      // C.M. quarter point angle as a function of laboratory E/A of projectile
      Double_t krint = k(*x) * RINT;
      Double_t eta = Eta(*x);
      if (krint < 2.*eta) return TMath::Pi();
      return 2.*asin(eta / (krint - eta));
   }
   Double_t ProjectileLabQP(Double_t e) const
   {
      // Quarter-point angle in the laboratory of projectile
      Double_t QP = QuarterPointAngle(&e, 0);
      return atan(sin(QP) / (cos(QP) + AP / (1.*AT)));
   }
   Double_t ProjectileLabEQP(Double_t e) const
   {
      // Laboratory energy of projectile elastically scattered at quarter-point angle
      return AP * e * (AP * AP + 2.*AP * AT * cos(QuarterPointAngle(&e, 0)) + AT * AT) / (1.*AC * AC);
   }

   Double_t Lmax(Double_t* x , Double_t*) const
   {
      return Eta(*x) / tan(QuarterPointAngle(x, 0) / 2.);
   }
   TF1* GetCMQuarterPointAngle() const
   {
      return fCMThetaQuart;
   }
   TF1* GetLmax() const
   {
      return fLmax;
   }
   Double_t SigmaR(Double_t* x, Double_t*) const
   {
      // Reaction X-section from Lmax in mb
      return 10.*(TMath::Pi() / pow(k(*x), 2)) * pow(Lmax(x, 0) + 0.5, 2);
   }
   Double_t ECM(Double_t e_sur_a) const
   {
      // Centre of mass energy from projectile E/A in laboratory
      return MU * e_sur_a;
   }
   TF1* GetReactionCrossSection() const
   {
      return fSigmaR;
   }
   Double_t SigmaFus(Double_t* e_sur_a, Double_t*) const
   {
      // Fusion cross section in mb
      if (*e_sur_a <= 0) return 0.;
      Double_t e = VRB / ECM(*e_sur_a);
      Double_t S1 = (e > 1. ? 0. : 10.*TMath::Pi() * pow(RBARRIER, 2) * (1. - e));
      Double_t S2 = 10.*(TMath::Pi() / pow(k(*e_sur_a), 2)) * pow(LCRIT + 0.5, 2);
      Double_t S = TMath::Min(S1, S2);
      return S;
   }
   TF1* GetFusionCrossSection() const
   {
      return fSigmaFus;
   }
   void DrawAllPotentials(Double_t l = 0) const
   {
      TF1* totpot;
      if (l > 0) {
         totpot = GetCentrifugalPotential(l);
         totpot->SetTitle(Form("HIPOT l=%3.0f", l));
      } else totpot = GetTotalPotential();
      totpot->SetNpx(1000);
      GetBSSCoulombPotential()->SetNpx(1000);
      GetNuclearProximityPotential()->SetNpx(1000);
      totpot->SetLineColor(kBlack);
      totpot->Draw();
      GetBSSCoulombPotential()->SetLineColor(kRed);
      GetBSSCoulombPotential()->Draw("same");
      Double_t max = TMath::Max(GetBSSCoulombPotential()->GetMaximum() * 2., totpot->GetMinimum() * 10);
      GetNuclearProximityPotential()->SetLineColor(kBlue);
      GetNuclearProximityPotential()->Draw("same");
      Double_t min = GetNuclearProximityPotential()->GetMinimum() * 1.2;
      totpot->GetHistogram()->GetYaxis()->SetRangeUser(min, max);
      ((TPad*)gPad)->BuildLegend();
   }

   void Print() const;
   Double_t PotentialPocketRadius(Double_t l);
   Double_t GetMaximumAngularMomentumWithPocket();

   Double_t GetMu() const
   {
      // reduced mass number
      return MU;
   }
   Int_t GetAP() const
   {
      return AP;
   }
   Int_t GetAT() const
   {
      return AT;
   }
   Int_t GetAC() const
   {
      return AC;
   }
   Double_t GetCP() const
   {
      return CP;
   }
   Double_t GetCT() const
   {
      return CT;
   }
   Double_t GetRint() const
   {
      return RINT;
   }

   ClassDef(KVWilckeReactionParameters, 1) //Reaction parameters for heavy-ion collisions (Wilcke et al)
};

#endif
