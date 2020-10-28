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

/**
\class KVElasticCountRates
\brief Calculate elastic scattering count rates in multidetector arrays
\ingroup Simulation

Use this class to calculate the count rates & energy losses of elastically scattered nuclei
in the detectors of a multidetector array. It is assumed that the following
global pointers have been initialised before using this class:

<ul>
   <li><pre>gMultiDetArray</pre> - points to object describing multidetector array</li>
   <li><pre>gExpDB</pre> - points to database of run-dependent information for experimental dataset</li>
</ul>

In other words, we assume that the user has chosen a dataset on which she wants
to work:

<pre>gDataSetManager->GetDataSet("name_of_dataset")->cd()</pre>

and then initialised the description of the experimental configuration for the dataset:

<pre>KVMultiDetArray::MakeMultiDetector(gDataSet->GetName())</pre>

<h3>Setting up the calculation</h3>

Create a new elastic scattering object defining angular range in which to perform calculation:

<pre>KVElasticCountRates ecr(2,45,0,360)</pre>

Then call the following methods in any order in order to set up the calculation:

<ul>
   <li><pre>ecr.SetProjectile(...)</pre></li>
   <li><pre>ecr.SetRun(...)</pre></li>
</ul>

The SetRun(...) method uses the experimental database in order to determine the target
for the run, detector state (gas pressures etc.).

<h3>Multilayer targets</h3>

When multilayer targets are used, the user can restrict the scattering calculation to one
specific component of the target, i.e. the scattering takes place between the projectile
and one of the nuclei of the specified layer. Energy losses before and after the scattering
in the other layers of the target are of course still taken into account. To set the part of
the target where scattering takes place use

<pre>ecr.SetTargetScatteringLayer(...)</pre>

with the name of the layer you require (this is normally the name of the element
making up the layer - see KVTarget class for details).

<h3>Inelastic scattering</h3>

Calculations can also be performed for inelastic scattering i.e. when the target nucleus is left
in an excited state (N.B. we still use the Rutherford elastic scattering cross-section for
weighting the energy loss distributions). In order to do this, call

<pre>ecr.SetTargetExcitedState(...)</pre>

with the energy of the excited state of the target after scattering.

<h3>Running the calculation, obtaining the results</h3>

To perform the calculation, do:

<pre>ecr.CaclulateScattering(10000)</pre>

Make sure the number of sampled points (default=10000) is large enough for accurate determination of count rates.
At the end of calculation, we print infos for all detectors hit by elastic particles, these are the count rates for
a nominal beam intensity of 10**7 particles per second, the mean energy loss in the detector,
the total integrated cross-section per detector in barn, the number of particles per second per unit area hitting
each detector, and the total energy per second per unit area deposited in each detector:

<pre>
kaliveda [3] esa.CalculateScattering(10000)
SI_0101  :  N=  204.42/sec.    <E>=  890.3 MeV   Tot.Xsec=6.687E+01 barn    fluence=2.711E+01/sec./cm**2     dissip.=2.413E+04 MeV/sec./cm**2
SI_0201  :  N=   61.37/sec.    <E>=  709.7 MeV   Tot.Xsec=2.008E+01 barn    fluence=1.413E+01/sec./cm**2     dissip.=1.002E+04 MeV/sec./cm**2
SI_0301  :  N=   15.29/sec.    <E>=  706.4 MeV   Tot.Xsec=5.002E+00 barn    fluence=2.390E+00/sec./cm**2     dissip.=1.688E+03 MeV/sec./cm**2
SI_0401  :  N=    6.33/sec.    <E>=  699.2 MeV   Tot.Xsec=2.070E+00 barn    fluence=1.582E+00/sec./cm**2     dissip.=1.106E+03 MeV/sec./cm**2
</pre>

If you want to see the count rates for a different beam intensity, call

<pre>ecr.PrintResults(5.e+06)</pr>

For each detector hit we fill three histograms:

<pre>
 OBJ: TH1F  SI_0223  Eloss in SI_0223 : 0 at: 0x10898980
 OBJ: TH1F  SI_0223_dW  Solid angle of SI_0223 : 0 at: 0x10899540
 OBJ: TH2F  SI_0223_map Map of SI_0223 : 0 at: 0x10e2e7d0
</pre>

Each histogram can be obtained using

<pre>ecr.GetHistos().FindObject("SI_0223_map")</pre>

The different informations for each detector can be obtained using

<pre>
kaliveda [7] ecr.GetDetector("SI_0201").count_rate
(double)6.13702995974404857e+01
kaliveda [8] ecr.GetDetector("SI_0201").mean_energy
(double)7.09659910827807721e+02
kaliveda [9] ecr.GetDetector("SI_0201").intXsec
(double)2.00758282694385635e+01
</pre>

See KVElasticCountRate class for details.

\warning The values returned correspond to the last beam intensity value given to PrintResults().

*/

/**
  \struct KVElasticCountRate
  \brief Utility class used by KVElasticCountRates to store results
  \ingroup Simulation
 */
struct KVElasticCountRate {
   double count_rate;
   double mean_energy;
   double intXsec;      // integrated X-section (barn)
   double fluence;      // ions/sec./cm**2
   double dissipation;  // MeV/sec./cm**2
   KVElasticCountRate(double c = 0, double e = 0, double i = 0, double f = 0, double d = 0)
      : count_rate(c), mean_energy(e), intXsec(i), fluence(f), dissipation(d) {}
   virtual ~KVElasticCountRate() {}

   ClassDef(KVElasticCountRate, 0) //Elastic scattering rate information for detector
};

class KVElasticCountRates {

   TH1F* fDepth;                //depth of scattering point in target
   TH1F* fTheta;                //angle of scattered particle
   Int_t  fBinE;                //Number of bins of the Energy histogram

   Double_t fEnergy;            //energy of projectile
   KVNucleus fProj;            //scattered nucleus

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
   KVNameValueList PutResultsInList(Double_t beam_intensity = 1.e+07);

   KVElasticCountRate GetDetector(const std::string& name)
   {
      return fRates[name];
   }

   ClassDef(KVElasticCountRates, 1)//Calculate elastic scattering count rates in multidetector arrays
};

#endif
