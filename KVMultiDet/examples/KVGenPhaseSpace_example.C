#include "KVNucleus.h"
#include "KVSimEvent.h"
#include "KVGenPhaseSpace.h"
#include "TTree.h"

#define NPARTS 3
#define AddArrBrI(type,name) \
   type name[NPARTS];\
   tri->Branch( #name , name , #name "[3]/I" )
#define AddArrBrD(type,name) \
   type name[NPARTS];\
   tri->Branch( #name , name , #name "[3]/D" )

void example(double E0 = 50, int nevents = 100000)
{
   // 12C* -> 3(4He)
   // compound nucleus = carbon
   KVNucleus CN(6, 12);
   CN.SetExcitEnergy(E0);

   // decay products
   KVSimEvent decay;
   KVNucleus* n = decay.AddParticle();
   n->SetZandA(2, 4);
   n = decay.AddParticle();
   n->SetZandA(2, 4);
   n = decay.AddParticle();
   n->SetZandA(2, 4);

   KVGenPhaseSpace gps;
   if (!gps.SetBreakUpChannel(CN, &decay)) {
      printf("Break-up channel is not allowed\n");
      return;
   }

   TFile* out = new TFile("ThreeAlphaDecay.root", "recreate");
   TTree* tri = new TTree("ThreeAlphaDecay", Form("12C(E*=%lf.2MeV) -> 3(4He)", E0));
   Int_t mult;
   Double_t wgt;
   tri->Branch("mult", &mult);
   tri->Branch("wgt", &wgt);
   AddArrBrI(Int_t, Z);
   AddArrBrI(Int_t, A);
   AddArrBrD(Double_t, E);
   AddArrBrD(Double_t, Theta);
   AddArrBrD(Double_t, Phi);


   while (nevents--) {
      wgt = gps.Generate();
      decay.FillArraysEThetaPhi(mult, Z, A, E, Theta, Phi);
      tri->Fill();
   }

   out->Write();
   delete out;
}
