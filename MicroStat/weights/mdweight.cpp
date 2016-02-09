//Created by KVClassFactory on Thu May  7 11:02:24 2015
//Author: John Frankland,,,

#include "mdweight.h"
#include "TMath.h"

ClassImp(MicroStat::mdweight)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>mdweight</h2>
<h4>Calculate molecular dynamics ensemble weights for events</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

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

   mdweight::mdweight()
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

      massTot0 = partition->GetSum("GetMass");
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
         } else {
            // last 2 particles: share remaining available energy
            p = sqrt(2.*(massTot - mPart) * mPart * eDisp / massTot);
            ec = p * p / 2. / mPart;
         }
         Double_t ct = 1. - 2.*gRandom->Rndm();
         Double_t st = TMath::Sqrt(1. - ct * ct);
         Double_t phi = gRandom->Rndm() * 2.*TMath::Pi();
         ppz = ct * p;
         ppx = st * TMath::Cos(phi) * p;
         ppy = st * TMath::Sin(phi) * p;
         ppx += px * rR;
         ppy += py * rR;
         ppz += pz * rR;
      } else if (N == 1) {
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
