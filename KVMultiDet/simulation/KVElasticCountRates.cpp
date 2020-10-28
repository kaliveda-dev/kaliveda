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
#include "KVDatime.h"

#include <TH2F.h>
#include <vector>
#include <algorithm>

using namespace std;

ClassImp(KVElasticCountRate)
ClassImp(KVElasticCountRates)

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
   fIntLayer =  0;
   fMultiLayer = kFALSE;
   fVolume = (theta_max - theta_min) * (phi_max - phi_min) * TMath::DegToRad() * TMath::DegToRad();
   //fVolume = (cos(theta_min*TMath::DegToRad())-cos(theta_max*TMath::DegToRad()))*(phi_max-phi_min)*TMath::DegToRad();
   fExx = 0.;
   if (gMultiDetArray) {
      gMultiDetArray->SetSimMode(kTRUE);
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_GeoThresh);
      gMultiDetArray->InitializeIDTelescopes();
   }
   else {
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
   fProj = KVNucleus(nuc, e_sur_a);
   fEnergy = fProj.GetE();
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

   if (!fProj.IsDefined()) {
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
   fProj.SetEnergy(fEnergy);
   fProj.SetTheta(0);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set random interaction point for scattering
   if (fIntLayer) {
      fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
   }
   else {
      fTarget->GetInteractionPoint(&fProj);
   }

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //get target nucleus properties from scattering layer
   TVector3 IP = fTarget->GetInteractionPoint();
   KVMaterial* targ_mat = fTarget->GetLayer(IP);
   KVNucleus t;
   t.SetZandA((Int_t) targ_mat->GetZ(), (Int_t) targ_mat->GetMass());
   fKinematics->SetTarget(t);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set excited state of target nucleus - in other words, dissipated energy for
   //reaction due to excitation of target
   fKinematics->SetEDiss(fExx);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   for (int i = 0; i < N; i++) {
      //calculate slowing of incoming projectile
      fTarget->SetIncoming();
      fTarget->DetectParticle(&fProj);
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
      fProj.SetEnergy(TMath::Max(e1, e2));
      fProj.SetTheta(theta);
      fProj.SetPhi(phi);
      fTheta->Fill(theta, xsec);
      //slowing of outgoing projectile in target
      fTarget->SetOutgoing();
      fTarget->DetectParticle(&fProj);
      //now detect particle in array
      KVNameValueList* detectors = gMultiDetArray->DetectParticle(&fProj);
      //fill histograms
      fDepth->Fill(IP.z());
      FillHistograms(detectors);
      fProj.GetParameters()->Clear();
      fProj.SetEnergy(fEnergy);
      fProj.SetTheta(0);
      //set random interaction point for scattering
      if (fIntLayer) {
         fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
      }
      else {
         fTarget->GetInteractionPoint(&fProj);
         //if target is multilayer and the interaction layer is not fixed,
         //the layer & hence the target nucleus may change
         if (fMultiLayer) {
            targ_mat = fTarget->GetLayer(fTarget->GetInteractionPoint());
            KVNucleus t;
            t.SetZandA((Int_t) targ_mat->GetZ(), (Int_t) targ_mat->GetMass());
            fKinematics->SetTarget(t);
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

/**
  \struct count_rate
  \brief Utility class used by KVElasticCountRates
  \ingroup Simulation
 */
struct count_rate {
   TString detector;
   double counts;
   double energy;
   double theta;
   double phi;
   double fluence;
   double dissipation;
   double intXsec;
   count_rate(TString n, double c, double e, double t, double p, double f, double d, double i)
      : detector(n), counts(c), energy(e), theta(t), phi(p), fluence(f), dissipation(d), intXsec(i) {}
   void print()
   {
      printf("%s \t:  N=%8.2f/sec. \t <E>=%7.1f MeV \t Tot.Xsec=%9.3E barn \t fluence=%9.3E/sec./cm**2 \t dissip.=%9.3E MeV/sec./cm**2\n",
             detector.Data(), counts, energy, intXsec, fluence, dissipation);
   }
   void PutInList(KVNameValueList& nl)
   {
      if (nl.HasParameter("DetList")) {
         KVString tmp = nl.GetStringValue("DetList");
         tmp += ",";
         tmp += detector;
         nl.SetValue("DetList", tmp.Data());
      }
      else {
         nl.SetValue("DetList", detector.Data());
      }
      nl.SetValue(Form("%s.Counts(/sec)", detector.Data()), Form("%8.2f", counts));

      nl.SetValue(Form("%s.Counts(/sec)", detector.Data()), Form("%8.2f", counts));
      nl.SetValue(Form("%s.Emean(MeV)", detector.Data()), Form("%7.1f", energy));
      nl.SetValue(Form("%s.TotXsec(barn)", detector.Data()), Form("%9.3E", intXsec));
      nl.SetValue(Form("%s.Fluence(/sec./cm**2)", detector.Data()), Form("%9.3E", fluence));
      nl.SetValue(Form("%s.dissip(MeV/sec./cm**2)", detector.Data()), Form("%9.3E", dissipation));
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
         // integrated cross-section
         double intXsec = h->Integral() * fVolume / fNtirages;
         // counting rate
         double rate = fAtomicDensity * beam_intensity * intXsec;
         // mean energy
         double emean = h->GetMean();
         KVDetector* det = gMultiDetArray->GetDetector(name);
         double fluence = rate / det->GetEntranceWindowSurfaceArea();
         double dissipation = emean * rate / det->GetEntranceWindowSurfaceArea();
         count_rates.push_back(
            count_rate(name, rate, emean, map->GetMean(), map->GetMean(2), fluence, dissipation, intXsec)
         );
         fRates[name.Data()] = KVElasticCountRate(rate, emean, intXsec, fluence, dissipation);
      }
   }
   std::sort(count_rates.begin(), count_rates.end(), compare_count_rates);

   for (std::vector<count_rate>::iterator it = count_rates.begin(); it != count_rates.end(); ++it) {
      it->print();
   }
}

KVNameValueList KVElasticCountRates::PutResultsInList(Double_t beam_intensity)
{
   // Print mean energy deposit & counting rate for given beam intensity in particles per second
   KVNameValueList nl;
   nl.SetName("Generated by KVElasticCountRates::PutResultsInList method");
   KVDatime dt;
   nl.SetTitle(dt.AsSQLString());
   TIter it(&fHistos);
   TH1F* h;
   fRates.clear();
   nl.SetValue("Intensity(pps)", beam_intensity);

   std::vector<count_rate> count_rates;

   while ((h = (TH1F*)it())) {
      TString name = h->GetName();
      if (!name.EndsWith("_dW") && !name.EndsWith("_map")) {
         TH2F* map = (TH2F*)fHistos.FindObject(name + "_map");
         // integrated cross-section
         double intXsec = h->Integral() * fVolume / fNtirages;
         // counting rate
         double rate = fAtomicDensity * beam_intensity * intXsec;
         // mean energy
         double emean = h->GetMean();
         KVDetector* det = gMultiDetArray->GetDetector(name);
         double fluence = rate / det->GetEntranceWindowSurfaceArea();
         double dissipation = emean * rate / det->GetEntranceWindowSurfaceArea();
         count_rates.push_back(
            count_rate(name, rate, emean, map->GetMean(), map->GetMean(2), fluence, dissipation, intXsec)
         );
         fRates[name.Data()] = KVElasticCountRate(rate, emean, intXsec, fluence, dissipation);
      }
   }
   std::sort(count_rates.begin(), count_rates.end(), compare_count_rates);

   for (std::vector<count_rate>::iterator it = count_rates.begin(); it != count_rates.end(); ++it) {
      it->PutInList(nl);
   }
   return nl;
}

//__________________________________________________________________//
