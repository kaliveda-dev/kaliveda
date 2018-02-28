//Created by KVClassFactory on Thu Sep 13 10:51:51 2012
//Author: Guilain ADEMARD

#ifndef __KVVAMOSRECONNUC_H
#define __KVVAMOSRECONNUC_H

// C++
#include <cassert>

// ROOT
#include "TGraphErrors.h"
#include "TROOT.h"

// KaliVeda (Standard)
#include "KVNamedParameter.h"
#include "KVReconstructedNucleus.h"
#include "KVTarget.h"
#include "KVMacros.h" // 'UNUSED' macro

// KaliVeda (VAMOS)
#include "KVHarpeeCsI.h"
#include "KVVAMOS.h"
#include "KVVAMOSCodes.h"
#include "KVVAMOSDetector.h"
#include "KVVAMOSReconGeoNavigator.h"
#include "KVVAMOSReconTrajectory.h"
#include "KVVAMOSTransferMatrix.h"
#include "KVZGOUBIReconstruction.h"

class KVVAMOSDetector;

class KVVAMOSReconNuc : public KVReconstructedNucleus {

private:

   KVVAMOSCodes fCodes; //Focal plane Pos. recon., calib. and ident. codes

protected:

   enum {
      kIsBasicQAidentified = BIT(23),  //flag set when default Q and A identification of particle is complete
      kIsCorrQAidentified = BIT(24),   //flag set when corrected Q and A identification of particle is complete
   };

   KVVAMOSReconTrajectory  fRT;             //handles trajectory reconstruction data

   Double_t                fStripFoilEloss; //calculated energy lost in the stripping foil
   std::vector<Double_t>   fDetE;           //array with the corrected energy lost in each detector of fDetList

   //Observables returned by the mass and charge identification
   //routine (see IdentifyQandA() method)
   //
   //For the following observables, basic corrections on path, time of flight and
   //energy losses are applied:
   //   - path: path correction from FP to stop detector is applied (see GetDeltaPath())
   //   - ToF: HF frequency correction is applied (see GetCorrectedT_HF() method)
   //   - Eloss: Eloss of target, strip foil and detectors is applied
   //
   //Any other wanted corrections need to be defined in an analysis class
   //with a KVVAMOSDataCorrection inherited class.
   //
   //NOTE: if no corrections are applied on a given observable, then
   //      the returned value will be the uncorrected one
   Float_t                fBasicToF;        //ToF corrected of HF frequency for the first TCode OK, in ns
   Float_t                fBasicPath;       //Path corrected of FP-stop_detector distance, in cm
   Float_t                fBasicRealAE;     //Mass returned for the first TCode OK
   Float_t                fBasicRealAoQ;    //Mass over charge ration for the first TCode OK
   Float_t                fBasicRealQ;      //float value of identified charge state
   Int_t                  fBasicQ;          //int value of identified charge state
   Float_t                fBasicRealA;      //float value of identified mass
   Int_t                  fBasicA;          //int value of identified mass
   Bool_t                 fkIsBasicQandAID; //=kTRUE if basic Q and A identification was OK
   Int_t                  fBasicNHF;        //number of time the beam frequency was added for the ToF estimation

   //CsI minimizer informations
   Int_t   fA_CsI;                  //the mass found by CsI minimizer

   //Observables returned by the mass and charge identification
   //routine (see IdentifyQandA() method) AFTER data corrections
   //class call set in fDataCorr (see KVVAMOSDataCorrection class
   //and ApplyCorrection() method).
   Float_t                fToF_corr;          //ToF for the first TCode OK after ApplyCorrection() call
   Float_t                fPath_corr;         //Path after ApplyCorrections() call
   Double_t               fKE_corr;           //KE corrected (not including strip foil+target)
   Double_t               fEBeforeVAMOS_corr; //Energy corrected from data correction class (including strip foil+target)
   Float_t                fRealAE_corr;       //Mass after ApplyCorrections() call
   Float_t                fRealAoQ_corr;      //Mass over charge ration after ApplyCorrections() call
   Float_t                fRealQ_corr;        //Q float value after ID
   Float_t                fRealA_corr;        //A float value after ID
   Bool_t                 fkIsCorrected;      //=kTRUE if any correction were applied to the nucleus
   Bool_t                 fkIsCorrQandAID;    //=kTRUE if corrected Q and A identification was OK
   Int_t                  fCorrNHF;           //number of time the beam frequency was added to the corrected ToF


   //Final results of identification applied
   //NOTE: mass results are set using
   //KVNucleus::SetA() and KVNucleus::SetRealA()
   Float_t fRealQ;
   UChar_t fQ;                      //integer value of fRealQ
   Bool_t  fQMeasured;              //true/false if charge state is measured/calculated
   Bool_t  fVAMeasured;             //true/false if A from IdentifyQandA() method was measured/calculated

   //debug
   Bool_t  fdebug;     //=kTRUE for verbose mode

   //-------inline protected methods-------
   void SetBasicToF(Float_t tof);
   void SetBasicPath(Float_t dist);
   void SetBasicRealAE(Float_t realAE);
   void SetBasicRealAoverQ(Float_t realAoQ);
   void SetBasicRealQ(Float_t realQ);
   void SetBasicQ(Int_t intQ);
   void SetBasicRealA(Float_t realA);
   void SetBasicA(Int_t intA);
   void SetBasicNHF(Int_t nHF);
   void SetIsBasicQandAidentified();
   void SetIsBasicQandAunidentified();

   void SetAMinimizer(Int_t a);

   virtual void CalibrateFromDetList();
   virtual void MakeDetectorList();
   KVVAMOSReconGeoNavigator* GetNavigator();

public:

   KVVAMOSReconNuc();
   KVVAMOSReconNuc(const KVVAMOSReconNuc&) ;
   virtual ~KVVAMOSReconNuc();
   virtual void Copy(TObject&) const;
   void init();

   vector<Double_t> GetDetEVector() const
   {
      return fDetE;
   }

   virtual void     Calibrate();
   virtual Bool_t   CheckTrackingCoherence();
   virtual void     Clear(Option_t* t = "");
   virtual void     GetAnglesFromStoppingDetector(Option_t* opt = "random");
   using     KVReconstructedNucleus::GetEnergy;
   Int_t     GetDetectorIndex(const Char_t* det_label) const;
   Double_t  GetEnergy(const Char_t* det_label)        const;
   Double_t  GetEnergyAfter(const Char_t* det_label)   const;
   Double_t  GetEnergyBefore(const Char_t* det_label)  const;

   using KVReconstructedNucleus::GetRealA;
   Float_t   CalculateRealAE(const Char_t* tof_name)         const;
   Float_t   CalculateRealAoverQ(const Char_t* tof_name)     const;
   Float_t   CalculateRealQ(const Char_t* tof_name)          const;

   Double_t  CalculatePath(KVVAMOSDetector* start, KVVAMOSDetector* stop = NULL)      const;
   Double_t  CalculatePath(const Char_t* start_label, const Char_t* stop_label = "")  const;
   Double_t  FindDeltaPath(KVVAMOSDetector* det) const;
   Bool_t    CalculateCorrFlightDistanceAndTime(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const;
   virtual   Double_t  CalculateCorrectedT_HF(Double_t tof, Double_t dist, Int_t& nHF)   const;
   Int_t     CalculateNBeamPeriod(Double_t tof, Double_t dist) const;

   virtual void     ReconstructTrajectory();
   virtual void     ReconstructFPtraj();
   virtual Bool_t   ReconstructFPtrajByFitting();
   virtual Bool_t   ReconstructLabTraj();
   virtual void     Propagate(ECalib cal = kNoCalib);
   virtual void     IdentifyZ();
   virtual void     IdentifyQandA();
   using            KVReconstructedNucleus::SetIdentification;
   virtual void     SetZIdentification(KVIdentificationResult* idr, KVIDTelescope* idt);
   virtual void     SetBasicQandAIdentification(KVIdentificationResult* idr);
   virtual void     SetBasicQandAIdentification(const Char_t* tof_name, Float_t tof, Float_t dist, Int_t nHF, Float_t realAE, Float_t realAoQ);
   virtual void     SetCorrectedQandAIdentification();

   virtual void     SetCorrected(Bool_t corrected);
   virtual void     SetVerbose(Bool_t verbose);

   virtual void     Print(Option_t* option = "") const;

   virtual Int_t GetIDCode() const
   {
      // Returns value of VEDA ID code
      return const_cast<KVVAMOSReconNuc*>(this)->GetCodes().GetVedaIDCode();
   }

   Bool_t StoppedInChIo();
   Bool_t StoppedInSi();
   Bool_t StoppedInCsI();

   //-------------- inline methods -----------------//
   //Energy losses
   static  Double_t         CalculateEnergy(Int_t Z, Int_t A, Int_t Q, Double_t Brho);
   static  Double_t         CalculateEnergy(Int_t Z, Int_t A, Double_t beta);
   static  Double_t         CalculateEnergy(Int_t Q, Double_t Brho, Double_t beta);

   virtual Double_t         GetStripFoilEnergyLoss()            const;

   //Mass and Mass over Charge computation methods
   static  Double_t         CalculateRealAE(Double_t E, Double_t beta);
   static  Double_t         CalculateRealAoverQ(Double_t Brho, Double_t beta);

   //Velocity computation methods
   Double_t          CalculateGamma(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const;
   Double_t          CalculateBeta(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const;
   using KVReconstructedNucleus::GetVelocity;
   Double_t          CalculateVelocity(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const;

   //------Getters------
   using             KVReconstructedNucleus::GetDetector;
   KVDetector*       GetDetector(const Char_t* det_label);
   Double_t          GetEnergy(Int_t idx_det)            const;
   Double_t          GetEnergyBeforeVAMOS()              const;

   //Reconstruction at FP results
   const   TVector3& GetFocalPlaneDirection()            const;
   const   TVector3& GetLabDirection()                   const;
   Float_t           GetXf()                             const;
   Float_t           GetYf()                             const;
   Double_t          GetThetaF()                         const;
   Double_t          GetPhiF()                           const;
   Float_t           GetPath()                           const;
   Float_t           GetBrho()                           const;
   Double_t          GetThetaVE()                        const;
   Double_t          GetThetaI()                         const;
   Double_t          GetThetaL()                         const;
   Double_t          GetThetaV()                         const;
   Double_t          GetPhiVE()                          const;
   Double_t          GetPhiI()                           const;
   Double_t          GetPhiL()                           const;
   Double_t          GetPhiV()                           const;

   //CsI minimizer informations
   Int_t GetAMinimizer()                      const;

   //Mass and Mass over Charge uncorrected observables
   Float_t GetBasicToF()                      const;
   Float_t GetBasicPath()                     const;
   Float_t GetBasicBeta()                     const;
   Float_t GetBasicGamma()                    const;
   Float_t GetBasicVelocity()                 const;
   Float_t GetBasicRealAE()                   const;
   Float_t GetBasicRealAoverQ()               const;
   Float_t GetBasicRealQ()                    const;
   Int_t   GetBasicQ()                        const;
   Float_t GetBasicRealA()                    const;
   Int_t   GetBasicA()                        const;
   Int_t   GetBasicNHF()                      const;
   Bool_t  IsBasicQandAidentified()           const;

   //Mass and Mass over Charge corrected observables
   Float_t GetCorrectedToF()                  const;
   Float_t GetCorrectedPath()                 const;
   Double_t GetCorrectedEnergy()              const;
   Double_t GetCorrectedEnergyBeforeVAMOS()   const;
   Float_t GetCorrectedBeta()                 const;
   Float_t GetCorrectedGamma()                const;
   Float_t GetCorrectedVelocity()             const;
   Float_t GetCorrectedRealAE()               const;
   Float_t GetCorrectedRealAoverQ()           const;
   Float_t GetCorrectedRealQ()                const;
   Int_t   GetCorrectedQ()                    const;
   Float_t GetCorrectedRealA()                const;
   Int_t   GetCorrectedA()                    const;
   Int_t   GetCorrectedNHF()                  const;
   Bool_t  IsCorrectedQandAidentified()       const;

   //Final ID results
   KVVAMOSCodes&     GetCodes();
   Float_t           GetRealQ();
   Int_t             GetQ();
   Bool_t            IsCorrected();

   //------Setters------
   //Energy losses
   virtual void     SetStripFoilEnergyLoss(Double_t e);

   //Reconstruction at FP results (only use for simulations),
   //else all should be defined in 'fRT' at the raw->recon step
   virtual void     SetFPCode(UInt_t code_mask);
   void             SetBrho(Double_t brho);
   void             SetPath(Double_t path);
   void             SetThetaVandPhiV(Double_t th_v, Double_t ph_v);
   void             SetXYf(Float_t x, Float_t y);
   void             SetFPDirection(Double_t th_f, Double_t ph_f);

   //Mass and Mass over Charge corrected observables
   void SetCorrectedToF(Float_t tof);
   void SetCorrectedPath(Float_t path);
   void SetCorrectedEnergyBeforeVAMOS(Double_t energy_mev);
   void SetCorrectedEnergy(Double_t energy_mev);
   void SetCorrectedRealAE(Float_t realAE);
   void SetCorrectedRealAoverQ(Float_t realAoQ);
   void SetCorrectedRealQ(Float_t realQ);
   void SetCorrectedRealA(Float_t realA);
   void SetCorrectedNHF(Int_t nHF);
   void SetIsCorrectedQandAidentified();
   void SetIsCorrectedQandAunidentified();

   //Final ID results
   virtual void     SetECode(UChar_t code_mask);
   virtual void     SetIDCode(UShort_t code_mask);
   virtual void     SetTCode(UShort_t code_mask);
   virtual void     SetTCode(const Char_t* parname);
   virtual void     SetQ(Int_t q);
   virtual void     SetRealQ(Float_t q);
   virtual void     SetQMeasured(Bool_t yes = kTRUE);
   void             SetIsZidentified();
   void             SetIsZunidentified();
   Bool_t           IsZidentified()                              const;
   Bool_t           IsQMeasured()                                const;

   ClassDef(KVVAMOSReconNuc, 7) //Nucleus identified by VAMOS spectrometer
};

//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateEnergy(Int_t Z, Int_t A, Int_t Q, Double_t Brho)
{
   // Calculates kinetic energy (E) in MeV.
   // Begin_Latex
   // E = #sqrt{M^{2}+100 c^{2} B#rho^{2} Q^{2}}- M
   // End_Latex
   // where
   //   M     : mass deduced from Z and A, in MeV/c^2
   //   c     : speed of light in cm/ns
   //   Brho  : magnetic rigidity in T.m
   //   Q     : charge state
   //
   // If Z is set lower or equal 0 then the mass excess is null to calculate
   // M (i.e. M = A*u)

   assert(Z > 0);
   assert(A > 0);
   assert(Q > 0);
   assert(Brho > 0.);

   Double_t M = 0.;
   if (Z > 0) {
      static KVNucleus nuc;
      nuc.SetZandA(Z, A);
      M = nuc.GetMass();
   } else  M = A * u();
   return TMath::Sqrt(M * M + 100.*TMath::Power(C() * Brho * Q, 2)) - M;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateEnergy(Int_t Z, Int_t A, Double_t beta)
{
   // Calculates kinetic energy in MeV.
   // Begin_Latex
   // E = (#gamma-1)M
   // End_Latex
   // where
   //   M     : mass deduced from Z and A in, MeV/c^2
   //   gamma : Lorentz factor calculated from beta (velocity/c)
   //
   // If Z is set lower or equal 0 then the mass excess is null to calculate
   // M (i.e. M = A*u)

   assert(Z > 0);
   assert(A > 0);
   assert((beta > 0.) && (beta < 1.));

   if (beta <= 0) return 0.;

   Double_t M = 0.;
   if (Z > 0) {
      static KVNucleus nuc;
      nuc.SetZandA(Z, A);
      M = nuc.GetMass();
   } else  M = A * u();
   Double_t gamma = 1.0 / TMath::Sqrt(1 - beta * beta);
   return (gamma - 1) * M ;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateEnergy(Int_t Q, Double_t Brho, Double_t beta)
{
   // Calculates kinetic energy in MeV from Q, Brho and beta.
   // First the M/Q is calculated from Brho and beta.
   // Then the energy is given by
   // Begin_Latex
   // E = (#gamma-1)*(M/Q)*Q
   // End_Latex
   // where
   //   gamma : Lorentz factor calculated from beta (velocity/c)

   Double_t M = CalculateRealAoverQ(Brho, beta) * Q;
   if (M <= 0) return 0.;
   Double_t gamma = 1.0 / TMath::Sqrt(1 - beta * beta);
   return (gamma - 1) * M ;
}
//____________________________________________________________________________________________//

//inline Double_t KVVAMOSReconNuc::CalculateRealAE(Int_t Z, Double_t E, Double_t beta)
inline Double_t KVVAMOSReconNuc::CalculateRealAE(Double_t E, Double_t beta)
{
   // Calculates the mass number deduced from the
   // energy E and and beta (i.e. velocity/c).
   // Begin_Latex
   // A_{E} = #frac{E}{(#gamma-1)u}
   // End_Latex
   // where
   //   u     : atomic mass unit in MeV/c^2
   //   gamma : Lorentz factor calculated from beta

   //assert(Z > 0);
   assert(E > 0.);
   assert((beta > 0.) && (beta < 1.));

   if (beta <= 0) return 0.;

   Double_t gamma = 1.0 / TMath::Sqrt(1 - beta * beta);
   Double_t realAE = E / ((gamma - 1) * u());

   // UPDATE 16/04/2017: since RealAE is not the final mass
   // why bothering with mass excess ?
   //   if (Z > 0) {
   //      static KVNucleus nuc;
   //      nuc.SetZandA(Z, TMath::Nint(realAE));
   //      realAE -= nuc.GetMassExcess() / u();
   //   }

   return realAE;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateRealAoverQ(Double_t Brho, Double_t beta)
{
   // Calculates the ratio between the mass (MeV/c^2) and the charge state Q
   // calculated from the magnetic rigitidy Brho and the relativistic beta
   // (i.e. velocity/c).
   // The returned value is real.
   // Begin_Latex
   // #frac{M}{Q} = 10 C #frac{B_{#rho}}{ #gamma #beta}
   // End_Latex
   // where
   //   Brho  : magnetic rigidity in T.m
   //   u     : atomic mass unit in MeV/c^2
   //   C     : speed of light in vacuum in cm/ns
   //   gamma : Lorentz factor calculated from beta

   assert((beta > 0.) && (beta < 1.));
   assert(Brho > 0.);

   if (beta <= 0) return 0.;

   Double_t gamma = 1.0 / TMath::Sqrt(1 - beta * beta);
   Double_t tmp = beta * gamma;
   return Brho * C() * 10. / tmp;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateVelocity(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const
{
   // returns the velocity calculated from the time of flight measurement
   // with name 'tof_name'.

   if (CalculateCorrFlightDistanceAndTime(dist, tof, nHF, tof_name)) return dist / tof;
   return 0.;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateBeta(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const
{
   // returns beta= v/c where 'v' is the velocity of this nucleus
   // calculated from the time of flight with name 'tof_name'.

   return CalculateVelocity(dist, tof, nHF, tof_name) / C();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateGamma(Double_t& dist, Double_t& tof, Int_t& nHF, const Char_t* tof_name) const
{
   // returns the relativistic quantity gamma calculated from the
   // velocity deduced from the time of flight measurment with name
   // 'tof_name', with only basic path, tof and eloss corrections.

   Double_t b = CalculateBeta(dist, tof, nHF, tof_name);
   return 1.0 / TMath::Sqrt(1. - b * b);
}


//_____________________________________GETTERS________________________________________________//

inline Double_t KVVAMOSReconNuc::GetStripFoilEnergyLoss() const
{
   // Returns calculated energy loss in stripping foil of reconstructed nucleus ( in MeV )
   return fStripFoilEloss;
}


//_____________________________RECONSTRUCTION AT FP___________________________________________//
inline const TVector3& KVVAMOSReconNuc::GetFocalPlaneDirection() const
{
   // Returns the trajectory normalized direction in the focal-plane frame
   // of reference
   return fRT.dirFP;
}
//____________________________________________________________________________________________//

inline const TVector3& KVVAMOSReconNuc::GetLabDirection() const
{
   // Returns the trajectory normalized direction at the target point in the
   // laboratory frame of reference.
   return fRT.dirLab;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetXf() const
{
   return fRT.pointFP[0];
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetYf() const
{
   return fRT.pointFP[1];
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetThetaF() const
{
   return fRT.GetThetaF();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetPhiF() const
{
   return fRT.GetPhiF();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetPath() const
{
   return fRT.path;

   // This line is the correction proposed by M. Camano if the reconstruction
   // returns the projection of the real path on the horizontal plane
   // return fRT.path/TMath::Cos( GetPhiV()*TMath::DetToRad() );
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBrho() const
{
   return fRT.Brho;
}
//____________________________________________________________________________________________//
inline Double_t KVVAMOSReconNuc::GetPhiVE() const
{
   return fRT.GetPhiVE();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetPhiI() const
{
   return fRT.GetPhiI();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetPhiL() const
{
   return fRT.GetPhiL();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetPhiV() const
{
   return fRT.GetPhiV();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetThetaVE() const
{
   return fRT.GetThetaVE();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetThetaI() const
{
   return fRT.GetThetaI();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetThetaL() const
{
   return fRT.GetThetaL();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetThetaV() const
{
   return fRT.GetThetaV();
}

//____________________________________________________________________________________________//

inline KVDetector* KVVAMOSReconNuc::GetDetector(const Char_t* det_label)
{
   // Returns the detector in the list of detectors (fDetList)
   // through which particle passed, from its label ("CHI","SI","SED1","SED2",...).

   return GetDetector(GetDetectorIndex(det_label));
}
//____________________________________________________________________________________________//

inline Double_t  KVVAMOSReconNuc::GetEnergy(Int_t idx_det) const
{
   // Returns the calculated contribution of each detector to the
   // nucleus' energy from their index in fDetList. GetEnergy(0) returns
   // the contribution of the stopping detector.
   return (!fDetE.empty() && idx_det < GetDetectorList()->GetEntries() ? fDetE[idx_det] : -1);
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetEnergyBeforeVAMOS() const
{
   // Kinetic energy of the nucleus prior to entering VAMOS
   return GetEnergy() - GetStripFoilEnergyLoss() - GetTargetEnergyLoss();
}
//____________________________________________________________________________________________//

inline KVVAMOSReconGeoNavigator* KVVAMOSReconNuc::GetNavigator()
{
   return gVamos->GetReconNavigator();
}
//____________________________________________________________________________________________//

inline Int_t KVVAMOSReconNuc::GetAMinimizer() const
{
   //return the mass found by the CsI energy minimizer
   //will return default value '-666' if IDCode!=3.
   return fA_CsI;
}
//__________________________________BASIC ID OBSERVABLES_____________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicToF() const
{
   //return the ToF for the first TCode OK
   //in IdentifyQandA() method
   return fBasicToF;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicPath() const
{
   //return the path for the first TCode OK
   //in IdentifyQandA() method
   return fBasicPath;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicBeta() const
{
   //return the beta=v/c before data corrections
   return GetBasicVelocity() / C();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicGamma() const
{
   //return the velocity of the particle

   Double_t b = GetBasicBeta();
   return 1.0 / TMath::Sqrt(1. - b * b);
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicVelocity() const
{
   //return the velocity of the particle
   return fBasicPath / fBasicToF;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicRealAE() const
{
   //return the Mass of the particle computed from energy
   //conservation (set when calling IdentifyQandA() method)
   return fBasicRealAE;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicRealAoverQ() const
{
   //return the Masse over Charge ratio of the particle computed from
   //energy conservation (set when calling IdentifyQandA() method)
   return fBasicRealAoQ;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicRealQ() const
{
   //return the basic charge state from IdentifyQandA()
   //
   //if the charge state was set by hand, return it
   //else compute it from fBasicRealAE and fBasicRealAoverQ

   if (fBasicRealQ > 0.) return fBasicRealQ;
   else return GetBasicRealAE() / GetBasicRealAoverQ();
}
//____________________________________________________________________________________________//
inline Int_t KVVAMOSReconNuc::GetBasicQ() const
{
   // return the charge state of this nucleus (number of protons minus number of electrons).
   // from IdentifyQandA().
   // If the charge state was set by hand (so using KVIDQA grids), return the defined one
   // (so fBasicQ variable),
   // else return the integer value of GetBasicRealQ().

   if (fBasicQ > 0) return fBasicQ;
   else {
      Float_t qq = GetBasicRealQ();
      return TMath::Nint(qq);
   }
}

//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBasicRealA() const
{
   // return the mass of the nucleus from IdentifyQandA()
   // identification routine.
   // If it was defined by hand (so using KVIDQA grids), return the defined one
   // (so fBasicRealA)
   // else compute it from:
   // Begin_Latex
   // A = (int) Q * A_{E}
   // End_Latex
   // where
   //   (int) Q   : integer value of the charge state identified in
   //               basic identification (see IdentifyQandA() method)
   //   A_E       : mass from energy conservation formula (see CalculateRealAE() method)
   //               from basic identification (see IdentifyQandA() method)

   if (fBasicRealA > 0.) return fBasicRealA;
   else {
      Float_t aa = GetBasicQ() * GetBasicRealAoverQ();
      return aa;
   }
}

//____________________________________________________________________________________________//
inline Int_t KVVAMOSReconNuc::GetBasicA() const
{
   // return the integer value of the mass of the nucleus
   // from IdentifyQandA() identification routine.
   // If it was defined by hand (so using KVIDQA grids)
   // return the defined one (so fBasicA),
   // else compute the integer value from GetBasicRealA()

   if (fBasicA > 0) return fBasicA;
   else {
      Int_t aa = TMath::Nint(GetBasicRealA());
      return aa;
   }
}

//____________________________________________________________________________________________//
inline Int_t KVVAMOSReconNuc::GetBasicNHF() const
{
   //return the number of time the beam frequency was added to the ToF
   //in the basic identification (see CalculateCorrectedT_HF method)

   return fBasicNHF;
}

//___________________________OBSERVABLES FROM KVVAMOSDATACORRECTION___________________________//
inline Float_t KVVAMOSReconNuc::GetCorrectedToF() const
{
   //return the corrected ToF for the first TCode OK
   //(see IdentifyQandA() method)
   return fToF_corr;
}

//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetCorrectedPath() const
{
   //return the corrected Path i.e the path from
   //FP reconstruction routine
   return fPath_corr;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetCorrectedEnergy() const
{
   return fKE_corr;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetCorrectedEnergyBeforeVAMOS() const
{
   return fEBeforeVAMOS_corr;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetCorrectedBeta() const
{
   //return the uncorrected beta=v/c
   return GetCorrectedVelocity() / C();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetCorrectedGamma() const
{
   //return the velocity of the particle

   Double_t b = GetCorrectedBeta();
   return 1.0 / TMath::Sqrt(1. - b * b);
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetCorrectedVelocity() const
{
   //return the uncorrected velocity of the particle
   return GetCorrectedPath() / GetCorrectedToF();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetCorrectedRealAE() const
{
   //return the Mass of the particle computed from energy
   //conservation (set when calling IdentifyQandA() method)
   return fRealAE_corr;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetCorrectedRealAoverQ() const
{
   //return the Masse over Charge ratio of the particle computed from
   //energy conservation (set when calling IdentifyQandA() method)
   return fRealAoQ_corr;
}

//____________________________________________________________________________________________//
inline Float_t KVVAMOSReconNuc::GetCorrectedRealQ() const
{
   //return the corrected charge state
   return fRealQ_corr;
}

//____________________________________________________________________________________________//
inline Int_t KVVAMOSReconNuc::GetCorrectedQ() const
{
   //return integer value of corrected Q
   return TMath::Nint(fRealQ_corr);
}

//____________________________________________________________________________________________//
inline Float_t KVVAMOSReconNuc::GetCorrectedRealA() const
{
   //return the corrected mass
   return fRealA_corr;
}

//____________________________________________________________________________________________//
inline Int_t KVVAMOSReconNuc::GetCorrectedA() const
{
   return TMath::Nint(fRealA_corr);
}

//____________________________________________________________________________________________//
inline Int_t KVVAMOSReconNuc::GetCorrectedNHF() const
{
   //return the number of time the beam frequency was added to the ToF
   //with the corrected data

   return fCorrNHF;
}

//_____________________________VAMOS SPECIFIC ID RESULTS______________________________________//
inline KVVAMOSCodes& KVVAMOSReconNuc::GetCodes()
{
   return fCodes;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetRealQ()
{
   // return the final identificated value of Q
   return fRealQ;
}
//____________________________________________________________________________________________//

inline Int_t KVVAMOSReconNuc::GetQ()
{
   // return the finalidentificated integer value of Q
   return fQ;
}

inline Bool_t KVVAMOSReconNuc::IsCorrected()
{
   // return kTRUE if any data correction have been applied to the data
   return fkIsCorrected;
}

//_____________________________________SETTERS_______________________________________________//
//____________________________ ______ENERGY LOSS_____________________________________________//
inline void KVVAMOSReconNuc::SetStripFoilEnergyLoss(Double_t e)
{
   // Set energy loss in the stripping foil ( in MeV ) of reconstructed nucleus
   fStripFoilEloss = e;
}

//__________________________________FP RECONSTRUCTION________________________________________//

inline void KVVAMOSReconNuc::SetFPCode(UInt_t code_mask)
{
   //Sets code for energy calibration
   GetCodes().SetFPCode(code_mask);
}
inline void KVVAMOSReconNuc::SetBrho(Double_t brho)
{
   fRT.Brho = brho;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetPath(Double_t path)
{
   //This method allows to set the path for simulations (the path must be set and >0
   //in order to be able to propagate the nucleus, see Propagate() method)
   fRT.path = path;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetThetaVandPhiV(Double_t th_v, Double_t ph_v)
{
   // Set ThetaV and PhiV in Degree

   th_v *= TMath::DegToRad();   // conversion in rad
   ph_v *= TMath::DegToRad();   // conversion in rad

   Double_t x     = TMath::Sin(th_v) * TMath::Cos(ph_v);
   Double_t y     = TMath::Sin(ph_v);
   Double_t z     = TMath::Cos(th_v) * TMath::Cos(ph_v);

   fRT.dirLab.SetXYZ(x, y, z);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetXYf(Float_t x, Float_t y)
{
   //Sets the coordinates of the point of intersection (Xf, Yf) of the trajectory
   //and the focal plane in the FP-frame ( in cm ).
   fRT.pointFP[0] = x;
   fRT.pointFP[1] = y;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetFPDirection(Double_t th_f, Double_t ph_f)
{
   //Set the angle coordinates (ThetaF, PhiF) of the trajectory in the FP-frame ( in degree )

   th_f *= TMath::DegToRad();
   ph_f *= TMath::DegToRad();

   Double_t xx = TMath::Sin(th_f) * TMath::Cos(ph_f);
   Double_t yy = TMath::Sin(ph_f);
   Double_t zz = TMath::Cos(th_f) * TMath::Cos(ph_f);

   fRT.dirFP.SetXYZ(xx, yy, zz);
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetAMinimizer(Int_t a)
{
   fA_CsI = a;
}

//___________________________________BASIC ID OBSERVABLES_____________________________________//

inline void KVVAMOSReconNuc::SetBasicToF(Float_t tof)
{
   fBasicToF = tof;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicPath(Float_t dist)
{
   fBasicPath = dist;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicRealAE(Float_t realAE)
{
   fBasicRealAE = realAE;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicRealAoverQ(Float_t realAoQ)
{
   fBasicRealAoQ = realAoQ;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicRealQ(Float_t realQ)
{
   fBasicRealQ = realQ;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicQ(Int_t Q)
{
   fBasicQ = Q;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicRealA(Float_t realA)
{
   fBasicRealA = realA;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicA(Int_t A)
{
   fBasicA = A;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetBasicNHF(Int_t nHF)
{
   fBasicNHF = nHF;
}

//___________________________OBSERVABLES FROM KVVAMOSDATACORRECTION___________________________//
inline void KVVAMOSReconNuc::SetCorrectedToF(Float_t tof)
{
   fToF_corr = tof;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedPath(Float_t dist)
{
   fPath_corr = dist;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedEnergy(Double_t energy_mev)
{
   fKE_corr = energy_mev;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedEnergyBeforeVAMOS(Double_t energy_mev)
{
   fEBeforeVAMOS_corr = energy_mev;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedRealAE(Float_t realAE)
{
   fRealAE_corr = realAE;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedRealAoverQ(Float_t realAoQ)
{
   fRealAoQ_corr = realAoQ;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedRealQ(Float_t realQ)
{
   fRealQ_corr = realQ;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedRealA(Float_t realA)
{
   fRealA_corr = realA;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetCorrectedNHF(Int_t nHF)
{
   fCorrNHF = nHF;
}

//_____________________________VAMOS SPECIFIC ID RESULTS______________________________________//
inline void KVVAMOSReconNuc::SetECode(UChar_t code_mask)
{
   //Sets code for energy calibration
   GetCodes().SetECode(code_mask);
}

//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIDCode(UShort_t code_mask)
{
   //Sets code for identification
   GetCodes().SetIDCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetTCode(UShort_t code_mask)
{
   //Sets code for energy calibration
   GetCodes().SetTCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetTCode(const Char_t* parname)
{
   //Sets code for energy calibration from the name of the acquisition
   //parameter used for the time of flight of the nucleus
   GetCodes().SetTCode(parname);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetRealQ(Float_t q)
{
   // set final charge state.
   fRealQ = q;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetQ(Int_t q)
{
   // set final charge state.
   fQ = (UChar_t) q;
}

//____________________________________________________________________________________________//
inline void KVVAMOSReconNuc::SetQMeasured(Bool_t yes)
{
   // Call with yes=kTRUE for reconstructed nuclei whose
   // charge state, Q, was measured during IdentifyQandA(), not calculated
   fQMeasured = yes;
}
//____________________________________________________________________________________________//

inline Bool_t KVVAMOSReconNuc::IsZidentified() const
{
   return IsIdentified();
}
//____________________________________________________________________________________________//

inline Bool_t KVVAMOSReconNuc::IsBasicQandAidentified() const
{
   return TestBit(kIsBasicQAidentified);
}
//____________________________________________________________________________________________//

inline Bool_t KVVAMOSReconNuc::IsCorrectedQandAidentified() const
{
   return TestBit(kIsCorrQAidentified);
}
//____________________________________________________________________________________________//


inline Bool_t KVVAMOSReconNuc::IsQMeasured() const
{
   // Returns kTRUE for reconstructed nuclei whose
   // charge state, Q, was measured, not calculated
   return fQMeasured;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsZidentified()
{
   SetIsIdentified();
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsZunidentified()
{
   SetIsUnidentified();
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsBasicQandAidentified()
{
   ResetBit(kIsCorrQAidentified);
   SetBit(kIsBasicQAidentified);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsBasicQandAunidentified()
{
   ResetBit(kIsBasicQAidentified);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsCorrectedQandAidentified()
{
   ResetBit(kIsBasicQAidentified);
   SetBit(kIsCorrQAidentified);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsCorrectedQandAunidentified()
{
   ResetBit(kIsCorrQAidentified);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetCorrected(Bool_t corrected)
{
   //=kTRUE if any correction was applied to the data
   fkIsCorrected = corrected;
}

//______________________________________VERBOSE_______________________________________________//
inline void KVVAMOSReconNuc::SetVerbose(Bool_t debug)
{
   fdebug = debug;
}

#endif
