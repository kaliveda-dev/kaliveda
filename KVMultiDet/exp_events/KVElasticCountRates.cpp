/*
$Id: KVElasticCountRates.cpp,v 1.9 2007/04/04 10:39:17 ebonnet Exp $
$Revision: 1.9 $
$Date: 2007/04/04 10:39:17 $
*/

//Created by KVClassFactory on Fri Nov 20 2015
//Author: John Frankland

#include "KVElasticCountRates.h"
#include "KVMultiDetArray.h"
#include "TH1F.h"
#include "KVDetector.h"
#include "KVTelescope.h"
#include "KVGroup.h"
#include "KVTarget.h"
#include "KV2Body.h"
#include "KVNucleus.h"
#include "TObjArray.h"

#include <TH2F.h>
#include <vector>
#include <algorithm>

using namespace std;

ClassImp(KVElasticCountRate)
ClassImp(KVElasticCountRates)
////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVElasticCountRates</h2>
<h4>Calculate elastic scattering count rates in multidetector arrays</h4>

<p>
Use this class to calculate the count rates & energy losses of elastically scattered nuclei
in the detectors of a multidetector array. It is assumed that the following
global pointers have been initialised before using this class:
</p>
<ul>
   <li><pre><a href="KVMultiDetArray.html">gMultiDetArray</a></pre> - points to object describing multidetector array</li>
   <li><pre><a href="KVDataBase.html">gDataBase</a></pre> - points to database of run-dependent information for experimental dataset</li>
</ul>
<p>
In other words, we assume that the user has chosen a dataset on which she wants
to work:
</p>
<pre><a href="KVDataSetManager.html">gDataSetManager</a>-><a href="KVDataSetManager.html#KVDataSetManager:GetDataSet">GetDataSet</a>("name_of_dataset")-><a href="KVDataSet.html#KVDataSet:cd">cd</a>()</pre>
<p>
and then initialised the description of the experimental configuration for the dataset:
</p>
<pre><a href="KVMultiDetArray.html">KVMultiDetArray</a>::<a href="KVMultiDetArray.html#KVMultiDetArray:MakeMultiDetector">MakeMultiDetector</a>(gDataSet->GetName())</pre>
<h3>Setting up the calculation</h3>
<p>Create a new elastic scattering object defining angular range in which to perform calculation:</p>
<pre>KVElasticCountRates ecr(2,45,0,360)</pre>
<p>
Then call the following methods in any order in order to set up the calculation:</p>
<ul>
   <li><pre>ecr.<a href="#KVElasticCountRates:SetProjectile">SetProjectile</a>(...)</pre></li>
   <li><pre>ecr.<a href="#KVElasticCountRates:SetRun">SetRun</a>(...)</pre></li>
</ul>
<p>
The SetRun(...) method uses the experimental database in order to determine the target
for the run, detector state (gas pressures etc.).
</p>
<h3>Multilayer targets</h3>
<p>
When multilayer targets are used, the user can restrict the scattering calculation to one
specific component of the target, i.e. the scattering takes place between the projectile
and one of the nuclei of the specified layer. Energy losses before and after the scattering
in the other layers of the target are of course still taken into account. To set the part of
the target where scattering takes place use
</p>
<pre>ecr.<a href="#KVElasticCountRates:SetTargetScatteringLayer">SetTargetScatteringLayer</a>(...)</pre>
<p>
with the name of the layer you require (this is normally the name of the element
making up the layer - see <a href="KVTarget.html">KVTarget</a> for details).
</p>
<h3>Inelastic scattering</h3>
<p>
Calculations can also be performed for inelastic scattering i.e. when the target nucleus is left
in an excited state (N.B. we still use the Rutherford elastic scattering cross-section for
weighting the energy loss distributions). In order to do this, call
</p>
<pre>ecr.<a href="#KVElasticCountRates:SetTargetExcitedState">SetTargetExcitedState</a>(...)</pre>
<p>
with the energy of the excited state of the target after scattering.
</p>
<h3>Running the calculation, obtaining the results</h3>
<p>To perform the calculation, do:</p>
<pre>ecr.CaclulateScattering(10000)</pre>
<p>Make sure the number of sampled points (default=10000) is large enough for accurate determination of count rates.
At the end of calculation, we print infos for all detectors hit by elastic particles, these are the count rates for
a nominal beam intensity of 10**7 particles per second, and the mean energy loss in the detector:</p>
<pre>
kaliveda [3] esa.CalculateScattering(10000)
SI_0112 : N=2.493e+00/sec.  <E>=1.659e+03 MeV
SI_0101 : N=1.572e+01/sec.  <E>=1.659e+03 MeV
SI_0102 : N=2.559e+00/sec.  <E>=1.659e+03 MeV
SI_0223 : N=1.148e+00/sec.  <E>=1.457e+03 MeV
CI_0223 : N=1.821e+00/sec.  <E>=8.248e+01 MeV
SI_0201 : N=5.757e+00/sec.  <E>=1.457e+03 MeV
CI_0201 : N=9.863e+00/sec.  <E>=8.247e+01 MeV
CI_0203 : N=1.286e+00/sec.  <E>=8.248e+01 MeV
SI_0203 : N=6.760e-01/sec.  <E>=1.456e+03 MeV
SI_0324 : N=5.157e-01/sec.  <E>=1.449e+03 MeV
</pre>
<p>If you want to see the count rates for a different beam intensity, call</p>
<pre>ecr.PrintResults(5.e+06)</pr>
<p>For each detector hit we fill three histograms:</p>
<pre>
 OBJ: TH1F  SI_0223  Eloss in SI_0223 : 0 at: 0x10898980
 OBJ: TH1F  SI_0223_dW  Solid angle of SI_0223 : 0 at: 0x10899540
 OBJ: TH2F  SI_0223_map Map of SI_0223 : 0 at: 0x10e2e7d0
</pre>
<p>Each histogram can be obtained using</p>
<pre>ecr.GetHistos().FindObject("SI_0223_map")</pre>
<p>The count rate and mean energy loss in each detector can be obtained using</p>
<pre>
kaliveda [7] ecr.GetDetector("SI_0201").count_rate
(double)5.75694360395366100e+00
kaliveda [8] ecr.GetDetector("SI_0201").mean_energy
(double)1.45679832635920388e+03
</pre>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
KVElasticCountRates::KVElasticCountRates(Double_t theta_min, Double_t theta_max, Double_t phi_min, Double_t phi_max):
   fAngularRange(theta_min, theta_max, phi_min, phi_max),
   fBeamDirection(0, 0, 1)
{
   //Default constructor
   fDepth = fTheta = 0;
   fBinE = 500;
   fEnergy = 0;
   fKinematics = 0;
   fTarget = 0;
   fProj = fTarg = 0;
   fIntLayer =  0;
   fMultiLayer = kFALSE;
   fVolume = (theta_max - theta_min) * (phi_max - phi_min) * TMath::DegToRad() * TMath::DegToRad();
   //fVolume = (cos(theta_min*TMath::DegToRad())-cos(theta_max*TMath::DegToRad()))*(phi_max-phi_min)*TMath::DegToRad();
   fExx = 0.;
   if (gMultiDetArray) {
      gMultiDetArray->SetSimMode(kTRUE);
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_GeoThresh);
      gMultiDetArray->InitializeIDTelescopes();
   } else {
      Warning("KVElasticCountRates", "gMultiDetArray does not refer to a valid multidetector array");
      printf("Define it before using this class, and put it in simulation mode : gMultiDetArray->SetSimMode(kTRUE)");
   }
}

//__________________________________________________________________//

KVElasticCountRates::~KVElasticCountRates()
{
   //Destructor
   if (fDepth)
      delete fDepth;
   if (fTheta)
      delete fTheta;
   if (fKinematics)
      delete fKinematics;
   if (fProj)
      delete fProj;
   if (fTarg)
      delete fTarg;
}

//__________________________________________________________________//

void KVElasticCountRates::SetRun(Int_t run)
{
   //Set detector parameters, target, etc. for run
   gMultiDetArray->SetParameters(run);
   gMultiDetArray->InitializeIDTelescopes();
   gMultiDetArray->SetROOTGeometry();
   fTarget = gMultiDetArray->GetTarget();
   fAtomicDensity = fTarget->GetAtomsPerCM2() * 1.e-24; //in barn^-1 units
   fTarget->SetRandomized();
   fIntLayer = 0;
   fMultiLayer = (fTarget->NumberOfLayers() > 1);
}

void KVElasticCountRates::SetProjectile(const Char_t* nuc, Double_t e_sur_a)
{
   // Set projectile and beam energy [MeV/nucleon]
   if (!fProj) fProj = new KVNucleus(nuc, e_sur_a);
   fEnergy = fProj->GetE();
}

//__________________________________________________________________//

void KVElasticCountRates::SetTargetScatteringLayer(const Char_t* name)
{
   //For multilayer targets, use this method to choose in which
   //layer the scattering will take place.
   //If name="", reset any previous choice so that scattering
   //can take place in any layer
   if (!fTarget) {
      cout <<
           "<KVElasticCountRates::SetTargetScatteringLayer> : No target set. Set run first."
           << endl;
      return;
   }
   fIntLayer = fTarget->GetLayerIndex(name);
   if (fIntLayer)
      fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
}

//__________________________________________________________________//

void KVElasticCountRates::SetEbinning(Int_t nbins)
{
   //Set the number of bins of the GetEnergy() histogram
   //Default value is 500; this function has to be called before
   //using CalculateScattering.
   fBinE = nbins;
}
//__________________________________________________________________//

void KVElasticCountRates::CalculateScattering(Int_t N)
{
   //Perform scattering 'N' times for current values
   //of particle Z, A and energy, target excited state, and detector.
   //Print out for each hit detector the mean energy loss & counting rate
   //for a beam intensity of 10**7 pps

   fNtirages = N;

   fHistos.Delete();
   fRates.clear();

   if (!fProj) {
      cout <<
           "<KVElasticCountRates::CalculateScattering> : Set projectile properties first"
           << endl;
      return;
   }
   if (!fEnergy) {
      cout <<
           "<KVElasticCountRates::CalculateScattering> : Set projectile energy first"
           << endl;
      return;
   }
   if (!fTarget) {
      cout <<
           "<KVElasticCountRates::CalculateScattering> : No target set. Set run first."
           << endl;
      return;
   }

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set up histograms

   /* -------------------------------------------------------------------------------------------------------------------------- */

   // -- histograms for debugging: depth in target and angular distribution
   if (fDepth)
      delete fDepth;
   if (fTheta)
      delete fTheta;
   fDepth =
      new TH1F("hDepth", "Depth (mg/cm2)", 500, 0., fTarget->GetTotalEffectiveThickness());
   fTheta = new TH1F("hTheta", "Theta (deg.)", 500, 0., 0.);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set up kinematics
   if (!fKinematics)
      fKinematics = new KV2Body;
   fProj->SetEnergy(fEnergy);
   fProj->SetTheta(0);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set random interaction point for scattering
   if (fIntLayer) {
      fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
   } else {
      fTarget->GetInteractionPoint(fProj);
   }

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //get target nucleus properties from scattering layer
   TVector3 IP = fTarget->GetInteractionPoint();
   KVMaterial* targ_mat = fTarget->GetLayer(IP);
   if (!fTarg) fTarg = new KVNucleus;
   fTarg->SetZ((Int_t) targ_mat->GetZ());
   fTarg->SetA((Int_t) targ_mat->GetMass());
   fKinematics->SetTarget(fTarg);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set excited state of target nucleus - in other words, dissipated energy for
   //reaction due to excitation of target
   fKinematics->SetEDiss(fExx);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   for (int i = 0; i < N; i++) {
      //calculate slowing of incoming projectile
      fTarget->SetIncoming();
      fTarget->DetectParticle(fProj);
      fKinematics->SetProjectile(fProj);
      fKinematics->SetOutgoing(fProj);
      fKinematics->CalculateKinematics();
      //set random direction of outgoing projectile
      fAngularRange.GetRandomAngles(theta, phi, "random");
      xsec = TMath::Abs(fKinematics->GetXSecRuthLab(theta));
      //set energy of scattered nucleus
      //WARNING: for inverse kinematics reactions, their are two energies for
      //each angle below the maximum scattering angle.
      //We only use the highest energy corresponding to the most forward CM angle.
      Double_t e1, e2;
      fKinematics->GetELab(3, theta, 3, e1, e2);
      fProj->SetEnergy(TMath::Max(e1, e2));
      fProj->SetTheta(theta);
      fProj->SetPhi(phi);
      fTheta->Fill(theta, xsec);
      //slowing of outgoing projectile in target
      fTarget->SetOutgoing();
      fTarget->DetectParticle(fProj);
      //now detect particle in array
      KVNameValueList* detectors = gMultiDetArray->DetectParticle(fProj);
      //fill histograms
      fDepth->Fill(IP.z());
      FillHistograms(detectors);
      fProj->GetParameters()->Clear();
      fProj->SetEnergy(fEnergy);
      fProj->SetTheta(0);
      //set random interaction point for scattering
      if (fIntLayer) {
         fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
      } else {
         fTarget->GetInteractionPoint(fProj);
         //if target is multilayer and the interaction layer is not fixed,
         //the layer & hence the target nucleus may change
         if (fMultiLayer) {
            targ_mat = fTarget->GetLayer(fTarget->GetInteractionPoint());
            fTarg->SetZ((Int_t) targ_mat->GetZ());
            fTarg->SetA((Int_t) targ_mat->GetMass());
            fKinematics->SetTarget(fTarg);
            fKinematics->SetOutgoing(fProj);
         }
      }
      IP = fTarget->GetInteractionPoint();
   }

   PrintResults();
}

//__________________________________________________________________//



void KVElasticCountRates::FillHistograms(KVNameValueList* dets)
{
   // parse the list dets
   // fill histograms with energy loss for all detectors
   // clear the detector energy losses
   // delete the list

   if (!dets) return;

   Int_t ndets = dets->GetNpar();
   for (int i = 0; i < ndets; i++) {
      TString detname = dets->GetNameAt(i);
      KVDetector* det = gMultiDetArray->GetDetector(detname);
      if (!det) continue;
      TH1F* histo = (TH1F*)fHistos.FindObject(detname);
      if (!histo) {
         histo = new TH1F(detname, Form("Eloss in %s", detname.Data()), fBinE, 0, 0);
         fHistos.Add(histo);
      }
      double de = dets->GetDoubleValue(i);
      histo->Fill(de, xsec * sin(theta * TMath::DegToRad()));
      histo = (TH1F*)fHistos.FindObject(detname + "_dW");
      if (!histo) {
         histo = new TH1F(detname + "_dW", Form("Solid angle of %s", detname.Data()), fBinE, 0, 0);
         fHistos.Add(histo);
      }
      histo->Fill(de, sin(theta * TMath::DegToRad()));
      TH2F* histo2 = (TH2F*)fHistos.FindObject(detname + "_map");
      if (!histo2) {
         histo2 = new TH2F(detname + "_map", Form("Map of %s", detname.Data()), 100, 0, 0, 100, 0, 0);
         fHistos.Add(histo2);
      }
      histo2->Fill(theta, phi, xsec);
      det->Clear();
   }
   delete dets;
}

struct count_rate {
   TString detector;
   double counts;
   double energy;
   double theta;
   double phi;
   double fluence;
   double dissipation;
   count_rate(TString n, double c, double e, double t, double p, double f, double d)
      : detector(n), counts(c), energy(e), theta(t), phi(p), fluence(f), dissipation(d) {}
   void print()
   {
      printf("%s \t:  N=%8.2f/sec. \t <E>=%7.1f MeV \t fluence=%9.3E/sec./cm**2 \t dissip.=%9.3E MeV/sec./cm**2\n",
             detector.Data(), counts, energy, fluence, dissipation);
   }
};
bool compare_count_rates(count_rate a, count_rate b)
{
   if (TMath::Abs(a.theta - b.theta) < 0.5) return a.phi < b.phi;
   return a.theta < b.theta;
}

void KVElasticCountRates::PrintResults(Double_t beam_intensity)
{
   // Print mean energy deposit & counting rate for given beam intensity in particles per second

   TIter it(&fHistos);
   TH1F* h;
   fRates.clear();

   std::vector<count_rate> count_rates;

   while ((h = (TH1F*)it())) {
      TString name = h->GetName();
      if (!name.EndsWith("_dW") && !name.EndsWith("_map")) {
         TH2F* map = (TH2F*)fHistos.FindObject(name + "_map");
         double rate = h->Integral() * fAtomicDensity * beam_intensity * fVolume / fNtirages;
         double emean = h->GetMean();
         KVDetector* det = gMultiDetArray->GetDetector(name);
         double fluence = rate / det->GetEntranceWindowSurfaceArea();
         double dissipation = emean * rate / det->GetEntranceWindowSurfaceArea();
         count_rates.push_back(
            count_rate(name, rate, emean, map->GetMean(), map->GetMean(2), fluence, dissipation)
         );
         fRates[name.Data()] = KVElasticCountRate(rate, emean, fluence, dissipation);
      }
   }
   std::sort(count_rates.begin(), count_rates.end(), compare_count_rates);

   for (std::vector<count_rate>::iterator it = count_rates.begin(); it != count_rates.end(); ++it) {
      it->print();
   }
}

//__________________________________________________________________//
