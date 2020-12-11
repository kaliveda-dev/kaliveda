//Created by KVClassFactory on Thu May  7 14:35:05 2015
//Author: John Frankland,,,

#include "MCSampler.h"
#include "TRandom.h"

#include <TGraph.h>
#include <TMultiGraph.h>
#include "TPad.h"
#include "TH1.h"

ClassImp(MicroStat::MCSampler)


namespace MicroStat {

   void MCSampler::init()
   {
      // default initialisations
      fWeightList = 0;
      fLastPicked = 0;
      fTheLegend = 0;
      fLegendProbaMin = 0;
      fModifyMasses = kFALSE;
   }

   void MCSampler::initialiseWeightList()
   {
      // set up the TClonesArray

      fWeightList = new TClonesArray(fWeight);
   }

   MCSampler::MCSampler()
   {
      // Default constructor
      init();
   }

   //________________________________________________________________

   MCSampler::MCSampler(const Char_t* name, const Char_t* title) : KVBase(name, title)
   {
      // Write your code here
      init();
   }

   MCSampler::~MCSampler()
   {
      // Destructor

      SafeDelete(fWeightList);
   }

   void MCSampler::SetEventList(TTree* t, const TString& branchname)
   {
      // Define the TTree or TChain containing all possible events (partitions).

      fPartitions = t->GetEntries();
      fBranch = t->GetBranch(branchname);
      if (!fBranch) {
         Error("SetEventList", "cannot find branch %s", branchname.Data());
      }
      fPartition = 0;
      fBranch->SetAddress(&fPartition);
   }

   void MCSampler::SetStatWeight(const TString& w)
   {
      // Set the kind of statistical weight to be used
      // This is the name of a class derived from MicroStat::StatWeight

      fWeight = TClass::GetClass(w);
      if (!fWeight) {
         Error("SetStatWeight", "class %s not found", w.Data());
      }
   }

   void MCSampler::UpdateMasses()
   {
      // if nuclear masses are modified, we have to update those in the
      // partition read from file
      KVNucleus* n;
      while ((n = fPartition->GetNextParticle())) n->SetA(n->GetA());
   }

   void MCSampler::CalculateWeights(Double_t excitation_energy)
   {
      // calculate weights of all partitions for the given excitation energy
      // (in MeV) of the initial compound nucleus

      //Info("CalculateWeights","Calculating channel weights for E*=%f",excitation_energy);

      if (!fWeightList) initialiseWeightList();

      fSumWeights = 0;

      for (int i = 0; i < fPartitions; i++) {
         StatWeight* w = (StatWeight*)fWeightList->ConstructedAt(i);
         GetPartition(i);
         if (fModifyMasses) UpdateMasses();
         w->SetWeight(fPartition, excitation_energy + fPartition->GetChannelQValue());
         w->SetIndex(i);
         fSumWeights += w->GetWeight();

         //std::cout << i << "   Q=" << fPartition->GetChannelQValue() << "   weight=" << w->GetWeight() << endl;
      }

      // sort weights in decreasing order
      fWeightList->Sort();
   }

   Long64_t MCSampler::PickRandomChannel()
   {
      // Return the TTree index of a random channel according to the
      // previously calculated weights
      // The corresponding weight for the chosen channel can be retrieved
      // by calling GetRandomChannelWeight()
      // If no channel is open (i.e. all weights = 0, E* < Q value of first channel),
      // we return -1.

      Double_t x = gRandom->Uniform(fSumWeights);

      int i;
      for (i = 0; i < fPartitions; i++) {
         Double_t w = ((StatWeight*)(*fWeightList)[i])->GetWeight();
         if (x < w) break;
         x -= w;
      }
      if (i == fPartitions) {
         fLastPicked = nullptr;
         return -1;
      }
      fLastPicked = (StatWeight*)(*fWeightList)[i];
      return fLastPicked->GetIndex();
   }

   void MCSampler::SetBranch(TTree* theTree, const TString& bname, void* variable, const TString& vartype)
   {
      TString leaflist = bname + "/";
      leaflist += vartype;
      TBranch* b = theTree->GetBranch(bname);
      if (!b) theTree->Branch(bname, variable, leaflist);
      else b->SetAddress(variable);
   }

   void MCSampler::SetUpTreeBranches(KVEvent*& event, TTree* theTree, const TString& bname)
   {
      // If branch 'bname' does not exist in 'theTree', it will be created and linked to
      // 'event' which must contain the address of a valid KVEvent object.
      // If branch already exists, we link it to 'event'.
      //
      // Branches will also be created/filled with the following information:
      //    ESTAR/D   the excitation energy (Exx)
      //    EDISP/D   the available kinetic energy
      //    IPART/I   the partition index
      TBranch* b = theTree->GetBranch(bname);
      if (!b) {
         theTree->Branch(bname, "KVEvent", event, 10000000, 0)->SetAutoDelete(kFALSE);
      }
      else {
         b->SetAddress(event);
      }
      SetBranch(theTree, "ESTAR", &ESTAR, "D");
      SetBranch(theTree, "EDISP", &EDISP, "D");
      SetBranch(theTree, "IPART", &IPART, "L");
   }

   void MCSampler::GenerateEvents(TTree* theTree, KVEvent* event, Double_t Exx, Long64_t npartitions, Long64_t nev_part)
   {
      // Generate (npartitions*nev_part) events for a given value of excitation energy 'Exx' [MeV].
      // For efficiency, for each chosen partition we generate nev_part events.
      // This means :
      //    - calculating the statistical weight of all available decay channels/partitions
      //    - picking a channel at random
      //    - generating momenta of all nuclei in chosen channel
      //    - filling the TTree with the new event

      Info("GenerateEvents", "Generating events for E*=%f", Exx);

      if (!SetExcitationEnergy(Exx)) {
         Error("GenerateEvents", "Excitation energy is too low, no channels are open");
         return;
      }

      // generate events
      while (npartitions--) {

         SetDecayChannel();


         for (Long64_t iev = 0; iev < nev_part; iev++) {

            // generate nev_part events for chosen partition

            GenerateEvent(theTree, event);

         }

      }

   }

   Bool_t MCSampler::SetExcitationEnergy(Double_t Exx)
   {
      // Define excitation energy for random event generation
      // This will calculate the channel weights for all partitions
      // If sumOfWeights = 0 i.e. no channels are open, return kFALSE

      ESTAR = Exx;
      CalculateWeights(Exx);
      if (fSumWeights == 0) return kFALSE;
      return kTRUE;
   }
   Bool_t MCSampler::SetDecayChannel()
   {
      // Pick a random decay channel with excitation energy given to
      // previous call to SetExcitationEnergy method.
      // Returns kFALSE if no channel is open.

      IPART = PickRandomChannel();
      if (IPART < 0) return kFALSE;

      GetPartition(IPART);

      fLastPicked->initGenerateEvent(fPartition);
      EDISP = fLastPicked->GetAvailableEnergy();

      return kTRUE;

   }
   void MCSampler::GenerateEvent(TTree* theTree, KVEvent* event)
   {
      // Generate 1 kinematical event for the partition previously
      // picked at random after calling:
      //       SetExcitationEnergy(E*);
      //       SetDecayChannel();
      // theTree->Fill() is automatically called.
      // This method can be called many times before either
      //  1) picking a new decay channel
      //  2) changing the energy

      fLastPicked->GenerateEvent(fPartition, event);

      theTree->Fill();

      fLastPicked->resetGenerateEvent();

   }

   void MCSampler::PlotProbabilities(double emin, double emax, double estep, Option_t* opt)
   {
      // Plot probability of each channel as a function of E* (default)
      // or E*/A (opt="E*/A")
      //
      // If SetLegendProbaMin has previously been called with a >0 value
      // for the minimum probability, we generate a TLegend which contains
      // only the channels whose probability reaches at least this minimum
      // value in the given energy range. Call ShowLegend immediately
      // after this method in order to display in current pad.

      Bool_t makeLegend = (fLegendProbaMin > 0.0);

      TMultiGraph* mg = new TMultiGraph();
      TGraph* g[fPartitions];
      Long64_t i = 0;
      int mark = 20;
      Double_t fac = 1.;

      for (; i < fPartitions; i++) {
         KVEvent* pt = GetPartition(i);
         if (!i && !strcmp(opt, "E*/A")) fac = pt->GetSum("GetA");
         g[i] = new TGraph();
         g[i]->SetName(pt->GetPartitionName());
         g[i]->SetMarkerStyle(mark);
         g[i]->SetMarkerColor((i % 9) + 1);
         g[i]->SetLineColor((i % 9) + 1);
         g[i]->SetFillColor(0);
         g[i]->ResetBit(BIT(20));
         mg->Add(g[i], "pl");

         if (!(i % 9)) {
            mark++;
            if (mark > 30) mark = 20;
         }

      }
      // build TGraph
      for (double E = emin; E <= emax; E += estep) {

         CalculateWeights(E * fac);
         for (i = 0; i < fPartitions; i++) {

            if (GetSumWeights() > 0.0) {
               StatWeight* wt = GetWeight(i);

               Double_t proba = wt->GetWeight() / GetSumWeights();
               Long64_t voie = wt->GetIndex();

               g[voie]->SetPoint(g[voie]->GetN(), E, proba * 100.);
               // if we are building a TLegend, we 'mark' each TGraph which
               // contains probabilities higher than the required minimum
               // by setting a bit in the TObject status bitfield
               if (makeLegend && proba > fLegendProbaMin) g[voie]->SetBit(BIT(20));
            }
         }
      }
      if (gPad) gPad->Clear();
      mg->Draw("apl");
      if (!strcmp(opt, "E*/A")) {
         mg->GetHistogram()->SetXTitle("E*/A (MeV)");
      }
      else {
         mg->GetHistogram()->SetXTitle("E* (MeV)");
      }
      mg->GetHistogram()->SetYTitle("Probability");
      if (makeLegend) {
         mg->GetHistogram()->SetAxisRange(fLegendProbaMin * 100, 100., "Y");
         // now add all sufficiently probable decay channels to the TLegend
         fTheLegend = new TLegend(.7, 0.12, .88, .88);
         fTheLegend->SetHeader(Form("Channels with P>%4.1f%%", fLegendProbaMin * 100));
         for (i = 0; i < mg->GetListOfGraphs()->GetSize(); i++) {
            if (g[i]->TestBit(BIT(20))) {
               fTheLegend->AddEntry(g[i], g[i]->GetName(), "pl");
            }
         }
      }
      gPad->Modified();
      gPad->Update();

   }

   void MCSampler::PlotMultiplicities(double emin, double emax, double estep, Option_t* opt)
   {
      // Plot multiplicities of n,p,d,t,3He,4He,... as a function of E* (default)
      // or E*/A (opt="E*/A")
      //

      TMultiGraph* mg = new TMultiGraph();
      const int nparticles = 7;
      TGraph* g[nparticles];

      TString particles[] = {"1n", "1H", "2H", "3H", "3He", "4He", "Z>2"};

      int mark = 20;
      Double_t fac = 1.;

      if (!strcmp(opt, "E*/A")) {
         KVEvent* pt = GetPartition(0);
         fac = pt->GetSum("GetA");
      }

      for (int i = 0; i < nparticles; i++) {
         g[i] = new TGraph();
         g[i]->SetName(particles[i]);
         g[i]->SetMarkerStyle(mark);
         g[i]->SetMarkerColor((i % 9) + 1);
         g[i]->SetLineColor((i % 9) + 1);
         g[i]->SetFillColor(0);
         g[i]->ResetBit(BIT(20));
         mg->Add(g[i], "pl");

         if (!(i % 9)) {
            mark++;
            if (mark > 30) mark = 20;
         }

      }
      // build TGraph
      for (double E = emin; E <= emax; E += estep) {

         CalculateWeights(E * fac);
         Double_t multiplicities[nparticles];
         memset(multiplicities, 0, sizeof(double)*nparticles);

         for (Long64_t i = 0; i < fPartitions; i++) {

            if (GetSumWeights() > 0.0) {
               StatWeight* wt = GetWeight(i);

               Double_t proba = wt->GetWeight() / GetSumWeights();
               if (proba > 1.e-06) {
                  KVEvent* pt = GetPartition(wt->GetIndex());
                  KVNucleus* n;
                  while ((n = pt->GetNextParticle())) {
                     for (int j = 0; j < nparticles - 1; j++) {
                        if (!strcmp(n->GetSymbol(), particles[j].Data())) multiplicities[j] += proba;
                     }
                     if (n->GetZ() > 2) multiplicities[nparticles - 1] += proba;
                  }
               }

            }
         }
         for (int i = 0; i < nparticles; i++) {
            g[i]->SetPoint(g[i]->GetN(), E, multiplicities[i]);
         }
      }
      if (gPad) gPad->Clear();
      mg->Draw("apl");
      if (!strcmp(opt, "E*/A")) {
         mg->GetHistogram()->SetXTitle("E*/A (MeV)");
      }
      else {
         mg->GetHistogram()->SetXTitle("E* (MeV)");
      }
      mg->GetHistogram()->SetYTitle("Multiplicity");
      gPad->Modified();
      gPad->Update();

   }

}

