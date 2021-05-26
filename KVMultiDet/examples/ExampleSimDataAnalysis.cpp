#include "ExampleSimDataAnalysis.h"
#include "KVSimNucleus.h"
#include "KVBatchSystem.h"

ClassImp(ExampleSimDataAnalysis)

#include "KVSimEvent.h"

void ExampleSimDataAnalysis::InitAnalysis()
{
   // INITIALISATION PERFORMED AT BEGINNING OF ANALYSIS
   // Here you define:
   //   - global variables
   //   - histograms
   //   - trees

   // DEFINITION OF GLOBAL VARIABLES FOR ANALYSIS
   AddGV("KVMult", "mult");   // total multiplicity of each event
   AddGV("KVMult", "Mcha")->SetSelection({"Z>0", [](const KVNucleus * n)
   {
      return n->GetZ() > 0;
   }}); // charged particle multiplicity

   // for sorting events according to multiplicity
   KVEventClassifier* ec = GetGVList()->AddEventClassifier("mult");
   ec->AddCut(5);
   ec->AddCut(10);
   ec->AddCut(15);
   ec->AddCut(20);
   ec->AddCut(25);

   // DEFINITION OF HISTOGRAMS
   for (int EC = 0; EC <= 5; ++EC)
      AddHisto(new TH2F(Form("VparVper_alphas_EC%d", EC),
                        Form("#alpha particle velocities EC=%d", EC),
                        250, -15, 15, 250, -15, 15));

   // DEFINITION OF TREE USED TO STORE RESULTS
   CreateTreeFile();

   TTree* t = new TTree("data", GetOpt("SimulationInfos"));

   // add a branch to tree for each defined global variable
   GetGVList()->MakeBranches(t);

   AddTree(t);
}

//____________________________________________________________________________________

Bool_t ExampleSimDataAnalysis::Analysis()
{
   // EVENT BY EVENT ANALYSIS

   Int_t EC = GetGV("mult_EC")->GetValue(); // event class according to mult

   for (auto& part : SimEventIterator(GetEvent())) {
      if (part.IsIsotope(2, 4)) FillHisto(Form("VparVper_alphas_EC%d", EC),
                                             part.GetVpar(), part.GetVperp());
   }

   GetGVList()->FillBranches();
   FillTree();

   return kTRUE;
}

