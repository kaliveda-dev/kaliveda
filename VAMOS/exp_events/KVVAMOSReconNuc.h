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

class KVVAMOSDetector;

class KVVAMOSReconNuc : public KVReconstructedNucleus {
private:

   KVVAMOSCodes fCodes; //Focal plane Pos. recon., calib. and ident. codes
protected:

   enum {
      kIsQAidentified = BIT(23)  //flag set when Q and A identification of particle is complete
   };

   KVVAMOSReconTrajectory fRT;             //handles trajectory reconstruction data
   Double_t               fStripFoilEloss; // calculated energy lost in the stripping foil

   Double_t*              fDetE;           //!array with the corrected energy lost in each detector of fDetList
   Float_t                fRealQ;          //Q returned by identification routine
   Float_t                fRealAoQ;        //A/Q returned by identification routine
   UChar_t fQ;                             // charge state
   Bool_t  fQMeasured;                     // true/false if charge state is measured/calculated

   virtual void CalibrateFromDetList();
   virtual void MakeDetectorList();
   KVVAMOSReconGeoNavigator* GetNavigator();

public:

   KVVAMOSReconNuc();
   KVVAMOSReconNuc(const KVVAMOSReconNuc&) ;
   virtual ~KVVAMOSReconNuc();
   virtual void Copy(TObject&) const;
   void init();

   Bool_t   GetCorrFlightDistanceAndTime(Double_t& dist, Double_t& tof, const Char_t* tof_name) const;
   virtual Double_t  GetCorrectedT_HF(Double_t tof, Double_t dist)   const;
   virtual void     Calibrate();
   virtual Bool_t   CheckTrackingCoherence();
   virtual void     Clear(Option_t* t = "");
   virtual void     GetAnglesFromStoppingDetector(Option_t* opt = "random");
   Double_t  GetDeltaPath(KVVAMOSDetector* det) const;
   using    KVReconstructedNucleus::GetEnergy;
   Int_t    GetDetectorIndex(const Char_t* det_label) const;
   Double_t  GetEnergy(const Char_t* det_label)        const;
   Double_t  GetEnergyAfter(const Char_t* det_label)   const;
   Double_t  GetEnergyBefore(const Char_t* det_label)  const;
   Double_t GetMassOverQ(const Char_t* tof_name)      const;
   Double_t  GetPath(KVVAMOSDetector* start, KVVAMOSDetector* stop = NULL)      const;
   Double_t  GetPath(const Char_t* start_label, const Char_t* stop_label = "") const;

   using KVReconstructedNucleus::GetRealA;
   Float_t GetRealA(const Char_t* tof_name)          const;
   Float_t GetRealAoverQ(const Char_t* tof_name)     const;
   Float_t GetRealQ(const Char_t* tof_name)          const;

   virtual void     IdentifyZ();
   virtual void     IdentifyQandA();
   virtual void     ReconstructTrajectory();
   virtual void     ReconstructFPtraj();
   virtual Bool_t   ReconstructFPtrajByFitting();
   virtual Bool_t   ReconstructLabTraj();

   virtual void     Propagate(ECalib cal = kNoCalib);
   virtual void     Print(Option_t* option = "") const;
   virtual void     SetQandAidentification(KVIdentificationResult* idr);


   //-------------- inline methods -----------------//
   static  Double_t         CalculateEnergy(Int_t Z, Int_t A, Int_t Q, Double_t Brho);
   static  Double_t         CalculateEnergy(Int_t Z, Int_t A, Double_t beta);
   static  Double_t         CalculateEnergy(Int_t Q, Double_t Brho, Double_t beta);
   static  Double_t         CalculateMassOverQ(Double_t Brho, Double_t beta);
   static  Double_t         CalculateRealA(Int_t Z, Double_t E, Double_t beta);
   Double_t         GetAoverQ()                         const;
   Double_t         GetBeta(const Char_t* tof_name)   const;
   Float_t          GetBrho()                           const;
   KVVAMOSCodes&    GetCodes();
   using            KVReconstructedNucleus::GetDetector;
   KVDetector*      GetDetector(const Char_t* det_label);
   Double_t          GetEnergy(Int_t idx_det)          const;
   Double_t         GetEnergyBeforeVAMOS()              const;
   const   TVector3&        GetFocalPlaneDirection()            const;
   Double_t         GetGamma(const Char_t* tof_name)  const;
   const   TVector3&        GetLabDirection()                   const;
   Float_t          GetPath()                           const;
   Double_t          GetPhiF()                           const;
   Double_t          GetPhiL()                           const;
   Double_t          GetPhiV()                           const;
   Int_t             GetQ()                              const;
   Float_t           GetRealAoverQ()                     const;
   Float_t           GetRealQ()                          const;


   virtual Double_t         GetStripFoilEnergyLoss()            const;
   Double_t          GetThetaF()                         const;
   Double_t          GetThetaL()                         const;
   Double_t          GetThetaV()                         const;
   Float_t           GetXf()                             const;
   Float_t           GetYf()                             const;
   using KVReconstructedNucleus::GetVelocity;
   Double_t         GetVelocity(const Char_t* tof_name) const;

   void             SetBrho(Double_t brho);
   virtual  void             SetECode(UChar_t code_mask);
   virtual void             SetFPCode(UInt_t code_mask);
   virtual void             SetIDCode(UShort_t code_mask);
   void             SetQ(Int_t q);
   virtual void             SetQMeasured(Bool_t yes = kTRUE);
   void             SetRealAoverQ(Float_t AoQ);
   void             SetRealQ(Float_t Q);
   virtual void             SetStripFoilEnergyLoss(Double_t e);
   virtual void             SetTCode(UShort_t code_mask);
   virtual void             SetTCode(const Char_t* parname);

   void             SetThetaVandPhiV(Double_t th_v, Double_t ph_v);
   void             SetXYf(Float_t x, Float_t y);

   Bool_t IsZidentified()                              const;
   Bool_t IsQandAidentified()                          const;
   Bool_t IsQMeasured()                                const;

   void SetIsZidentified();
   void SetIsZunidentified();
   void SetIsQandAidentified();
   void SetIsQandAunidentified();

   ClassDef(KVVAMOSReconNuc, 3) //Nucleus identified by VAMOS spectrometer
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

   Double_t M = CalculateMassOverQ(Brho, beta) * Q;
   if (M <= 0) return 0.;
   Double_t gamma = 1.0 / TMath::Sqrt(1 - beta * beta);
   return (gamma - 1) * M ;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateRealA(Int_t Z, Double_t E, Double_t beta)
{
   // Calculates the real value of the mass number deduced from the
   // energy E and and beta (i.e. velocity/c).
   // Begin_Latex
   // A = #frac{E}{(#gamma-1)u}
   // End_Latex
   // where
   //   u     : atomic mass unit in MeV/c^2
   //   gamma : Lorentz factor calculated from beta

   assert(Z > 0);
   assert(E > 0.);
   assert((beta > 0.) && (beta < 1.));

   if (beta <= 0) return 0.;

   Double_t gamma = 1.0 / TMath::Sqrt(1 - beta * beta);
   Double_t realA = E / ((gamma - 1) * u());
   if (Z > 0) {
      static KVNucleus nuc;
      nuc.SetZandA(Z, TMath::Nint(realA));
      realA -= nuc.GetMassExcess() / u();
   }
   return realA;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::CalculateMassOverQ(Double_t Brho, Double_t beta)
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

inline Double_t KVVAMOSReconNuc::GetAoverQ() const
{
   // returns the ration A/Q calculated from the integer values of the
   // identified A and Q.
   return Double_t(GetA()) / GetQ();
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetBeta(const Char_t* tof_name) const
{
   // returns beta= v/c where 'v' is the velocity of this nucleus
   // calculated from the time of flight with name 'tof_name'.
   return GetVelocity(tof_name) / C();
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetBrho() const
{
   return fRT.Brho;
}
//____________________________________________________________________________________________//

inline KVVAMOSCodes& KVVAMOSReconNuc::GetCodes()
{
   return fCodes;
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
   return (fDetE && idx_det < GetDetectorList()->GetEntries() ? fDetE[idx_det] : -1);
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetEnergyBeforeVAMOS() const
{
   // Kinetic energy of the nucleus prior to entering VAMOS
   return GetEnergy() - GetStripFoilEnergyLoss() - GetTargetEnergyLoss();
}
//____________________________________________________________________________________________//

inline const TVector3& KVVAMOSReconNuc::GetFocalPlaneDirection() const
{
   // Returns the trajectory normalized direction in the focal-plane frame
   // of reference
   return fRT.dirFP;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetGamma(const Char_t* tof_name) const
{
   // returns the relativistic quantitie gamma calculated from the
   // velocity deduced from the time of flight measurment with name
   // 'tof_name'
   Double_t b = GetBeta(tof_name);
   return 1.0 / TMath::Sqrt(1 - b * b);
}
//____________________________________________________________________________________________//

inline const TVector3& KVVAMOSReconNuc::GetLabDirection() const
{
   // Returns the trajectory normalized direction at the target point in the
   // laboratory frame of reference.
   return fRT.dirLab;
}
//____________________________________________________________________________________________//

inline KVVAMOSReconGeoNavigator* KVVAMOSReconNuc::GetNavigator()
{
   return gVamos->GetReconNavigator();
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

inline Double_t KVVAMOSReconNuc::GetPhiF() const
{
   return fRT.GetPhiF();
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

inline Int_t KVVAMOSReconNuc::GetQ() const
{
   //Returns the charge state of this nucleus (number of protons minus number of electrons).
   return (Int_t)fQ;
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetRealAoverQ() const
{
   // returns the real value of A/Q deduced from identification
   return (fRealAoQ > 0 ? fRealAoQ : (GetQ() > 0 ? ((Float_t)GetA()) / GetQ() : 0.));
}
//____________________________________________________________________________________________//

inline Float_t KVVAMOSReconNuc::GetRealQ() const
{
   // returns the real value of Q deduced from identification
   return (fRealQ > 0 ? fRealQ : (Float_t)GetQ());
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetStripFoilEnergyLoss() const
{
   // Returns calculated energy loss in stripping foil of reconstructed nucleus ( in MeV )
   return fStripFoilEloss;
}
//____________________________________________________________________________________________//

inline Double_t KVVAMOSReconNuc::GetThetaF() const
{
   return fRT.GetThetaF();
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

inline Double_t KVVAMOSReconNuc::GetVelocity(const Char_t* tof_name) const
{
   // returns the velocity calculated from the time of flight measurement
   // with name 'tof_name'. Before the velocity calculation, the
   // flight distance and the path are corrected (see GetPath and
   // GetCorrectedT_HF methods)

   Double_t t, d;
   t = d = 0.;
   if (GetCorrFlightDistanceAndTime(d, t, tof_name)) return d / t;
   return 0.;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetBrho(Double_t brho)
{
   fRT.Brho = brho;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetECode(UChar_t code_mask)
{
   //Sets code for energy calibration
   GetCodes().SetECode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetFPCode(UInt_t code_mask)
{
   //Sets code for energy calibration
   GetCodes().SetFPCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIDCode(UShort_t code_mask)
{
   //Sets code for identification
   GetCodes().SetIDCode(code_mask);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetQ(Int_t q)
{
   // set charge state.
   fQ = (UChar_t) q;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetQMeasured(Bool_t yes)
{
   // Call with yes=kTRUE for reconstructed nuclei whose
   // charge state, Q, was measured, not calculated
   fQMeasured = yes;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetRealAoverQ(Float_t AoQ)
{
   fRealAoQ = AoQ;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetRealQ(Float_t Q)
{
   fRealQ = Q;
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetStripFoilEnergyLoss(Double_t e)
{
   // Set energy loss in the stripping foil ( in MeV ) of reconstructed nucleus
   fStripFoilEloss = e;
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

inline Bool_t KVVAMOSReconNuc::IsZidentified() const
{
   return IsIdentified();
}
//____________________________________________________________________________________________//

inline Bool_t KVVAMOSReconNuc::IsQandAidentified() const
{
   return TestBit(kIsQAidentified);
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

inline void KVVAMOSReconNuc::SetIsQandAidentified()
{
   SetBit(kIsQAidentified);
}
//____________________________________________________________________________________________//

inline void KVVAMOSReconNuc::SetIsQandAunidentified()
{
   ResetBit(kIsQAidentified);
}


#endif
