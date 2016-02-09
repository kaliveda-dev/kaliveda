/***************************************************************************
                          KV2Body.cpp  -  description
                             -------------------
    begin                : 28/11/2003
    copyright            : (C) 2003 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KV2Body.cpp,v 1.4 2009/02/02 13:52:29 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KV2Body.h"
#include "Riostream.h"
#include "TROOT.h"

using namespace std;

ClassImp(KV2Body)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//KV2Body
//
//This class calculates various useful characteristics of binary nuclear reactions.
//It is based on a Fortran programme by Jean-Luc Charvet (cibi.for).
//It allows to calculate:
//      - centre of mass recoil velocity and available energy
//      - laboratory grazing angle of projectile and target
//      - maximum laboratory scattering angles
//      - Q-values
//      - CM and laboratory Rutherford cross-sections for elastic scattering
//      - CM and laboratory angles of ejectiles after elastic or inelastic scattering
//The values are calculated using relativistic kinematics.
//
//
//1. Defining the entrance channel.
//
//Either use the constructor with arguments to specify projectile and target:
//      KV2Body kin( proj, targ );//'proj' and 'targ' are pointers to KVNucleus objects
//or declare them afterwards:
//      KV2Body kin;
//      kin.SetProjectile( proj ); kin.SetTarget( targ );
//One can also specify the projectile & target using their Z and A:
//      kin.SetProjectile( z1, a1 );
//      kin.SetTarget( z2, a2 );
//(In each case a KVNucleus is created which will be deleted with the KV2Body object).
//If you need to further define the properties of these nuclei, you can get a pointer to
//them using GetNucleus(int i):
//      kin.GetNucleus(1)->SetMomentum(...);//set momentum of projectile
//
//2. Defining the exit channel
//
//If no exit channel is defined, it is assumed to be identical to the entrance channel,
//i.e. elastic scattering.
//You can define a different exit channel, either using the constructor:
//      KV2Body kin( proj, targ, QP );// QP is a KVNucleus
//      KV2Body kin( proj, targ, QP, Ediss );// Ediss = energy dissipated in inelastic reaction
//or using the methods:
//      kin.SetOutgoing( QP );
//      kin.SetEDiss( Ediss );  OR  kin.SetExcitEnergy( Ediss );
//
//3. Calculating and obtaining reaction information
//
//Once the entrance channel (and, if necessary, the exit channel) is(are) defined,
//the kinematical calculation is carried out by:
//      kin.CalculateKinematics();
//For a general print-out of the reaction characteristics:
//      kin.Print();
//      kin.Print("lab");//laboratory scattering angles and energies
//      kin.Print("ruth");//Rutherford scattering angles, energies and cross-sections
//
//4. Extracting information on Quasi-Projectile and/or target
//
//Several methods are defined and calculate energy/velocity/angle in CM or lab frame
//for projectile (3) and/or target (4)
//The two arguments are : the angle and the angular range of which nucleus you are interested in
//
//For example : the Double_t GetThetaCMTarget(Double_t ThetaLab,Int_t OfNucleus=3)
//give the polar angle in the CM of reaction of the target
//    depending on the polar angle in the lab of the projectile (OfNucleus=3) or of the target (OfNucleus=4)
//
//
//_____________________________________________________________________________________________________________
void KV2Body::init()
{
   //Default initialisations
   WLT = WCT = 0.;
   for (int i = 0; i < 5; i++) {
      WC[i] = 0.;
      VC[i] = 0.;
      EC[i] = 0.;
      K[i] = 0.;
      TETAMAX[i] = 0.;
      TETAMIN[i] = 0.;
      fThetaLabVsThetaCM[i] = 0;
      fELabVsThetaCM[i] = 0;
      fELabVsThetaLab[i] = 0;
   }
   fEDiss = 0.0;
   fDeleteTarget = kFALSE;
   fDeleteProj = kFALSE;
   fDeleteN4 = kFALSE;
   fKoxReactionXSec = 0;
   fEqbmChargeState = 0;
   fEqbmChargeStateShSol = 0;
   fEqbmChargeStateShGas = 0;
   fSetOutgoing = kFALSE;

   SetIntegralPrecision(1e-10);
}

KV2Body::KV2Body(): fNuclei(4, 1)
{
   //default ctor
   init();
}

KV2Body::KV2Body(const Char_t* systemname) : fNuclei(4, 1)
{
   //Set up calculation defining entrance channel following
   //this prescription  :
   //[Projectile_Symbol]+[Target_Symbol]@[Incident_Energy]MeV/A
   // Example :
   // 129Xe+119Sn@50.0MeV/A
   // U+U@5MeV/A
   // Ta+Zn
   init();
   KVString sener = "";
   Double_t  ee = 0;
   KVString syst(systemname);
   syst.ReplaceAll(" ", "");
   KVString scouple(syst);

   if (syst.Contains("@")) {
      syst.Begin("@");
      scouple = syst.Next();
      sener = syst.Next();
   }
   if (sener != "") {
      sener.ReplaceAll("MeV/A", "");
      ee = sener.Atof();
   }
   scouple.Begin("+");
   fNuclei[1] = new KVNucleus(scouple.Next(), ee);
   fNuclei[2] = new KVNucleus(scouple.Next());
}

KV2Body::KV2Body(KVNucleus* proj, KVNucleus* cib, KVNucleus* proj_out, Double_t Ediss): fNuclei(4, 1)
{
   //Set up calculation defining entrance channel (projectile & target nuclei or
   //single decaying nucleus), exit channel (if necessary, the remaining nucleus of the
   //exit channel is deduced by conservation of mass, charge, momentum) and energy dissipated in reaction.
   //
   //By default the dissipated energy is zero (elastic reaction).
   //If only nucleus 'proj' is defined and Ediss=0, we use the excitation energy of 'proj' as -Ediss

   init();
   fNuclei[1] = proj;
   fNuclei[2] = cib;
   fEDiss = Ediss;
   if (!cib) {
      // break-up kinematics
      if (Ediss == 0.0) {
         fEDiss = -proj->GetExcitEnergy();
      }
   }

   if (proj_out) {
      SetOutgoing(proj_out);
   }
}

void KV2Body::SetTarget(KVNucleus* targ)
{
   //Set target for reaction. The KVNucleus will not be deleted by ~KV2Body.
   fNuclei[2] = targ;
}

void KV2Body::SetTarget(Int_t z, Int_t a)
{
   //Set target for reaction
   //Creates a new KVNucleus which will be deleted by ~KV2Body.
   fNuclei[2] = new KVNucleus(z, a);
   fDeleteTarget = kTRUE;
}

void KV2Body::SetProjectile(KVNucleus* proj)
{
   //Set projectile for reaction. The KVNucleus will not be deleted by ~KV2Body.
   fNuclei[1] = proj;
}

void KV2Body::SetProjectile(Int_t z, Int_t a)
{
   //Set projectile for reaction
   //Creates a new KVNucleus which will be deleted by ~KV2Body.
   fNuclei[1] = new KVNucleus(z, a);
   fDeleteProj = kTRUE;
}

KV2Body::~KV2Body()
{
   //Any nuclei created by the object are deleted
   if (fDeleteN4)
      delete GetNucleus(4);
   if (fDeleteTarget)
      delete GetNucleus(2);
   if (fDeleteProj)
      delete GetNucleus(1);
   fNuclei.Clear();
   if (fKoxReactionXSec) delete fKoxReactionXSec;
   if (fEqbmChargeState) delete fEqbmChargeState;
   for (int i = 0; i < 5; i++) {
      if (fThetaLabVsThetaCM[i]) delete fThetaLabVsThetaCM[i];
      if (fELabVsThetaCM[i]) delete fELabVsThetaCM[i];
      if (fELabVsThetaLab[i]) delete fELabVsThetaLab[i];
   }
}

//_____________________________________________________________________________

Double_t KV2Body::GetVelocity(Double_t mass, Double_t E)
{
   //Static function, calculates relativistic velocity (in cm/ns) from rest mass and total
   //energy (i.e. KE + mass) 'E' for any particle

   Double_t p = TMath::Sqrt(E * E - mass * mass);
   return KVParticle::C() * p / E;
}

//_____________________________________________________________________________

void KV2Body::SetOutgoing(KVNucleus* proj_out)
{
   // Set outgoing projectile-like nucleus' properties.
   // The properties of the outgoing target-like nucleus
   // will be deduced from mass, charge and momentum/energy conservation.
   // The KVNucleus will not be deleted by ~KV2Body.

   fSetOutgoing = kTRUE;
   fNuclei[3] = proj_out;
   Set4thNucleus();
   GetNucleus(3)->SetExcitEnergy(0.);// no mass modification due to E*
}

//_____________________________________________________________________________

void KV2Body::Set4thNucleus()
{
   // Private method, used to deduce 4th nucleus (target-like) from projectile, target
   // and outgoing projectile using conservation of mass, momentum and energy.
   // This nucleus will be deleted with the object.
   //
   // if the exit channel is a the compund nucleus, there is no 4th nucleus defined
   // and the excitation energy of the fusion reaction is set in the fEdiss variable
   // see GetExcitEnergy() methode
   //

   KVNucleus sum = *GetNucleus(1);
   if (GetNucleus(2)) sum += *GetNucleus(2);
   if (GetNucleus(4))
      delete GetNucleus(4);
   KVNucleus* tmp4 = new KVNucleus(sum - *GetNucleus(3));
   fDeleteN4 = kTRUE;
   if (!tmp4->GetZ() && !tmp4->GetA()) {
      SetExcitEnergy(sum.GetExcitEnergy());
      delete tmp4;
      tmp4 = 0;
      fDeleteN4 = kFALSE;
   }
   fNuclei[4] = tmp4;
   if (tmp4) tmp4->SetExcitEnergy(0.); // no mass modification due to E*
}

//_____________________________________________________________________________

KVNucleus* KV2Body::GetNucleus(Int_t i) const
{
   //Return pointer to nucleus i (1 <= i <= 4)
   // Entrance channel nuclei .....  i=1 : projectile  i=2 : target
   // Exit channel nuclei .....  i=3 : projectile-like  i=4 : target-like

   if (i > 0 && i < 5)
      return (KVNucleus*) fNuclei[i];
   Warning("GetNucleus(Int_t i)", "Index i out of bounds, i=%d", i);
   return 0;
}

//_____________________________________________________________________________

Double_t KV2Body::GetQReaction() const
{
   //Calculate Q-value for reaction, including dissipated (excitation) energy

   if (!GetNucleus(3)) {
      Warning("GetQReaction", "Parameters for outgoing nuclei not set");
      return 0.0;
   }
   Double_t QR = GetQGroundStates() - fEDiss;

   return QR;
}

//_______________________________________________________________________________

Double_t KV2Body::GetQGroundStates() const
{
   //Calculate Q-value for reaction, assuming all nuclei in ground state

   if (!GetNucleus(3)) {
      Warning("GetQGroundStates",
              "Parameters for outgoing nuclei not set");
      return 0.0;
   }
   Double_t QGG =
      GetNucleus(1)->GetMassExcess() +
      (GetNucleus(2) ? GetNucleus(2)->GetMassExcess() : 0.)
      - GetNucleus(3)->GetMassExcess() -
      (GetNucleus(4) ? GetNucleus(4)->GetMassExcess() : 0.);
   return QGG;
}

//________________________________________________________________________________

Double_t KV2Body::GetCMEnergy() const
{
   //Return available kinetic energy in centre of mass

   return WCT - (GetNucleus(1)->GetMass() +
                 (GetNucleus(2) ? GetNucleus(2)->GetMass() : 0.));
}

//_____________________________________________________________________________
Double_t KV2Body::GetMaxAngleLab(Int_t i) const
{
   //Returns maximum scattering angle in lab for nuclei i=3 (quasiproj)
   //and i=4 (quasitarget)
   if (i < 3 || i > 4) {
      Warning("GetMaxAngleLab(Int_t i)",
              "Returns maximum scattering angle in lab for nuclei i=3 (quasiproj) and i=4 (quasitarget)");
      return 0.;
   }
   return TETAMAX[i];
}

//_____________________________________________________________________________
Double_t KV2Body::GetMinAngleLab(Int_t i) const
{
   //Returns minimum scattering angle in lab for nuclei i=3 (quasiproj)
   //and i=4 (quasitarget)
   if (i < 3 || i > 4) {
      Warning("GetMinAngleLab(Int_t i)",
              "Returns minimum scattering angle in lab for nuclei i=3 (quasiproj) and i=4 (quasitarget)");
      return 0.;
   }
   return TETAMIN[i];
}

//_____________________________________________________________________________

TVector3 KV2Body::GetCMVelocity() const
{
   //Return vector velocity of centre of mass of reaction (units: cm/ns)

   return VCM;
}

//_____________________________________________________________________________

Double_t KV2Body::GetCMVelocity(Int_t i) const
{
   //Return velocity of nucleus "i" in the centre of mass of the reaction
   // Entrance channel nuclei .....  i=1 : projectile  i=2 : target
   // Exit channel nuclei .....  i=3 : projectile-like  i=4 : target-like
   return VC[i];
}

//_____________________________________________________________________________

Double_t KV2Body::GetLabGrazingAngle(Int_t i) const
{
   //Calculate laboratory grazing angle.
   // i = 1 (default) : projectile
   // i = 2           : target

   if (i < 1 || i > 2) {
      Warning("GetLabGrazingAngle(Int_t i)",
              "i should be 1 (proj) or 2 (targ)");
      return 0.0;
   }
   if (!GetNucleus(2)) {
      Warning("GetLabGrazingAngle(Int_t i)",
              "No target defined for reaction");
      return 0.0;
   }
   Double_t RP = 1.28 * TMath::Power(GetNucleus(1)->GetA(), 1. / 3.)
                 - 0.76 + 0.8 * TMath::Power(GetNucleus(1)->GetA(), -1. / 3.);
   Double_t RT = 1.28 * TMath::Power(GetNucleus(2)->GetA(), 1. / 3.)
                 - 0.76 + 0.8 * TMath::Power(GetNucleus(2)->GetA(), -1. / 3.);
   Double_t CP = RP * (1. - TMath::Power(1. / RP, 2.));
   Double_t CT = RT * (1. - TMath::Power(1. / RT, 2.));
   Double_t RINT = CP + CT + 4.49 - (CT + CP) / 6.35;
   Double_t BAR =
      1.44 * GetNucleus(1)->GetZ() * GetNucleus(2)->GetZ() / RINT;
   if (GetCMEnergy() < BAR) {
      //below Coulomb barrier
      switch (i) {
         case 1:
            return 180.;
            break;
         case 2:
            return 90.;
            break;
      }
   }
   Double_t X = 2. * RINT * GetCMEnergy();
   Double_t Y =
      X / (GetNucleus(1)->GetZ() * GetNucleus(2)->GetZ() * 1.44) - 1.;
   Double_t U = 1. / Y;
   Double_t GR = 2. * TMath::ASin(U);
   Double_t GPART = TMath::Pi() - GR;
   Double_t ARAZ =
      TMath::Sin(GR) / (TMath::Cos(GR) +
                        GetNucleus(1)->GetA() / (1.0 *
                              GetNucleus(2)->GetA()));
   Double_t GRAZ = TMath::ATan(ARAZ);
   if (GRAZ <= 0.)
      GRAZ += TMath::Pi();

   switch (i) {
      case 1:                     //projectile
         return (GRAZ * TMath::RadToDeg());
         break;
      case 2:                     //target
         return (GPART * TMath::RadToDeg() / 2.);
         break;
   }
   return -99.;
}

//_____________________________________________________________________________

void KV2Body::CalculateKinematics()
{
   // Called to make kinematics calculation for all nuclei, use once properties of
   // entrance-channel (and, if necessary, exit-channel) nuclei have been defined.
   // If no exit-channel nuclei are defined, we assume elastic scattering (i.e. identical
   // outgoing nuclei)

   KVNucleus* Nuc1 = GetNucleus(1);
   KVNucleus* Nuc2 = GetNucleus(2);

   // make sure E* of nuclei is zero
   // this is because the E*/Ediss is handled separately here
   Nuc1->SetExcitEnergy(0.);
   if (Nuc2) Nuc2->SetExcitEnergy(0.);

   // call SetOutgoing if not already done
   if (!fSetOutgoing) SetOutgoing(Nuc1);
   //fSetOutgoing = kFALSE;

   // set everything to zero
   WLT = WCT = BCM = 0.;
   for (int i = 0; i < 5; i++) {
      WC[i] = 0.;
      VC[i] = 0.;
      EC[i] = 0.;
      K[i] = 0.;
      TETAMAX[i] = 0.;
      TETAMIN[i] = 0.;
   }
   VCM.SetXYZ(0., 0., 0.);

   //total energy (T + m) of entrance channel
   WLT = Nuc1->E();
   //velocity of CM
   VCM = Nuc1->GetMomentum();
   if (Nuc2) {
      WLT += Nuc2->E();
      VCM += Nuc2->GetMomentum();
   }

   //beta of CM
   BCM = VCM.Mag() / WLT;

   VCM *= (1. / WLT) * KVParticle::C();
   Nuc1->SetFrame("CM", VCM, kFALSE);
   if (Nuc2)
      Nuc2->SetFrame("CM", VCM, kFALSE);

   //total energy in CM
   WCT = (GetCMGamma() > 0.0 ? WLT / GetCMGamma() : 0.);
   if (WCT == 0.0)
      return;

   //total energy proj in CM
   WC[1] = Nuc1->GetFrame("CM")->E();
   //kinetic energy proj in CM
   EC[1] = Nuc1->GetFrame("CM")->GetKE();
   VC[1] = Nuc1->GetFrame("CM")->GetVelocity().Mag();
   K[1] = VCM.Mag() / VC[1];
   if (Nuc2) {
      WC[2] = Nuc2->GetFrame("CM")->E();
      EC[2] = Nuc2->GetFrame("CM")->GetKE();
      VC[2] = Nuc2->GetFrame("CM")->GetVelocity().Mag();
      K[2] = VCM.Mag() / VC[2];
   }

   Double_t AM3 = GetNucleus(3)->GetMass();
   Double_t AM4 = (GetNucleus(4) ? GetNucleus(4)->GetMass() : 0.0);
   //total cm energy of nucleus 3 (quasiproj)
   WC[3] = (WCT - GetEDiss()) / 2. + (AM3 - AM4) * (AM3 + AM4)
           / (2. * (WCT - GetEDiss()));
   VC[3] = GetVelocity(AM3, WC[3]);
   //cm kinetic energy
   EC[3] = WC[3] - AM3;
   if (VC[3] > 0.) K[3] = VCM.Mag() / VC[3];

   Double_t T3MAX = 0.;
   if (TMath::AreEqualAbs(K[3], 1., 1.e-16))
      T3MAX = TMath::PiOver2();
   else if (K[3] < 1.)
      T3MAX = TMath::Pi();
   else
      T3MAX = TMath::ATan((1. / TMath::Sqrt(K[3] * K[3] - 1.)) / GetCMGamma());
   TETAMAX[3] = T3MAX * TMath::RadToDeg();

   if (!GetNucleus(4))
      return;                   //only valid for binary channels
   //total cm energy of nucleus 4 (quasitarg)
   WC[4] = WCT - GetEDiss() - WC[3];
   VC[4] = GetVelocity(AM4, WC[4]);
   //cm kinetic energy
   EC[4] = WC[4] - AM4;
   if (VC[4] > 0.) K[4] = VCM.Mag() / VC[4];

   Double_t T4MAX = 0.;
   if (TMath::AreEqualAbs(K[4], 1., 1.e-16))
      T4MAX = TMath::PiOver2();
   else if (K[4] < 1.)
      T4MAX = TMath::Pi();
   else
      T4MAX = TMath::ATan((1. / TMath::Sqrt(K[4] * K[4] - 1.)) / GetCMGamma());
   if (TMath::Abs(GetQReaction()) < 1.E-08 && K[4] < 1.)
      T4MAX = TMath::PiOver2();
   TETAMAX[4] = T4MAX * TMath::RadToDeg();
}

//_____________________________________________________________________________

Double_t KV2Body::GetCMEnergy(Int_t i) const
{
   //Returns kinetic energy of nucleus "i" in the centre of mass of the reaction
   // Entrance channel nuclei .....  i=1 : projectile  i=2 : target
   // Exit channel nuclei .....  i=3 : projectile-like  i=4 : target-like
   return EC[i];
}

//_____________________________________________________________________________

void KV2Body::Print(Option_t* opt) const
{
   // Print out characteristics of reaction.
   //
   // If a two-body exit channel has been defined, you can use the following options:
   //    opt = "ruth" :  list Rutherford scattering cross-sections as a function of angle in laboratory frame
   //    opt = "lab"  :  list energies and angles in laboratory frame

   cout << " ***** REACTION    " << GetNucleus(1)->GetSymbol();
   if (GetNucleus(2))
      cout << "  +  " << GetNucleus(2)->GetSymbol();
   if (GetNucleus(3)) {
      cout << " --->   " << GetNucleus(3)->GetSymbol();
   }
   if (GetNucleus(4))
      cout << "  +  " << GetNucleus(4)->GetSymbol();
   cout << "    ******" << endl;

   cout << "  E.LAB = " << GetNucleus(1)->GetEnergy() << " MEV";
   if (GetNucleus(3)) {
      cout << "     QGG = " << GetQGroundStates() << " MEV";
   }
   cout << endl;
   cout << "  E.EXC = " << GetExcitEnergy() << " MEV";
   if (GetNucleus(3)) {
      cout << " ==> Q-REACTION = " << GetQReaction() << " MEV";
   }
   cout << endl;
   cout << " AVAILABLE ENERGY IN C.M. : ECM = " << GetCMEnergy() <<
        " MEV  (" << GetCMEnergy() / (GetNucleus(1)->GetA() +
                                      (GetNucleus(2) ? GetNucleus(2)->
                                       GetA() : 0.)) << " MEV/A)" << endl;
   cout << " PROJECTILE VELOCITY IN LAB " << GetNucleus(1)->GetV().Mag()
        << " CM/NS  ( " << GetNucleus(1)->Beta() << " * C )" << endl;
   cout << " VELOCITY OF C.M.           " << GetCMVelocity().
        Mag() << " CM/NS" << endl;

   for (int i = 1; i <= 4; i++) {
      if (GetNucleus(i))
         cout << " ENERGY - VELOCITY OF NUCLEUS " << i << " IN CM : " <<
              GetCMEnergy(i) << " MEV  " << GetCMVelocity(i) << " CM/NS   (K=" << K[i] << ")" <<
              endl;
   }
   if (GetNucleus(3)) {
      cout << " MAXIMUM SCATTERING ANGLE IN LABORATORY" << endl;
      cout << "        THETA #3#   " << GetMaxAngleLab(3) << " DEG." <<
           endl;
      if (GetNucleus(4))
         cout << "        THETA #4#   " << GetMaxAngleLab(4) << " DEG." <<
              endl;
   }
   if (GetNucleus(2)) {
      cout << " GRAZING ANGLE IN LABORATORY : PROJECTILE " <<
           GetLabGrazingAngle(1) << " DEG." << endl;
      cout << " GRAZING ANGLE IN LABORATORY : TARGET     " <<
           GetLabGrazingAngle(2) << " DEG." << endl;
   }

   if (GetNucleus(3)) {
      Int_t nsteps = 15;

      if (!strcmp(opt, "lab") || !strcmp(opt, "LAB")
            || !strcmp(opt, "Lab")) {
         //laboratory angular distribution
         cout << endl <<
              "                   LABORATORY ANGULAR DISTRIBUTION" << endl
              << endl;
         cout <<
              "   TETA 3        EN.DIF.3               V3              TETA 4            EN.DIF.4            TETA 3"
              << endl;
         cout <<
              "   (LAB)           (LAB)               (LAB)            (LAB)              (LAB)              (C.M)"
              << endl;
         Double_t dtheta = GetMaxAngleLab(3) / nsteps;
         for (int step = 0; step <= nsteps; step++) {
            Double_t theta = dtheta * step;

            Double_t elabP1, elabP2;
            Double_t tcmP1, tcmP2;
            Double_t vP1, vP2;
            Double_t tlT1, tlT2;
            Double_t elabT1, elabT2;
            GetELab(3, theta, 3, elabP1, elabP2);
            GetThetaCM(theta, 3, tcmP1, tcmP2);
            GetVLab(3, theta, 3, vP1, vP2);
            GetThetaLab(4, theta, 3, tlT1, tlT2);
            GetELab(4, theta, 3, elabT1, elabT2);
            printf
            ("   %6.2f     %7.2f/%7.2f      %5.2f/%5.2f      %6.2f/%6.2f      %7.2f/%7.2f     %6.2f/%6.2f\n",
             theta, elabP1, elabP2, vP1, vP2,
             tlT1, tlT2, elabT1, elabT2,
             tcmP1, tcmP2);
         }
      }
      if (GetNucleus(2) && (!strcmp(opt, "ruth") || !strcmp(opt, "RUTH")
                            || !strcmp(opt, "Ruth"))) {
         //laboratory angular distribution with Rutherford x-section
         cout << endl <<
              "             RUTHERFORD LABORATORY ANGULAR DISTRIBUTION" <<
              endl << endl;
         cout <<
              "   TETA 3     TETA 3    EN.DIF.3        SIG.RUT.        SIG.RUT."
              << endl;
         cout <<
              "   (LAB)       (CM)       (LAB)       (LAB) (b/sr)    (CM)  (b/sr)"
              << endl;
         //go up to grazing angle
         Double_t dtheta = GetLabGrazingAngle(1) / nsteps;
         for (int step = 0; step < nsteps; step++) {
            Double_t theta = dtheta * (step + 1);
            Double_t elabP1, elabP2;
            Double_t tcm1, tcm2;
            GetELab(3, theta, 3, elabP1, elabP2);
            GetThetaCM(theta, 3, tcm1, tcm2);
            printf
            ("   %6.2f     %6.2f     %7.2f      %10.4g      %10.4g\n",
             theta, tcm1, elabP1,
             GetXSecRuthLab(theta), GetXSecRuthCM(theta));
         }
      }
   }
}

//______________________________________________________________________________________________
Int_t KV2Body::GetELab(Int_t OfNucleus, Double_t ThetaLab, Int_t AngleNucleus, Double_t& e1, Double_t& e2) const
{
   // Calculate laboratory kinetic energy of nucleus OfNucleus as a function of the lab angle of
   // nucleus AngleNucleus.
   //
   // In general there may be two solutions for a given angle, therefore we return the number
   // of solutions (0 if ThetaLab > max lab angle for nucleus in question).

   e1 = e2 = -1.;
   if (ThetaLab > TETAMAX[AngleNucleus]) return 0;
   // calculate CM angle(s)
   Double_t TCM1, TCM2;
   Int_t nsol = GetThetaCM(ThetaLab, AngleNucleus, TCM1, TCM2);
   TCM1 = GetThetaCM(OfNucleus, TCM1, AngleNucleus);
   if (nsol > 1) TCM2 = GetThetaCM(OfNucleus, TCM2, AngleNucleus);
   // calculate lab energie(s) of corresponding to CM angle(s)
   e1 = GetELab(TCM1, OfNucleus);
   if (nsol == 2) e2 = GetELab(TCM2, OfNucleus);
   return nsol;
}

Int_t KV2Body::GetThetaCM(Double_t ThetaLab, Int_t OfNucleus, Double_t& t1, Double_t& t2) const
{
   // Calculate CM angle of nucleus OfNucleus as a function of its lab angle.
   // Returns number of solutions (there may be <=2 solutions).

   t1 = t2 = -1.;
   KV2Body* kin = const_cast<KV2Body*>(this);
   TF1* TLF = kin->GetThetaLabVsThetaCMFunc(OfNucleus);
   Int_t nsol = FindRoots(TLF, 0., 180., ThetaLab, t1, t2);
   return nsol;
}

//______________________________________________________________________________________________
Double_t KV2Body::ELabVsThetaLab(Double_t* x, Double_t* par)
{
   // Function calculating lab energy of nucleus par[0] for any lab angle x[0]

   Double_t e1, e2;
   Int_t nsol = GetELab((Int_t)par[0], x[0], (Int_t)par[0], e1, e2);
   if (!nsol) return 0;
   //if(nsol>1) Warning("ELabVsThetaLab", "Two energies are possible for %f deg. : %f and %f",
   //                   x[0],e1,e2);
   return e1;
}

//______________________________________________________________________________________________
Double_t KV2Body::ELabVsThetaCM(Double_t* x, Double_t* par)
{
   // Function calculating lab energy of nucleus par[0] for any CM angle x[0]

   Int_t OfNucleus = (Int_t)par[0];
   Double_t TCM = x[0] * TMath::DegToRad();
   Double_t WL =
      WC[OfNucleus] * GetCMGamma() * (1. +
                                      BCM * (VC[OfNucleus] / KVParticle::C()) *
                                      TMath::Cos(TCM));
   return (WL - GetNucleus(OfNucleus)->GetMass());
}

//______________________________________________________________________________________________
Double_t KV2Body::ThetaLabVsThetaCM(Double_t* x, Double_t* par)
{
   // Calculate Lab angle of nucleus as function of CM angle x[0]
   // par[0] = index of nucleus = 1, 2, 3, 4

   Double_t ThetaCM = x[0] * TMath::DegToRad();
   Int_t OfNucleus = (Int_t)par[0];
   Double_t TanThetaL = TMath::Sin(ThetaCM) / (K[OfNucleus] + TMath::Cos(ThetaCM)) / GetCMGamma();
   Double_t ThetaL = TMath::ATan(TanThetaL) * TMath::RadToDeg();
   if (ThetaL < 0.) ThetaL += 180.;
   return ThetaL;
}

//______________________________________________________________________________________________
TF1* KV2Body::GetELabVsThetaCMFunc(Int_t OfNucleus)
{
   // Return TF1 giving lab energy of nucleus as function of CM angle
   // OfNucleus = 1 or 2 (entrance channel) or 3 or 4 (exit channel)

   if (!fELabVsThetaCM[OfNucleus]) {
      fELabVsThetaCM[OfNucleus] = new TF1(Form("KV2Body:ELabVsThetaCM:%d", OfNucleus),
                                          this, &KV2Body::ELabVsThetaCM, 0, 180, 1, "KV2Body", "ELabVsThetaCM");
      fELabVsThetaCM[OfNucleus]->SetNpx(1000);
      fELabVsThetaCM[OfNucleus]->SetParameter(0, OfNucleus);
   }
   return fELabVsThetaCM[OfNucleus];
}

//______________________________________________________________________________________________
TF1* KV2Body::GetELabVsThetaLabFunc(Int_t OfNucleus)
{
   // Return TF1 giving lab energy of nucleus as function of its lab angle
   // OfNucleus = 1 or 2 (entrance channel) or 3 or 4 (exit channel)

   if (!fELabVsThetaLab[OfNucleus]) {
      fELabVsThetaLab[OfNucleus] = new TF1(Form("KV2Body:ELabVsThetaLab:%d", OfNucleus),
                                           this, &KV2Body::ELabVsThetaLab, 0, 180, 1, "KV2Body", "ELabVsThetaLab");
      fELabVsThetaLab[OfNucleus]->SetNpx(1000);
      fELabVsThetaLab[OfNucleus]->SetParameter(0, OfNucleus);
   }
   return fELabVsThetaLab[OfNucleus];
}

//______________________________________________________________________________________________

Int_t KV2Body::GetVLab(Int_t OfNucleus, Double_t ThetaLab, Int_t AngleNucleus, Double_t& v1, Double_t& v2) const
{
   // Calculate laboratory velocity of nucleus OfNucleus as a function of the lab angle of
   // nucleus AngleNucleus.
   //
   // In general there may be two solutions for a given angle, therefore we return the number
   // of solutions (0 if ThetaLab > max lab angle for nucleus in question).

   Double_t e1, e2;
   v1 = v2 = 0.;
   Int_t nsol = GetELab(OfNucleus, ThetaLab, AngleNucleus, e1, e2);
   if (!nsol) return nsol;
   Double_t etot1 = e1 + GetNucleus(OfNucleus)->GetMass();
   Double_t etot2 = e2 + GetNucleus(OfNucleus)->GetMass();
   v1 = GetVelocity(GetNucleus(OfNucleus)->GetMass(), etot1);
   if (nsol > 1) v2 = GetVelocity(GetNucleus(OfNucleus)->GetMass(), etot2);
   return nsol;
}

//______________________________________________________________________________________________

Int_t KV2Body::GetThetaLab(Int_t OfNucleus, Double_t ThetaLab, Int_t AngleNucleus, Double_t& t1, Double_t& t2) const
{
   // Calculate laboratory angle of nucleus OfNucleus as a function of the laboratory angle of
   // nucleus AngleNucleus.
   //
   // In general there may be two solutions for a given angle, therefore we return the number
   // of solutions (0 if ThetaLab > max lab angle for nucleus in question).

   t1 = t2 = -1.;
   if (ThetaLab > TETAMAX[AngleNucleus]) return 0;
   if (!(TMath::Abs(OfNucleus - AngleNucleus) % 2)) {
      // same nucleus!
      t1 = ThetaLab;
      return 1;
   }
   // calculate CM angle(s)
   Double_t TCM1, TCM2;
   Int_t nsol = GetThetaCM(ThetaLab, AngleNucleus, TCM1, TCM2);
   TCM1 = GetThetaCM(OfNucleus, TCM1, AngleNucleus);
   if (nsol > 1) TCM2 = GetThetaCM(OfNucleus, TCM2, AngleNucleus);
   // calculate lab angle(s) of corresponding CM angle(s)
   t1 = GetThetaLab(TCM1, OfNucleus);
   if (nsol == 2) t2 = GetThetaLab(TCM2, OfNucleus);
   return nsol;
}

//______________________________________________________________________________________________
TF1* KV2Body::GetThetaLabVsThetaCMFunc(Int_t OfNucleus)
{
   // Return TF1 giving lab angle of nucleus as function of CM angle
   // OfNucleus = 1 or 2 (entrance channel) or 3 or 4 (exit channel)

   if (!fThetaLabVsThetaCM[OfNucleus]) {
      fThetaLabVsThetaCM[OfNucleus] = new TF1(Form("KV2Body:ThetaLabVsThetaCM:%d", OfNucleus),
                                              this, &KV2Body::ThetaLabVsThetaCM, 0, 180, 1, "KV2Body", "ThetaLabVsThetaCM");
      fThetaLabVsThetaCM[OfNucleus]->SetNpx(1000);
      fThetaLabVsThetaCM[OfNucleus]->SetParameter(0, OfNucleus);
   }
   return fThetaLabVsThetaCM[OfNucleus];
}

//______________________________________________________________________________________________
Double_t KV2Body::GetXSecRuthCM(Double_t ThetaLab, Int_t OfNucleus) const
{
   //Calculate Rutherford cross-section (b/sr) in the CM as a
   //function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle
   //
   // WARNING: in inverse kinematics, projectile lab angles generally have
   // two corresponding CM angles. We only use the most forward (smallest) CM angle.

   Double_t par = OfNucleus;
   return const_cast<KV2Body*>(this)->XSecRuthCM(&ThetaLab, &par);
}

//______________________________________________________________________________________________
Double_t KV2Body::XSecRuthCM(Double_t* x, Double_t* par)
{
   //Calculate Rutherford cross-section (b/sr) in the CM as a
   //function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle
   //
   // WARNING: in inverse kinematics, projectile lab angles generally have
   // two corresponding CM angles. We only use the most forward (smallest) CM angle.

   if (!GetNucleus(2)) {
      Warning("GetXSecRuthCM", "No target defined for reaction");
      return 0.;
   }
   Double_t PB =
      1.44 * GetNucleus(1)->GetZ() * GetNucleus(2)->GetZ() /
      GetCMEnergy();
   // get projectile CM angle from lab angle of nucleus par[0]
   Double_t TCM = GetMinThetaCMFromThetaLab(1, x[0], par[0]);
   Double_t D = 1. / (16. * (TMath::Power(TMath::Sin(TCM * TMath::DegToRad() / 2.), 4.)));

   return ((TMath::Power(PB, 2.)) * D / 100.);
}

Double_t KV2Body::GetMinThetaCMFromThetaLab(Int_t OfNucleus, Double_t theta, Int_t OtherNucleus) const
{
   // Return the smallest (i.e. most forward) CM angle of nucleus OfNucleus
   // corresponding to laboratory angle theta of nucleus OtherNucleus

   Double_t t1, t2;
   Int_t nsol = GetThetaCM(theta, OtherNucleus, t1, t2);
   if (!nsol) return -1.;
   Double_t Pt1 = GetThetaCM(OfNucleus, t1, OtherNucleus);
   Double_t Pt2 = GetThetaCM(OfNucleus, t2, OtherNucleus);
   Double_t Pt;
   if (nsol > 1)
      Pt = TMath::Min(Pt1, Pt2);
   else
      Pt = Pt1;
   return Pt;
}

//______________________________________________________________________________________________
Double_t KV2Body::XSecRuthCMVsThetaCM(Double_t* x, Double_t* par)
{
   // Calculate CM Rutherford cross-section (b/sr) in the CM as a function of
   // scattering angle in the CM frame for nucleus par[0]

   if (!GetNucleus(2)) {
      Warning("XSecRuthCMVsThetaCM", "No target defined for reaction");
      return 0.;
   }
   Double_t PB =
      1.44 * GetNucleus(1)->GetZ() * GetNucleus(2)->GetZ() /
      GetCMEnergy();
   Double_t TCM = x[0] * TMath::DegToRad();
   Int_t OfNucleus = (Int_t)par[0];
   if (OfNucleus == 2 || OfNucleus == 4) TCM = TMath::Pi() - TCM; // get projectile CM angle from target CM angle
   Double_t D = 1. / (16. * (TMath::Power(TMath::Sin(TCM / 2.), 4.)));
   return ((TMath::Power(PB, 2.)) * D / 100.);
}

//______________________________________________________________________________________________
Double_t KV2Body::GetXSecRuthLab(Double_t ThetaLab, Int_t OfNucleus) const
{
   //Calculate Rutherford cross-section (b/sr) in the Lab as a
   //function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle
   //
   // WARNING: in inverse kinematics, projectile lab angles generally have
   // two corresponding CM angles. We only use the most forward (smallest) CM angle.

   Double_t par = OfNucleus;
   return const_cast<KV2Body*>(this)->XSecRuthLab(&ThetaLab, &par);
}

//______________________________________________________________________________________________
Double_t KV2Body::XSecRuthLab(Double_t* x, Double_t* par)
{
   //Calculate Rutherford cross-section (b/sr) in the Lab as a
   //function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle
   //
   // WARNING: in inverse kinematics, projectile lab angles generally have
   // two corresponding CM angles. We only use the most forward (smallest) CM angle.

   Double_t DSIDTB = XSecRuthCM(x, par);
   // get projectile CM angle from lab angle of nucleus par[0]
   Double_t T3CM = GetMinThetaCMFromThetaLab(1, x[0], par[0]);
   Double_t T3L = GetThetaLab(T3CM, 1) * TMath::DegToRad();
   T3CM *= TMath::DegToRad();
   Double_t RLC = (TMath::Power(TMath::Sin(T3CM), 3.)) /
                  ((TMath::Power(TMath::Sin(T3L), 3.)) * GetCMGamma() *
                   (1. + K[3] * TMath::Cos(T3CM)));
   if (DSIDTB * RLC < 0) {
      Warning("GetXSecRuthLab", "negative value for choosen parameters : %lf %d\n", x[0], (Int_t)par[0]);
      return 0;
   }
   return (DSIDTB * RLC);
}

//______________________________________________________________________________________________
Double_t KV2Body::XSecRuthLabInt(Double_t* x, Double_t* par)
{
   //Rutherford cross-section (b/sr) function in the Lab as a
   //function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle x[0]
   //including 'sin theta' factor needed for integrating over solid angles.

   return XSecRuthLab(x, par) * TMath::Sin(x[0] * TMath::DegToRad());
}

//______________________________________________________________________________________________
//Double_t KV2Body::GetIntegratedXSecRuthLab(KVTelescope* tel, Int_t OfNucleus)
//{
//   //Calculate Integrated Rutherford cross-section (barns) in the Lab using
//   //polar and azimuthal angular range of the given KVTelescope.
//   // if (OfNucleus==3) => X-section for scattered projectile
//   // if (OfNucleus==4) => X-section for scattered target
//   //
//   //The returned value is in barns
//
//   return GetIntegratedXSecRuthLab(tel->GetThetaMin(), tel->GetThetaMax(), tel->GetPhiMin(), tel->GetPhiMax(), OfNucleus);
//}
//
////______________________________________________________________________________________________
//Double_t KV2Body::GetIntegratedXSecRuthLab(KVDetector* det, Int_t OfNucleus)
//{
//   //Calculate Integrated Rutherford cross-section (barns) in the Lab using
//   //polar and azimuthal angular range of the given KVDetector. These will be taken
//   //from the parent KVTelescope of the detector.
//   // if (OfNucleus==3) => X-section for scattered projectile
//   // if (OfNucleus==4) => X-section for scattered target
//   //
//   //The returned value is in barns
//
//   KVTelescope* tel = (KVTelescope*)det->GetParentStructure("TELESCOPE");
//   if (!det) {
//      Error("GetIntegratedXSecRuthLab(KVDetector*,Int_t)",
//            "Detector has no parent telescope: it has not been positioned in a multidetector geometry");
//      return 0;
//   }
//   return GetIntegratedXSecRuthLab(tel, OfNucleus);
//}

//______________________________________________________________________________________________
Double_t KV2Body::GetIntegratedXSecRuthLab(Float_t th1, Float_t th2, Float_t phi1, Float_t phi2, Int_t OfNucleus)
{
   //Calculate Integrated Rutherford cross-section (barns) in the Lab using
   //polar and azimuthal angular range expressed in degree
   // if (OfNucleus==3) This angular range is considered to be the scattered projectile one
   // if (OfNucleus==4) This angular range is considered to be the scattered target one
   //
   //If phi1 ou phi2 ==-1 the azimuthal width is set to 2pi
   //Else if phi1=phi2 the azimuthal width is set to 1 ie the integral is only on theta
   //
   //The returned value is in barns

   if (th2 < th1) return 0;
   Double_t dphi = 0;
   //azimuthal width expressed in rad
   if (phi1 == -1 || phi2 == -1) dphi = 2 * TMath::Pi();
   else if (phi1 == phi2) dphi = 1;
   else {
      dphi = phi2 - phi1;
      dphi *= TMath::DegToRad();
   }
   Double_t theta_min = 1.;
   Double_t theta_max = 179.;
   if (th1 < theta_min) theta_min = th1;
   if (th2 > theta_max) theta_max = th2;

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   return GetXSecRuthLabIntegralFunc(OfNucleus, theta_min, theta_max)->Integral(th1, th2, fIntPrec) * TMath::DegToRad() * dphi;
#else
   const Double_t* para = 0;
   return GetXSecRuthLabIntegralFunc(OfNucleus, theta_min, theta_max)->Integral(th1, th2, para, fIntPrec) * TMath::DegToRad() * dphi;
#endif
}

//__________________________________________________________________________________________________

Double_t KV2Body::BassIntBarrier()
{
   // calculate Bass interaction barrier B_int
   // r0 = 1.07 fm

   const Double_t r0 = 1.07;
   const Double_t e2 = 1.44;
   Double_t A1third = pow(GetNucleus(1)->GetA(), 1. / 3.);
   Double_t A2third = pow(GetNucleus(2)->GetA(), 1. / 3.);
   Double_t R12 = r0 * (A1third + A2third);

   Double_t Bint = GetNucleus(1)->GetZ() * GetNucleus(2)->GetZ() * e2 / (R12 + 2.7)
                   - 2.9 * A1third * A2third / (A1third + A2third);

   return Bint;
}

//__________________________________________________________________________________________________

Double_t KV2Body::KoxReactionXSec(Double_t* eproj, Double_t*)
{
   // calculate Kox reaction X-section (in barns) for a given lab energy of projectile (in MeV/nucleon)
   //
   // r0 = 1.05 fm
   // c = 0.6 @ 30MeV/A, 1.4 @ 83 MeV/A => linear interpolation:
   //   c = 0.8/53*eproj + 0.6 - 30*0.8/53
   // uses Bass interaction barrier Bint in the (1 - B/Ecm) term

   const Double_t r0 = 1.05;
   const Double_t a = 1.9;
   const Double_t c0 = 0.8 / 53.;
   const Double_t c1 = 0.6 - 30 * c0;

   KVNucleus* proj = (KVNucleus*)fNuclei[1];
   proj->SetEnergy(eproj[0]*proj->GetA());
   CalculateKinematics();
   Double_t ECM = GetCMEnergy();

   //printf("C.M. energy = %f\n", ECM);
   Double_t Bint = BassIntBarrier();
   //printf("Barrier = %f\n", Bint);
   Double_t EFac = 1 - Bint / ECM;
   //printf("1 - Bint/Ecm = %f\n", EFac);
   Double_t c = TMath::Max(0., c0 * eproj[0] + c1);
   //printf("Kox c-factor = %f\n", c);
   Double_t A1third = pow(proj->GetA(), 1. / 3.);
   Double_t A2third = pow(((KVNucleus*)fNuclei[2])->GetA(), 1. / 3.);

   Double_t Xsec = TMath::Pi() * pow(r0, 2) *
                   pow((A1third + A2third + a * A1third * A2third / (A1third + A2third) - c), 2) *
                   EFac;

   return Xsec / 100.;
}

//__________________________________________________________________________________________________

Double_t KV2Body::GetSphereDureReactionXSec(Double_t r0)
{
   // calculate Reaction Cross Section with the "Sphere Dure"
   // approximation

   Double_t A1third = pow(((KVNucleus*)fNuclei[1])->GetA(), 1. / 3.);
   Double_t A2third = pow(((KVNucleus*)fNuclei[2])->GetA(), 1. / 3.);

   Double_t Xsec = TMath::Pi() * pow(r0, 2) *
                   pow(A1third + A2third , 2);

   return Xsec / 100.;
}

//__________________________________________________________________________________________________

Double_t KV2Body::GetBmaxFromReactionXSec(Double_t ReacXsec)
{

   // Deduce the maximum impact parameter (in fm) from a given reaction cross section (in barn)
   // in the approximation Xsec = \int(0,bmax) 2Pi b db

   return 10 * TMath::Sqrt(ReacXsec / TMath::Pi());

}



//__________________________________________________________________________________________________

Double_t KV2Body::GetIntegratedXsec(Double_t b1, Double_t b2)
{

   // Integrate the cross section between two impact parameter (in fm)
   // and give the result in barn

   return TMath::Pi() * (TMath::Power(b2, 2.) - TMath::Power(b1, 2.)) / 100;

}

//__________________________________________________________________________________________________

TF1* KV2Body::GetKoxReactionXSecFunc()
{
   // Return pointer to TF1 with Kox reaction X-section in barns as a
   // function of projectile lab energy (in Mev/nucleon) for this reaction.
   // By default the range of the function is [20,100] MeV/nucleon.
   // Change with TF1::SetRange.

   if (!fKoxReactionXSec) {
      TString name = GetNucleus(1)->GetSymbol();
      name += " + ";
      name += GetNucleus(2)->GetSymbol();
      fKoxReactionXSec = new TF1(name.Data(),
                                 this, &KV2Body::KoxReactionXSec, 20, 100, 0, "KV2Body", "KoxReactionXSec");
      fKoxReactionXSec->SetNpx(1000);
   }
   return fKoxReactionXSec;
}

//__________________________________________________________________________________________________

Double_t KV2Body::EqbmChargeState(Double_t* t, Double_t*)
{
   // Calculate the mean charge state of the projectile after passage through the target,
   // assuming that the equilibrium charge state distribution is achieved*.
   // t[0] = energy of projectile after the target (in MeV/nucleon)
   //
   // We use the empirical parameterization of Leon et al., At. Dat. and Nucl. Dat. Tab. 69, 217 (1998)
   // developed for heavy ions in the GANIL energy range (it represents a fit to data measured using
   // GANIL beams).
   //
   // *N.B. Concerning equilibrium charge state distributions, it is not easy to know whether, for a given
   // combination of projectile, projectile energy, target, and target thickness, the equilibrium
   // distribution is reached or not. Here are some comments from the paper cited above which
   // may give some guidelines:
   //
   // "The energies available at the GANIL accelerator range from 24 to 95 MeV/u. Within this energy range,
   //  the equilibrium charge state is reached only for fairly thick targets (~1 mg/cm2 for C foils)."
   //
   // "Mean Charge State as a Function of the Target Thickness
   // A typical example of the variation of the mean charge as a function of the foil thickness is shown ... It is seen
   // that the mean charge initially increases due to the ionization process. Then, the equilibrium state is reached at
   // a certain thickness, the so-called equilibrium thickness, due to the equilibration of electron loss and
   // capture processes. Finally, for foils thicker than the equilibrium thickness, the mean charge decreases due to the
   // slowing down of the ions in matter leading to higher capture cross sections."
   //
   // It should be noted that, according to the data published in this and other papers, the equilibrium thickness
   // decreases with increasing atomic number of the target, and increases with increasing energy of the projectile.

   KVNucleus* proj = (KVNucleus*)fNuclei[1];
   Double_t Zp = proj->GetZ();
   proj->SetEnergy(t[0]*proj->GetA());
   Double_t beta = proj->Beta();
   Double_t vp = beta * KVParticle::C();
   Double_t Zt = ((KVNucleus*)fNuclei[2])->GetZ();

   Double_t q = Zp * (1. - TMath::Exp(-83.275 * beta / pow(Zp, 0.477)));

   // correction for target Z
   Double_t g = 0.929 + 0.269 * TMath::Exp(-0.16 * Zt) + (0.022 - 0.249 * TMath::Exp(-0.322 * Zt)) * vp / pow(Zp, 0.477);
   q *= g;

   if (Zp > 54) {
      // f(Zp) - correction for projectiles with Z>54
      Double_t f = 1. - TMath::Exp(-12.905 + 0.2124 * Zp - 0.00122 * pow(Zp, 2));
      q *= f;
   }

   return q;
}

//__________________________________________________________________________________________________

TF1* KV2Body::GetEqbmChargeStateFunc()
{
   // Return pointer to TF1 giving mean charge state of the projectile after passage through the target,
   // assuming that the equilibrium charge state distribution is achieved, as a function of projectile
   // energy after the target (in MeV/nucleon).
   // We use the empirical parameterization of Leon et al., At. Dat. and Nucl. Dat. Tab. 69, 217 (1998)
   // (see EqbmChargeState(Double_t *t,Double_t*) for details)
   //
   // By default the range of the function is [5,100] MeV/nucleon.
   // Change with TF1::SetRange.

   if (!fEqbmChargeState) {
      TString name = GetNucleus(1)->GetSymbol();
      name += " + ";
      name += GetNucleus(2)->GetSymbol();
      name += " LEON";
      fEqbmChargeState = new TF1(name.Data(),
                                 this, &KV2Body::EqbmChargeState, 5, 100, 0, "KV2Body", "EqbmChargeState");
      fEqbmChargeState->SetNpx(1000);
   }
   return fEqbmChargeState;
}

Double_t KV2Body::eqbm_charge_state_shiwietz_solid(Double_t* t, Double_t*)
{
   // G. Shiwietz et al Nucl. Instr. and Meth. in Phys. Res. B 175-177 (2001) 125-131
   // for solid targets

   KVNucleus* proj = (KVNucleus*)fNuclei[1];
   Double_t Zp = proj->GetZ();
   proj->SetEnergy(t[0]*proj->GetA());
   KVNucleus* targ = (KVNucleus*)fNuclei[2];
   Double_t Zt = targ->GetZ();
   Double_t Vp = proj->Beta();
   const Double_t V0 = 2.19e+06 / TMath::C();
   Double_t x = -0.019 * pow(Zp, -0.52) * Vp / V0;
   x = Vp / V0 * pow(Zp, -0.52) * pow(Zt, x) / 1.68;
   x = pow(x, 1. + 1.8 / Zp);
   Double_t q = 0.07 / x + 6. + 0.3 * pow(x, 0.5) + 10.37 * x + pow(x, 4);
   q = (Zp * (12 * x + pow(x, 4.))) / q;
   return q;
}
TF1* KV2Body::GetShiwietzEqbmChargeStateFuncForSolidTargets()
{
   // Return pointer to TF1 giving mean charge state of the projectile after passage through the target,
   // assuming that the equilibrium charge state distribution is achieved, as a function of projectile
   // energy after the target (in MeV/nucleon).
   // G. Shiwietz et al Nucl. Instr. and Meth. in Phys. Res. B 175-177 (2001) 125-131
   // This formula is valid for solid targets.
   //
   // By default the range of the function is [5,100] MeV/nucleon.
   // Change with TF1::SetRange.

   if (!fEqbmChargeStateShSol) {
      TString name = GetNucleus(1)->GetSymbol();
      name += " + ";
      name += GetNucleus(2)->GetSymbol();
      name += " SHIWIETZ-SOLID";
      fEqbmChargeStateShSol = new TF1(name.Data(),
                                      this, &KV2Body::eqbm_charge_state_shiwietz_solid, 5, 100, 0, "KV2Body", "eqbm_charge_state_shiwietz_solid");
      fEqbmChargeStateShSol->SetNpx(1000);
   }
   return fEqbmChargeStateShSol;
}

Double_t KV2Body::eqbm_charge_state_shiwietz_gas(Double_t* t, Double_t*)
{
   // G. Shiwietz et al Nucl. Instr. and Meth. in Phys. Res. B 175-177 (2001) 125-131
   // for gas targets

   KVNucleus* proj = (KVNucleus*)fNuclei[1];
   Double_t Zp = proj->GetZ();
   proj->SetEnergy(t[0]*proj->GetA());
   KVNucleus* targ = (KVNucleus*)fNuclei[2];
   Double_t Zt = targ->GetZ();
   Double_t Vp = proj->Beta();
   const Double_t V0 = 2.19e+06 / TMath::C();
   Double_t x = 0.03 - 0.017 * pow(Zp, -0.52) * Vp / V0;
   x = Vp / V0 * pow(Zp, -0.52) * pow(Zt, x);
   x = pow(x, 1. + 0.4 / Zp);
   Double_t q = 1428. - 1206.*pow(x, 0.5) + 690 * x + pow(x, 6.);
   q = (Zp * (376.*x + pow(x, 6.))) / q;
   return q;
}

TF1* KV2Body::GetShiwietzEqbmChargeStateFuncForGasTargets()
{
   // Return pointer to TF1 giving mean charge state of the projectile after passage through the target,
   // assuming that the equilibrium charge state distribution is achieved, as a function of projectile
   // energy after the target (in MeV/nucleon).
   // G. Shiwietz et al Nucl. Instr. and Meth. in Phys. Res. B 175-177 (2001) 125-131
   // This formula is valid for gas targets.
   //
   // By default the range of the function is [5,100] MeV/nucleon.
   // Change with TF1::SetRange.

   if (!fEqbmChargeStateShGas) {
      TString name = GetNucleus(1)->GetSymbol();
      name += " + ";
      name += GetNucleus(2)->GetSymbol();
      name += " SHIWIETZ-GAS";
      fEqbmChargeStateShGas = new TF1(name.Data(),
                                      this, &KV2Body::eqbm_charge_state_shiwietz_gas, 5, 100, 0, "KV2Body", "eqbm_charge_state_shiwietz_gas");
      fEqbmChargeStateShGas->SetNpx(1000);
   }
   return fEqbmChargeStateShGas;
}

//__________________________________________________________________________________________________

TF1* KV2Body::GetXSecRuthLabFunc(Int_t OfNucleus, Double_t theta_min, Double_t theta_max)
{
   // Return pointer to TF1 giving Rutherford cross-section (b/sr) in the Lab as a
   // function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle
   // By default, theta_min = 1 degree & theta_max = 179 degrees

   TString name = "RuthXSec: ";
   name += GetNucleus(1)->GetSymbol();
   name += " + ";
   name += GetNucleus(2)->GetSymbol();
   name += " ";
   Double_t elab = GetNucleus(1)->GetEnergy() / GetNucleus(1)->GetA();
   name += Form("%6.1f AMeV ", elab);
   if (OfNucleus == 3) name += "(projectile)";
   else name += "(target)";
   TF1* fXSecRuthLab = (TF1*)gROOT->GetListOfFunctions()->FindObject(name.Data());
   if (!fXSecRuthLab) {
      fXSecRuthLab = new TF1(name.Data(),
                             this, &KV2Body::XSecRuthLab, theta_min, theta_max, 1, "KV2Body", "XSecRuthLab");
      fXSecRuthLab->SetParameter(0, OfNucleus);
      fXSecRuthLab->SetNpx(1000);
   }
   fXSecRuthLab->SetRange(theta_min, theta_max); //in case TF1 already exists, but new range is required
   return fXSecRuthLab;
}
//__________________________________________________________________________________________________

TF1* KV2Body::GetXSecRuthLabIntegralFunc(Int_t OfNucleus, Double_t theta_min, Double_t theta_max)
{
   // Return pointer to TF1 giving Rutherford cross-section (b/sr) in the Lab as a
   // function of projectile (OfNucleus=3) or target (OfNucleus=4) lab scattering angle
   //
   // This function is equal to sin(theta)*dsigma/domega, i.e. it is the integrand
   // needed for calculating total cross-sections integrated over solid angle.
   //
   // WARNING: when integrating this function using TF1::Integral, the result must
   // be multiplied by TMath::DegToRad(), because 'x' is in degrees rather than radians,
   // e.g. the integrated cross-section in barns is given by
   //
   //    GetXSecRuthLabIntegralFunc(OfNucleus)->Integral(theta_min, theta_max)*TMath::DegToRad()

   TString name = "RuthXSecInt: ";
   name += GetNucleus(1)->GetSymbol();
   name += " + ";
   name += GetNucleus(2)->GetSymbol();
   name += " ";
   Double_t elab = GetNucleus(1)->GetEnergy() / GetNucleus(1)->GetA();
   name += Form("%6.1f AMeV ", elab);
   if (OfNucleus == 3) name += "(projectile)";
   else name += "(target)";

   TF1* fXSecRuthLab = (TF1*)gROOT->GetListOfFunctions()->FindObject(name.Data());
   if (!fXSecRuthLab) {
      fXSecRuthLab = new TF1(name.Data(),
                             this, &KV2Body::XSecRuthLabInt, theta_min, theta_max, 1, "KV2Body", "XSecRuthLabInt");
      fXSecRuthLab->SetParameter(0, OfNucleus);
      fXSecRuthLab->SetNpx(1000);
   }
   fXSecRuthLab->SetRange(theta_min, theta_max); //in case TF1 already exists, but new range is required
   return fXSecRuthLab;
}

//_____________________________________________________________________________________//

Int_t KV2Body::FindRoots(TF1* fonc, Double_t xmin, Double_t xmax, Double_t val, Double_t& x1, Double_t& x2) const
{
   // Find at most two solutions x1 and x2 between xmin and xmax for which fonc->Eval(x) = val
   // i.e. we use TF1::GetX for a function which may be (at most) double-valued in range (xmin, xmax)
   // This is adapted to the case of the lab angle vs. CM angle of scattered particles.
   // if fonc has a maximum between xmin and xmax, and if val < max, we look for x1 between
   // xmin and the maximum, and x2 between the maximum and xmax.
   // If not (single-valued function) we look for x1 between xmin and xmax.
   // This method returns the number of roots found.
   // If val > maximum of fonc between xmin and xmax, there is no solution: we return 0.

   x1 = x2 = xmin - 1.;
   Double_t max = fonc->GetMaximum(xmin, xmax);
   Int_t nRoots = 0;
   if (val > max) return nRoots;
   Double_t maxX = fonc->GetMaximumX(xmin, xmax);
   nRoots = 1;
   if (TMath::AreEqualAbs(val, max, 1.e-10)) {
      // value corresponds to maximum of function
      x1 = maxX;
      return nRoots;
   }
   if ((maxX < xmax) && (maxX > xmin)) nRoots = 2;
   Double_t xmax1 = (nRoots == 2 ? maxX : xmax);
   x1 = fonc->GetX(val, xmin, xmax1);
   if (nRoots == 1) return nRoots;
   else
      x2 = fonc->GetX(val, maxX, xmax);
   return nRoots;
}
