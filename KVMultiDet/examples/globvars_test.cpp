/** \example globvars_test.cpp
   Demonstrate use of global variables and check correct function
   */

#include <KVEvent.h>
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
   KVEvent test_event;

   // global variable list
   KVGVList globVars;
   globVars.SetOwner(); // let list delete variables after use

   // add global variables to list
   globVars.AddGV("KVEkin", "Etot"); // total energy lab frame
   globVars.AddGV("KVEkin", "Etot_CM")->SetFrame("CM"); // total energy CM frame
   globVars.AddGV("KVEtransLCP", "Et12"); // total transverse energy LCP
   globVars.AddGV("KVMult", "Mult"); // total multiplicity
   globVars.AddGV("KVMultAv", "Mult_av_CM"); // multiplicity forward CM
   globVars.AddGV("KVMultIMF", "MultIMF"); // mult Z>=3
   globVars.AddGV("KVMultIMFAv", "MultIMF_av_CM"); // mult Z>=3 forward CM
   globVars.AddGV("KVMultLeg", "MultLCP"); // mult Z<=2
   globVars.AddGV("KVMultLegAv", "MultLCP_av_CM"); // mult Z<=2 forward CM
   KVVarGlob* vg = globVars.AddGV("KVMultLeg", "MultLCP_back_CM"); // mult Z<=2 backwards CM
   vg->SetFrame("CM");
   vg->AddSelection("_NUC_->GetVpar()<0"); // 'adds' selection to existing "_NUC_->GetZ()<=2" selection

   // initialize all variables
   globVars.Init();

   // illustrate automatic TTree branch creation
   TTree test_tree;
   globVars.MakeBranches(&test_tree);
   test_tree.GetListOfBranches()->ls();

   // calculate all variables for 1 event
   generate_event(test_event);
   globVars.CalculateGlobalVariables(&test_event);

   // fill values in TTree
   globVars.FillBranches();
   test_tree.Fill();

   globVars.Print();
   test_tree.Scan("*");
}
