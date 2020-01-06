/*
$Id: KVElasticScatter.h,v 1.6 2007/04/04 10:39:17 ebonnet Exp $
$Revision: 1.6 $
$Date: 2007/04/04 10:39:17 $
*/

//Created by KVClassFactory on Thu Oct 19 22:31:02 2006
//Author: John Frankland

#ifndef __KVELASTICSCATTER_H
#define __KVELASTICSCATTER_H

#include "TVector3.h"
#include "KVNameValueList.h"
#include "KVNucleus.h"

class TH1F;
class TList;
class KVGroup;
class KVTelescope;
class KVDetector;
class KVTarget;
class KV2Body;
class TObjArray;

class KVElasticScatter {
   TH1F* fDepth;                //depth of scattering point in target
   TH1F* fTheta;                //angle of scattered particle
   Int_t  fBinE;                //Number of bins of the Energy histogram

   Double_t fEnergy;            //energy of projectile
   KVNucleus fProj;            //scattered nucleus

   KV2Body* fKinematics;        //kinematics calculation

   KVTelescope* fTelescope;     //telescope where particle will be detected
   KVDetector* fDetector;       //detector where particle will be detected
   TList* fAlignedDetectors;    //all aligned detectors
   Int_t fNDets;                //number of aligned detectors
   KVTarget* fTarget;           //target for current run
   Bool_t fMultiLayer;          //kTRUE for multilayer target

   TVector3 fBeamDirection;     //beam direction vector
   Int_t fIntLayer;             //index of interaction layer in multilayer target

   Double_t fExx;               //excited state of target nucleus

   TObjArray* fHistos;          //energy loss histograms for all hit detectors
   KVNameValueList* fDetInd;    //detector type-index association

public:

   KVElasticScatter();
   virtual ~ KVElasticScatter();

   void SetRun(Int_t run);
   void SetProjectile(Int_t Z, Int_t A);
   void SetEnergy(Double_t E);
   void SetTargetScatteringLayer(const Char_t* name);
   //Set the number of bins of the GetEnergy() histogram
   //Default value is 500; this function has to be called before
   //using CalculateScattering.
   void SetEbinning(Int_t nbins = 500);
   void SetDetector(const Char_t* det);
   void CalculateScattering(Int_t N);
   //To simulate inelastic scattering i.e. where the target nucleus is left in
   //an excited state, use this method to set the excitation energy (in MeV).
   //This value will be used for all subsequent calculations.
   //Use SetTargetExcitedState(0) to return to elastic scattering.
   void SetTargetExcitedState(Double_t ex)
   {
      fExx = ex;
   };

   //Return pointer to histogram of 'depth' of scattering point in target (in mg/cm2)
   TH1F* GetDepth()
   {
      return fDepth;
   };
   //Return pointer to energy loss histogram for chosen detector (in MeV)
   TH1F* GetEnergy()
   {
      return GetEnergy(fNDets - 1);
   };
   //Return pointer to polar angle distribution of scattered particle (in degrees)
   TH1F* GetTheta()
   {
      return fTheta;
   };
   TH1F* GetEnergy(const Char_t* type);
   TH1F* GetEnergy(Int_t index);
   //Returns the number of detectors crossed by the scattered particle
   Int_t GetNDets() const
   {
      return fNDets;
   };
   //Returns the number of bins of the GetEnergy histogram
   Int_t GetEbinning(void)
   {
      return fBinE;
   };

   ClassDef(KVElasticScatter, 1)        //Calculate elastic scattering spectra in multidetector arrays
};

#endif
