void SimulatedEventAnalysisTemplate::InitAnalysis()
{
   // INITIALISATION PERFORMED AT BEGINNING OF ANALYSIS
   // Here you define:
   //   - global variables
   //   - histograms
   //   - trees

   // DEFINITION OF GLOBAL VARIABLES FOR ANALYSIS

   // charged particle multiplicity - unfiltered simulations may include neutrons!
   KVVarGlob* v = AddGV("KVMult", "Mcha");
#ifdef USING_ROOT6
   v->SetSelection({"Z>0", [](const KVNucleus * n)
   {
      return n->GetZ() > 0;
   }));
#else
   v->SetSelection("_NUC_->GetZ()>0");
#endif

   ZMAX = (KVZmax*)AddGV("KVZmax", "ZMAX");//fragments sorted by Z

   // DEFINITION OF TREE USED TO STORE RESULTS
   CreateTreeFile();

   TTree* t = new TTree("data", GetOpt("SimulationInfos"));

   // add a branch to tree for each defined global variable
   GetGVList()->MakeBranches(t);

   // add branches to be filled by user
   t->Branch("mult", &mult);
   t->Branch("Z", Z, "Z[mult]/I");
   t->Branch("A", A, "A[mult]/I");
   t->Branch("Vper", Vper, "Vper[mult]/D");
   t->Branch("Vpar", Vpar, "Vpar[mult]/D");
   t->Branch("E", E, "E[mult]/D");
   t->Branch("Theta", Theta, "Theta[mult]/D");
   t->Branch("Phi", Phi, "Phi[mult]/D");

   AddTree(t);

   }

//____________________________________________________________________________________

   Bool_t SimulatedEventAnalysisTemplate::Analysis()
   {
      // EVENT BY EVENT ANALYSIS

      mult = GetEvent()->GetMult();

      for (int i = 0; i < mult; i++) {
         KVSimNucleus* part = (KVSimNucleus*)ZMAX->GetZmax(i);
         Z[i] = part->GetZ();
         A[i] = part->GetA();
         Vper[i] = part->GetVperp();
         Vpar[i] = part->GetVpar();
         E[i] = part->GetEnergy();
         Theta[i] = part->GetTheta();
         Phi[i] = part->GetPhi();
      }

      GetGVList()->FillBranches();
      FillTree();

      return kTRUE;
   }

