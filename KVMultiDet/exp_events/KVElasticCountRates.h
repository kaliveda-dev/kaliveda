//Created by KVClassFactory on Fri Nov 20 2015
//Author: John Frankland

#ifndef __KVElasticCountRates_H
#define __KVElasticCountRates_H

#include "TVector3.h"
#include "KVNameValueList.h"
#include "KVPosition.h"
#include "TH1F.h"
#include "KVNucleus.h"
#include "KV2Body.h"
#include "KVTarget.h"
#include <map>

struct KVElasticCountRate {
   double count_rate;
   double mean_energy;
   double fluence;      // ions/sec./cm**2
   double dissipation;  // MeV/sec./cm**2
   KVElasticCountRate(double c = 0, double e = 0, double f = 0, double d = 0)
      : count_rate(c), mean_energy(e), fluence(f), dissipation(d) {}
   virtual ~KVElasticCountRate() {}

   ClassDef(KVElasticCountRate, 0) //Elastic scattering rate information for detector
};

class KVElasticCountRates {

   TH1F* fDepth;                //depth of scattering point in target
   TH1F* fTheta;                //angle of scattered particle
   Int_t  fBinE;                //Number of bins of the Energy histogram

   Double_t fEnergy;            //energy of projectile
   KVNucleus* fProj;            //scattered nucleus
   KVNucleus* fTarg;            //target nucleus

   KVPosition fAngularRange;    //angular range in which to scatter

   KV2Body* fKinematics;        //kinematics calculation

   KVTarget* fTarget;           //target for current run
   Double_t fAtomicDensity;     //number of atoms per barn (10^-24 cm2) in target
   Double_t fVolume;            //volume factor for MC integration
   Double_t xsec;
   Double_t theta, phi;
   Double_t fNtirages;
   Bool_t fMultiLayer;          //kTRUE for multilayer target

   TVector3 fBeamDirection;     //beam direction vector
   Int_t fIntLayer;             //index of interaction layer in multilayer target

   Double_t fExx;               //excited state of target nucleus

   KVHashList fHistos;          //histograms for all hit detectors

   std::map<std::string, KVElasticCountRate> fRates;

public:

   KVElasticCountRates(Double_t theta_min = 0, Double_t theta_max = 180, Double_t phi_min = 0, Double_t phi_max = 360);
   virtual ~ KVElasticCountRates();

   void SetRun(Int_t run);
   void SetProjectile(const Char_t* nuc, Double_t e_sur_a);
   void SetTargetScatteringLayer(const Char_t* name);
   void SetEbinning(Int_t nbins = 500);

   void CalculateScattering(Int_t N = 10000);
   void SetTargetExcitedState(Double_t ex)
   {
      //To simulate inelastic scattering i.e. where the target nucleus is left in
      //an excited state, use this method to set the excitation energy (in MeV).
      //This value will be used for all subsequent calculations.
      //Use SetTargetExcitedState(0) to return to elastic scattering.
      fExx = ex;
   }

   TH1F* GetDepth()
   {
      //Return pointer to histogram of 'depth' of scattering point in target (in mg/cm2)

      return fDepth;
   }
   TH1F* GetTheta()
   {
      //Return pointer to polar angle distribution of scattered particle (in degrees)

      return fTheta;
   }
   Int_t GetEbinning(void)
   {
      //Returns the number of bins of the GetEnergy histogram

      return fBinE;
   }

   void FillHistograms(KVNameValueList*);
   const KVHashList& GetHistos() const
   {
      return fHistos;
   };

   void PrintResults(Double_t beam_intensity = 1.e+07);

   KVElasticCountRate GetDetector(const std::string& name)
   {
      return fRates[name];
   }

   ClassDef(KVElasticCountRates, 1)//Calculate elastic scattering count rates in multidetector arrays
};

#endif
