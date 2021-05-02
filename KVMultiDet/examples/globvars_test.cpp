/** \example globvars_test.cpp
   Demonstrate use of global variables and check correct function
   */

#include <KVTemplateEvent.h>
#include <KVGVList.h>
#include <KVPosition.h>
#include <iostream>
using namespace std;

void generate_event(KVEvent& e)
{
   // Make example event for demonstration purposes
   //  - 5 protons & 3 20Ne fragments in forward "CM" hemisphere
   //  - 4 4He particles in backward "CM" hemisphere

   e.Clear();
   KVPosition ranVec(0, 90, 0, 360);
   // protons - E=50MeV, theta=60 => Etrans=50*sin^2(60)
   for (int i = 1; i <= 5; ++i) {
      KVNucleus* nuc = e.AddParticle();
      nuc->SetZandA(1, 1);
      nuc->SetE(50);
      nuc->SetTheta(60);
      nuc->SetPhi(gRandom->Uniform(360));
   }
   // neons - E=250MeV, theta=30 => Etrans=250*sin^2(30)
   for (int i = 1; i <= 3; ++i) {
      KVNucleus* nuc = e.AddParticle();
      nuc->SetZandA(10, 20);
      nuc->SetE(250);
      nuc->SetTheta(30);
      nuc->SetPhi(gRandom->Uniform(360));
   }
   // alphas - E=100MeV, theta=120 => Etrans=100*sin^2(120)
   ranVec.SetPolarMinMax(90, 180);
   for (int i = 1; i <= 4; ++i) {
      KVNucleus* nuc = e.AddParticle();
      nuc->SetZandA(2, 4);
      nuc->SetE(100);
      nuc->SetTheta(120);
      nuc->SetPhi(gRandom->Uniform(360));
   }

   // boost to labo
   e.SetFrameName("CM");
   e.SetFrame("lab", TVector3(0, 0, -4));
   e.ChangeDefaultFrame("lab");

   cout << "Total energy in lab frame = " << e.GetSum("GetE") << endl;
}

void globvars_test()
{
   // set up event for test
   KVNucleusEvent test_event;

   // global variable list
   KVGVList globVars;
   globVars.SetOwner(); // let list delete variables after use

   // add global variables to list
   globVars.AddGV("KVEkin", "Etot"); // total energy lab frame
   globVars.AddGV("KVEkin", "Etot_CM")->SetFrame("CM"); // total energy CM frame
   globVars.AddGV("KVEtransLCP", "Et12")->SetFrame("CM"); // total transverse energy LCP
   globVars.AddGV("KVMult", "Mult"); // total multiplicity
   globVars.AddGV("KVZmean", "MeanZ");
   globVars.AddGV("KVDirectivity", "Directo"); // FOPI directivity variable

#ifdef USING_ROOT6
   KVParticleCondition av_CM({"av_CM", [](const KVNucleus * n)
   {
      return n->GetVpar() > 0;
   }
                             });
#else
   KVParticleCondition av_CM("_NUC_->GetVpar()>0");
#endif

   KVVarGlob* vg = globVars.AddGV("KVMult", "Mult_av_CM"); // multiplicity forward CM
   vg->SetFrame("CM");
   vg->SetSelection(av_CM);

   globVars.AddGV("KVMultIMF", "MultIMF"); // mult Z>=3
   vg = globVars.AddGV("KVMultIMF", "MultIMF_av_CM"); // mult Z>=3 forward CM
   vg->SetFrame("CM");
   vg->AddSelection(av_CM);  // 'adds' selection to existing "nuc->GetZ()<=2" selection

   globVars.AddGV("KVMultLeg", "MultLCP"); // mult Z<=2
   vg = globVars.AddGV("KVMultLeg", "MultLCP_av_CM"); // mult Z<=2 forward CM
   vg->SetFrame("CM");
   vg->AddSelection(av_CM);  // 'adds' selection to existing "nuc->GetZ()<=2" selection

   vg = globVars.AddGV("KVMultLeg", "MultLCP_back_CM"); // mult Z<=2 backwards CM
   vg->SetFrame("CM");
#ifdef USING_ROOT6
   // 'adds' selection to existing "nuc->GetZ()<=2" selection
   vg->AddSelection({"Vpar<0", [](const KVNucleus * nuc)
   {
      return nuc->GetVpar() < 0;
   }});
#else
   vg->AddSelection("_NUC_->GetVpar()<0"); // 'adds' selection to existing "_NUC_->GetZ()<=2" selection
#endif
   // initialize all variables
   globVars.Init();

   // illustrate automatic TTree branch creation
   TFile f("globvars_test.root", "recreate");
   TTree* test_tree = new TTree("test_tree", "globvars_test results");
   globVars.MakeBranches(test_tree);
   test_tree->GetListOfBranches()->ls();

   // calculate all variables for 1 event
   generate_event(test_event);

   globVars.CalculateGlobalVariables(&test_event);
   // fill values in TTree
   globVars.FillBranches();
   test_tree->Fill();
   f.Write();

   test_tree->Scan("*");

   cout << "Et12 should be " << 5 * 50 * pow(TMath::Sin(60 * TMath::DegToRad()), 2) + 4 * 100 * pow(TMath::Sin(120 * TMath::DegToRad()), 2) << " MeV" << endl;

#ifdef USING_ROOT6
   for (auto varg : globVars) {
#else
   TObject* varg;
   TIter it(&globVars);
   while ((varg = it())) {
#endif
      std::cout << varg->GetName() << " = " << ((KVVarGlob*)varg)->GetValue() << std::endl;
   }
}
