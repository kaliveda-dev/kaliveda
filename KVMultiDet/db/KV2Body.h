/***************************************************************************
                          KV2Body.h  -  description
                             -------------------
    begin                : 28/11/2003
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KV2Body.h,v 1.5 2009/02/02 13:52:29 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KV2BODY_H
#define KV2BODY_H

#include "TObject.h"
#include <vector>
#include "TVector3.h"
#include "KVNucleus.h"
#include "TF1.h"

class KV2Body: public TObject {

   std::vector<KVNucleus> fNuclei; //nuclei involved in calculation
   Double_t fEDiss;             //dissipated energy, 0 means elastic scattering

   TVector3 VCM;                //velocity of centre of mass
   Double_t BCM;                //beta of centre of mass
   Double_t WLT;                //total lab energy
   Double_t WCT;                //total cm energy
   Double_t WC[5];           //cm energy of each nucleus

   Double_t VC[5];              //cm velocities
   Double_t EC[5];              //cm energies
   Double_t K[5];                //ratio of c.m. velocity to velocity of nucleus in c.m. v_cm/v_i_cm
   Double_t TETAMAX[5];          //defined only for nuclei 3 et 4
   Double_t TETAMIN[5];          //defined only for nuclei 3 et 4

   mutable TF1* fKoxReactionXSec;   // function Kox reaction cross-section [barns] vs. E/A projectile
   mutable TF1* fEqbmChargeState;   // function equilibrium charge state of projectile vs. E/A projectile (Leon et al)
   mutable TF1* fEqbmChargeStateShSol;   // function equilibrium charge state of projectile vs. E/A projectile (Shiwietz et al solid)
   mutable TF1* fEqbmChargeStateShGas;   // function equilibrium charge state of projectile vs. E/A projectile (Shiwietz et al gas)

   void Set4thNucleus();
   Double_t ThetaLabVsThetaCM(Double_t*, Double_t*);
   Double_t ELabVsThetaCM(Double_t*, Double_t*);
   Double_t ELabVsThetaLab(Double_t*, Double_t*);
   Double_t XSecRuthLab(Double_t*, Double_t*);
   Double_t XSecRuthLabInt(Double_t*, Double_t*);
   Double_t XSecRuthCM(Double_t*, Double_t*);
   Double_t XSecRuthCMVsThetaCM(Double_t*, Double_t*);

   mutable TF1* fThetaLabVsThetaCM[5];
   mutable TF1* fELabVsThetaCM[5];
   mutable TF1* fELabVsThetaLab[5];

   mutable TF1* fXSecRuthLabIntegral[5];
   mutable TF1* fXSecRuthLab[5];

   Bool_t fSetOutgoing;// = kTRUE if SetOutgoing is called before CalculateKinematics

   Int_t FindRoots(TF1*, Double_t, Double_t, Double_t, Double_t&, Double_t&) const;
   Double_t fIntPrec;   //Precision of the TF1::Integral method

public:

   void init();
   KV2Body();
   KV2Body(const Char_t* systemname);
   KV2Body(const KVNucleus& compound, double Exx = 0.0);
   KV2Body(const KVNucleus& proj, const KVNucleus& targ, double Ediss = 0.0);
   KV2Body(const KVNucleus& proj, const KVNucleus& targ, const KVNucleus& proj_out, double Ediss = 0.0);
   KV2Body(KVNucleus*, KVNucleus* = nullptr, KVNucleus* = nullptr, Double_t = 0.0);
   virtual ~ KV2Body();

   void CalculateKinematics();

   static Double_t GetVelocity(Double_t mass, Double_t E);

   void SetProjectile(const KVNucleus&);
   void SetProjectile(Int_t z, Int_t a = 0);
   void SetTarget(const KVNucleus&);
   void SetTarget(Int_t z, Int_t a = 0);
   void SetOutgoing(const KVNucleus& proj_out);

   void SetExcitEnergy(Double_t ex)
   {
      fEDiss = ex;
   };
   Double_t GetExcitEnergy() const
   {
      return fEDiss;
   }
   void SetEDiss(Double_t ex)
   {
      SetExcitEnergy(ex);
   }
   Double_t GetEDiss() const
   {
      return GetExcitEnergy();
   }
   KVNucleus* GetNucleus(Int_t i) const;

   Double_t GetQReaction() const;
   Double_t GetQGroundStates() const;
   Double_t GetLabGrazingAngle(Int_t i = 1) const;

   Double_t GetCMEnergy() const;
   Double_t GetCMEnergy(Int_t i) const;

   Double_t GetMaxAngleLab(Int_t i) const;
   Double_t GetMinAngleLab(Int_t i) const;

   TVector3 GetCMVelocity() const;
   Double_t GetCMVelocity(Int_t i) const;
   Double_t GetCMGamma() const
   {
      Double_t gamma =
         1.0 - TMath::Power(BCM, 2.);
      if (gamma > 0.)
         gamma = 1. / TMath::Sqrt(gamma);
      else {
         Warning("GetCMGamma", "1 - (beta)**2 = %f ... strange!", gamma);
         gamma = 0.0;
      }
      return gamma;
   };

   TF1* GetThetaLabVsThetaCMFunc(Int_t OfNucleus) const;
   TF1* GetELabVsThetaCMFunc(Int_t OfNucleus) const;
   TF1* GetELabVsThetaLabFunc(Int_t OfNucleus) const;

   Double_t GetThetaLab(Double_t ThetaCM, Int_t OfNucleus) const
   {
      // Calculate lab angle of nucleus OfNucleus (=1,2,3,4) as a function of CM angle
      return const_cast<KV2Body*>(this)->GetThetaLabVsThetaCMFunc(OfNucleus)->Eval(ThetaCM);
   };
   Double_t GetELab(Double_t ThetaCM, Int_t OfNucleus) const
   {
      // Calculate lab energy of nucleus OfNucleus (=1,2,3,4) as a function of CM angle
      return const_cast<KV2Body*>(this)->GetELabVsThetaCMFunc(OfNucleus)->Eval(ThetaCM);
   };
   Int_t GetThetaCM(Double_t ThetaLab, Int_t OfNucleus, Double_t& t1, Double_t& t2) const;
   Double_t GetThetaCM(Int_t OfNucleus, Double_t theta, Int_t OtherNucleus) const
   {
      // Calculate projectile CM angle from target CM angle and vice versa
      if (TMath::Abs(OfNucleus - OtherNucleus) % 2) return 180. - theta;
      return theta;
   };
   Double_t GetMinThetaCMFromThetaLab(Int_t OfNucleus, Double_t theta, Int_t OtherNucleus) const;

   Int_t GetELab(Int_t OfNucleus, Double_t ThetaLab, Int_t AngleNucleus, Double_t& e1, Double_t& e2) const;
   Int_t GetVLab(Int_t OfNucleus, Double_t ThetaLab, Int_t AngleNucleus, Double_t& e1, Double_t& e2) const;
   Int_t GetThetaLab(Int_t OfNucleus, Double_t ThetaLab, Int_t AngleNucleus, Double_t& e1, Double_t& e2) const;

   Double_t GetXSecRuthLab(Double_t ThetaLab_Proj, Int_t OfNucleus = 3) const;
   Double_t GetXSecRuthCM(Double_t ThetaLab_Proj, Int_t OfNucleus = 3) const;

   Double_t GetIntegratedXSecRuthLab(Float_t th1, Float_t th2, Float_t phi1 = -1, Float_t phi2 = -1, Int_t OfNucleus = 3);
   //Double_t GetIntegratedXSecRuthLab(KVTelescope* tel, Int_t OfNucleus = 3);
   //Double_t GetIntegratedXSecRuthLab(KVDetector* det, Int_t OfNucleus = 3);

   TF1* GetXSecRuthLabFunc(Int_t OfNucleus = 3, Double_t theta_min = 1., Double_t theta_max = 179.) const;
   TF1* GetXSecRuthLabIntegralFunc(Int_t OfNucleus = 3, Double_t theta_min = 1., Double_t theta_max = 179.) const;

   void Print(Option_t* opt = "") const;

   Double_t BassIntBarrier();
   Double_t KoxReactionXSec(Double_t*, Double_t*);
   TF1* GetKoxReactionXSecFunc() const;

   Double_t GetSphereDureReactionXSec(Double_t r0 = 1.05);
   Double_t GetBmaxFromReactionXSec(Double_t ReacXsec);
   Double_t GetIntegratedXsec(Double_t b1, Double_t b2);

   Double_t EqbmChargeState(Double_t* t, Double_t*);
   TF1* GetEqbmChargeStateFunc() const;
   Double_t eqbm_charge_state_shiwietz_solid(Double_t* t, Double_t*);
   TF1* GetShiwietzEqbmChargeStateFuncForSolidTargets() const;
   Double_t eqbm_charge_state_shiwietz_gas(Double_t* t, Double_t*);
   TF1* GetShiwietzEqbmChargeStateFuncForGasTargets() const;

   Double_t GetIntegralPrecision()
   {
      //Precision of the TF1::Integral() method
      return fIntPrec;
   }
   void SetIntegralPrecision(Double_t precision)
   {
      //Set the precision of the TF1::Integral() method
      fIntPrec = precision;
   }

   ClassDef(KV2Body, 0)         //Relativistic binary kinematical calculation
};

#endif
