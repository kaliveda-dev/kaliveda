//Created by KVClassFactory on Thu May  7 11:02:24 2015
//Author: John Frankland,,,

#include "mdweight.h"
#include "TMath.h"

ClassImp(MicroStat::mdweight)

namespace MicroStat {

   Double_t mdweight::edist(Double_t* arg, Double_t* par)
   {
      // energy distribution of particle in gas
      // arg[0] = energy/available energy
      // par[0] = number of particles in gas
      // par[1] = mass ratio = massTot/(massTot-mPart)
      Double_t e = arg[0];
      Double_t N = par[0];
      Double_t massRat = par[1];

      Double_t val = 0.;
      if ((e > 0) && (e * massRat < 1) && (N > 2)) {
         val = TMath::Sqrt(e) * TMath::Power(1. - massRat * e, (3.*N - 8.) / 2.);
      }
      return val;
   }

   TF1* mdweight::getKEdist(Int_t N, Double_t R)
   {
      // find/create energy distribution for given number of particles
      // N and mass ratio R.

      TString fn = Form("fKE_N%d_R%f", N, R);
      TF1* f = (TF1*)fKEDist.FindObject(fn);
      if (!f) {
         f = new TF1("mdweight::SPKEDist", mdweight::edist, 0, 1, 2);
         f->SetRange(0, 1.);
         f->SetParameters((Double_t)N, R);
         f->SetNpx(100);
         f->SetName(fn);
         fKEDist.Add(f);
      }
      return f;
   }

   double CosThetaDist(double* x, double* par)
   {
      // parameterise angular distribution as function of ratio between
      // P(cos theta=+/-1) and P(cos theta=0)
      // par[0] = a = P(cos theta=+/-1)
      // par[1] = b = P(cos theta=0)
      // x = cosTheta
      if (abs(par[0] - par[1]) < 1.e-6) return par[1];

      double a = par[0];
      double b = par[1];
      double cosTheta = x[0];
      double alpha = TMath::Pi() * cosTheta;
      return (a - b) / 2.*(1. - TMath::Cos(alpha)) + b;
   }

   mdweight::mdweight()
      : fCosTheta("CosTheta", CosThetaDist, -1, 1, 2)
   {
      fKEDist.SetOwner();
      log2pi = TMath::Log(TMath::TwoPi());
      log10twelve = TMath::Log(1e+12);
      eDisp = 0.0;
      massTot = 0.0;
      massTot0 = 0.0;
      px = 0.0;
      py = 0.0;
      pz = 0.0;
      SetAnisotropy(1, 1);
   }

   mdweight::~mdweight()
   {
      // Destructor
   }

   void mdweight::SetWeight(KVEvent* e, Double_t E)
   {
      // Set available energy, E, and calculate statistical weight
      // for this event

      if (E <= 0) {
         setAvailableEnergy(0.);
         setWeight(0.);
         return;
      }
      setAvailableEnergy(E);
      Double_t N = e->GetMult();
      Double_t logmass_sum, mass_sum;
      logmass_sum = mass_sum = 0.;
      KVNucleus* n;
      while ((n = e->GetNextParticle())) {
         Double_t m = n->GetMass();
         logmass_sum += TMath::Log(m);
         mass_sum += m;
      }
      A = (3 * (N - 1) / 2.) * log2pi - TMath::LnGamma(3 * (N - 1) / 2.)
          + 1.5 * (logmass_sum - TMath::Log(mass_sum));
      B = (3 * N - 5) / 2.;

      Double_t w = TMath::Exp((A + B * TMath::Log(E)) - log10twelve);
      setWeight(w);
   }

   void mdweight::initGenerateEvent(KVEvent* partition)
   {
      // Call before generating an event with StatWeight::GenerateEvent
      // using the given partition and available energy

      massTot0 = 0;
#ifdef WITH_CPP11
      for (auto& e : EventIterator(partition)) massTot0 += e.GetMass();
//#else
      for (KVNucleusEvent::Iterator it = EventIterator(partition).begin(); it != KVNucleusEvent::Iterator::End(); ++it) massTot0 += (*it).GetMass();
#endif
      resetGenerateEvent();
   }

   void mdweight::resetGenerateEvent()
   {
      // Called by StatWeight::GenerateEvent before generating another
      // event using the same partition as the last
      eDisp = GetAvailableEnergy();
      massTot = massTot0;
      px = py = pz = 0.0;
   }

   void mdweight::nextparticleGenerateEvent(Int_t N, KVNucleus* part)
   {
      // Called by StatWeight::GenerateEvent when adding a particle to the event
      // N is the number of particles still to add including this one
      //
      // The algorithm was written by Daniel Cussol (LPC Caen, France).

      Double_t mPart = part->GetMass();
      Double_t rR = mPart / massTot;
      Double_t ratio;
      if (rR < 1.) ratio = 1. / (1. - rR);
      else ratio = 1.;
      Double_t ec = 0.; // kinetic energy of particle
      Double_t ppx, ppy, ppz; // components of particle momentum
      ppx = ppy = ppz = 0;
      if (N >= 2) {
         Double_t p = 0.; //momentum to give particle
         if (N > 2) {
            // draw random KE from 1-particle distribution for given N & ratio
            ec = eDisp * getKEdist(N, ratio)->GetRandom();
            p = sqrt(2.*mPart * ec);
         }
         else {
            // last 2 particles: share remaining available energy
            p = sqrt(2.*(massTot - mPart) * mPart * eDisp / massTot);
            ec = p * p / 2. / mPart;
         }
         Double_t ct = fCosTheta.GetRandom(-1, 1); //1. - 2.*gRandom->Rndm();
         Double_t st = TMath::Sqrt(1. - ct * ct);
         Double_t phi = gRandom->Rndm() * 2.*TMath::Pi();
         ppz = ct * p;
         ppx = st * TMath::Cos(phi) * p;
         ppy = st * TMath::Sin(phi) * p;
         ppx += px * rR;
         ppy += py * rR;
         ppz += pz * rR;
      }
      else if (N == 1) {
         ppx = px;
         ppy = py;
         ppz = pz;
         ec = 0;
      }
      part->SetMomentum(ppx, ppy, ppz);

      eDisp -= ec * ratio;
      massTot -= mPart;
      px -= ppx;
      py -= ppy;
      pz -= ppz;
   }

}/*  namespace MicroStat */

/**
  \example MicroStat_example.C
# Calculate 12C*->3-alpha decay & compare alpha KE with exact microcanonical distribution

 Example of use of MicroStat::mdweight class, which can
 generate events according to the molecular dynamics ensemble
 i.e. fixed total energy and zero total momentum.

 By default, 10**5 events are generated for the 3-alpha decay of
 a Carbon-12 nucleus with E*=50 MeV.

 A histogram is filled with the kinetic energy distribution of one of the
 alpha particles, which is then fitted using the exact microcanonical
 probability distribution for a classical gas of 3 equal-mass particles.

 To execute this function do
~~~~
 $ kaliveda
 kaliveda[0] .L MicroStat_example.C+
 kaliveda[1] example()
~~~~

*/
/**
  \example MicroStat_example2.C
# Calculate 12C*->2H+4He+6Li & compare with exact microcanonical distributions

 Example of use of MicroStat::mdweight class, which can
 generate events according to the molecular dynamics ensemble
 i.e. fixed total energy and zero total momentum.

 By default, 10**5 events are generated for the decay of
 a Carbon-12 nucleus with E*=50 MeV.

Histograms are filled with the kinetic energy distributions of the
 three particles, which are then fitted using the exact microcanonical
 probability distribution for a classical gas of 3 unequal-mass particles.

 To execute this function do

 $ kaliveda
 kaliveda[0] .L MicroStat_example2.C+
 kaliveda[1] example()
*/

