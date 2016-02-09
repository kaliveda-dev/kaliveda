//Created by KVClassFactory on Thu Mar 28 14:12:49 2013
//Author: Guilain ADEMARD

#ifndef __KVVAMOSRECONTRAJECTORY_H
#define __KVVAMOSRECONTRAJECTORY_H

#include "KVBase.h"
#include "TVector3.h"
#include "TMath.h"

class KVVAMOSReconTrajectory : public KVBase {
   friend class KVVAMOSReconNuc;
   friend class KVVAMOSTransferMatrix;

protected:
   enum {
      kFPisOK  = BIT(14), //flag set when FP parameters are set
      kLabIsOK = BIT(15), //flag set when Lab parameters (direction end Brho) are set
      kFPtoLabAttempted = BIT(16),//Flag set when a reconstruction attempt was made from FP to Lab
      kLabToFPattempted = BIT(17),//Flag set when a reconstruction attempt was made from Lab to Lab

   };


   Float_t pointFP[2];//point of intersection (Xf, Yf in cm) of the trajectory and the focal plane in the FP-frame
   TVector3 dirFP;    //trajectory normalized direction in the FP-frame

   TVector3 dirLab;   //trajectory normalized direction at the target point in the lab-frame
   Float_t Brho;      //magnetic rigidity in the laboratory (T.m)
   Float_t path;      //path of the nucleus, from the target to the focal plane (cm)


public:

   void init();

   KVVAMOSReconTrajectory();
   KVVAMOSReconTrajectory(const KVVAMOSReconTrajectory&) ;
   virtual ~KVVAMOSReconTrajectory();
   virtual void Copy(TObject&) const;


   virtual void Reset();


   inline Double_t GetThetaF() const
   {
      //Returns the angle (in degree) between the Z-axis and the projection of the velocity vector
      //of the trajectory on the XZ plane (symmetry plane) in the focal-plane frame of reference.
      //It varies between -90 to 90 degrees.

      return TMath::RadToDeg() * TMath::ATan(dirFP.X() / dirFP.Z());

   }

   inline Double_t GetPhiF() const
   {
      //Returns the angle (in degree) between the velocity vector of the trajectory and its projection
      //on the XZ plane (symmetry plane) in the focal plane frame of reference.
      //It varies between -90 to 90 degrees.

      return TMath::RadToDeg() * TMath::ASin(dirFP.Y() / dirFP.Mag());
   }


   inline Double_t GetThetaV() const
   {
      //Returns the angle (in degree) between the Z-axis and the projection of the velocity vector
      //of the trajectory on the XZ plane (symmetry plane) in the laboratory frame of reference and in the
      //VAMOS coordinate system.
      //It varies between -90 to 90 degrees.

      return TMath::RadToDeg() * TMath::ATan(dirLab.X() / dirLab.Z());

   }

   inline Double_t GetPhiV() const
   {
      //Returns the angle (in degree) between the velocity vector of the trajectory and its projection
      //on the XZ plane (symmetry plane) in the laboratory frame of reference and in the VAMOS coordinate system.
      //It varies between -90 to 90 degrees.


      return TMath::RadToDeg() * TMath::ASin(dirLab.Y() / dirLab.Mag());
   }


   inline Double_t GetThetaL() const
   {
      //Returns the angle (in degree) between the Z-axis and the the velocity vector of the trajectory
      //(i.e. polar angle in spherical coordinates) in the laboratory frame of reference and in the VAMOS coordinate system.
      //It varies between 0 to 180 degrees.

      return TMath::RadToDeg() * dirLab.Theta();

   }

   inline Double_t GetPhiL() const
   {
      //Returns the angle (in degree) between the X-axis and the projection of the velocity vector of the trajectory
      //on the XY-plane (i.e. azimuth angle in spherical coordinates) in the laboratory frame of reference
      //and in the VAMOS coordinate system.
      //It varies between -179.999... to 180 degrees.

      return TMath::RadToDeg() * dirLab.Phi();
   }

   inline Float_t GetBrho() const
   {
      //Returns the magnetic rigidity in T.m
      return Brho;
   }

   inline Float_t GetPath() const
   {
      //Retruns the path of the particle between the target point to the
      //focal plane in cm
      return path;
   }

   inline void SetFPparamsReady()
   {
      // Set flag when the Focal plane parameters
      // (point and direction) are set.
      SetBit(kFPisOK);
   }

   inline Bool_t FPparamsAreReady() const
   {
      // Returns true if the focal plane parameters was set.
      return TestBit(kFPisOK);
   }

   inline void SetLabParamsReady()
   {
      // Set flag when the Laboratory parameters
      // (direction and Brho) are set.
      SetBit(kLabIsOK);
   }

   inline Bool_t LabParamsAreReady() const
   {
      // Returns true if the laboratory parameters was set.
      return TestBit(kLabIsOK);
   }

   inline void SetFPtoLabAttempted()
   {
      // Set flag when the reconstruction of the trajectory
      // in the laboratory reference frame is attempted
      // from the parameters of the focal plane.
      SetBit(kFPtoLabAttempted);
   }

   inline Bool_t FPtoLabWasAttempted() const
   {
      // Returns true if the reconstruction of the trajector
      // in the laboratory reference frame was attempted
      // from the parameters of the focal plane.
      return TestBit(kFPtoLabAttempted);
   }

   inline void SetLabToFPattempted()
   {
      // Set flag when the reconstruction of the trajectory
      // at the focal plane reference frame is attempted from
      // the parameters of the laboratory.
      SetBit(kLabToFPattempted);
   }

   inline Bool_t LabToFPwasAttempted() const
   {
      // Returns true if the reconstruction of the trajectory
      // at the focal plane reference frame is attempted from
      // the parameters of the laboratory.
      return TestBit(kLabToFPattempted);
   }

   ClassDef(KVVAMOSReconTrajectory, 1) //Handle data used to reconstruct the trajectory of a nucleus in VAMOS
};

#endif
