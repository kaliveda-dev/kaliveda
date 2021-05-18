#include "KVNucleus.h"
#include "KVTemplateEvent.h"
#include "mdweight.h"
#include "TH1F.h"
#include "TF1.h"
#include "TStyle.h"
#include "TStopwatch.h"

double edist(double* x, double* par)
{
   // microcanonical energy distribution for gas of identical
   // (same mass) particles.
   //
   // p0 = normalisation
   // p1 = Etot
   // p2 = number of particles

   double E = x[0];
   double k = par[2] / (par[2] - 1.);
   if (E > 0 && E < par[1] / k) {
      double w = par[0] * pow(E / pow(par[1], 3), 0.5);
      w *= pow((1. - k * E / par[1]), (3.*par[2] - 8.) / 2.);
      return w;
   }
   return 0.;
}

void example(double E0 = 50, int nevents = 100000)
{
   TStopwatch timer;

   // 12C* -> 3(4He)
   // compound nucleus = carbon
   KVNucleus CN(6, 12);
   CN.SetExcitEnergy(E0);

   // decay products
   KVNucleusEvent decay;
   KVNucleus* n = decay.AddParticle();
   n->SetZandA(2, 4);
   n = decay.AddParticle();
   n->SetZandA(2, 4);
   n = decay.AddParticle();
   n->SetZandA(2, 4);

   MicroStat::mdweight gps;
   Double_t etot = E0 + decay.GetChannelQValue();

   if (etot <= 0) {
      printf("Break-up channel is not allowed\n");
      return;
   }
   gps.SetWeight(&decay, etot);
   gps.initGenerateEvent(&decay);

   TH1F* h1 = new TH1F("h1", "Kinetic energy of alpha particle 3", 200, 0, etot * 2. / 3.);
   h1->Sumw2();

   KVNucleusEvent event;

   while (nevents--) {
      gps.GenerateEvent(&decay, &event);
      h1->Fill(event.GetParticle(3)->GetKE());
      gps.resetGenerateEvent();
   }

   h1->Draw();
   TF1* EDis = new TF1("EDis", edist, 0., etot, 3);
   EDis->SetNpx(500);
   EDis->SetParLimits(0, 0, 1.e+08);
   EDis->SetParLimits(1, 0, 2 * etot);
   EDis->FixParameter(2, 3);
   gStyle->SetOptFit(1);
   h1->Fit(EDis, "EM");

   timer.Print();
}
