#include "KVNucleus.h"
#include "KVTemplateEvent.h"
#include "mdweight.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TStopwatch.h"
#include "TCanvas.h"

double edist(double* x, double* par)
{
   // microcanonical energy distribution for gas of non-identical
   // (different mass) particles.
   //
   // p0 = normalisation
   // p1 = Etot
   // p2 = number of particles
   // p3 = total mass / (total mass - mass of particle)

   double E = x[0];
   if (E > 0 && E * par[3] < par[1]) {
      double w = par[0] * pow(E / pow(par[1], 3), 0.5);
      w *= pow((1. - par[3] * E / par[1]), (3.*par[2] - 8.) / 2.);
      return w;
   }
   return 0.;
}

TF1* EDis = nullptr;

void FitEDist(TH1F* histo, Double_t etot, Int_t Npart, Double_t Mtot, Double_t Mpart)
{
   if (!EDis) EDis = new TF1("EDis", edist, 0., etot, 4);
   EDis->SetNpx(500);
   EDis->SetParLimits(0, 0, 1.e+08);
   EDis->SetParLimits(1, 0, 2 * etot);
   EDis->FixParameter(2, Npart);
   EDis->FixParameter(3, Mtot / (Mtot - Mpart));
   gStyle->SetOptFit(1);
   histo->Fit(EDis, "EM");
}

void example(double E0 = 50, int nevents = 100000)
{
   TStopwatch timer;

   // compound nucleus = carbon
   KVNucleus CN(6, 12);
   CN.SetExcitEnergy(E0);

   // decay products
   KVNucleusEvent decay;
   KVNucleus* n = decay.AddParticle();
   n->SetZandA(1, 2);
   n = decay.AddParticle();
   n->SetZandA(2, 4);
   n = decay.AddParticle();
   n->SetZandA(3, 6);

   MicroStat::mdweight gps;
   Double_t etot = E0 + decay.GetChannelQValue();
   Double_t total_mass = decay.GetSum("GetMass");

   if (etot <= 0) {
      printf("Break-up channel is not allowed\n");
      return;
   }
   gps.SetWeight(&decay, etot);
   gps.initGenerateEvent(&decay);

   std::cout << "Edisp = " << etot << " MeV" << std::endl;
   KVHashList histos;
   TH1F* h;

   while ((n = decay.GetNextParticle())) {
      Double_t kappa = total_mass / (total_mass - n->GetMass());
      std::cout << n->GetSymbol() << " : max KE = " << 1. / kappa << " * " << etot << " MeV" << std::endl;
      std::cout << n->GetSymbol() << " : m/M = " << n->GetMass() / total_mass << " k = " << kappa << std::endl;
      histos.Add(h = new TH1F(n->GetSymbol(), Form("Kinetic energy of %s", n->GetSymbol()), 200, 0, etot));
      h->Sumw2();
   }
   KVNucleusEvent event;

   while (nevents--) {
      gps.GenerateEvent(&decay, &event);
      while ((n = event.GetNextParticle()))((TH1F*)histos.FindObject(n->GetSymbol()))->Fill(n->GetE());
      gps.resetGenerateEvent();
   }

   TIter it(&histos);

   while ((h = (TH1F*)it())) {

      KVNucleus part(h->GetName());
      new TCanvas;
      FitEDist(h, etot, decay.GetMult(), total_mass, part.GetMass());

   }

   timer.Print();
}
