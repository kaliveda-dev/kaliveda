//Created by KVClassFactory on Tue Mar 27 21:25:05 2018
//Author: John Frankland,,,

#include "ExampleFilteredSimDataAnalysis.h"
#include "KVReconstructedNucleus.h"
#include "KVBatchSystem.h"

ClassImp(ExampleFilteredSimDataAnalysis)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>ExampleFilteredSimDataAnalysis</h2>
<h4>Analysis of filtered simulated events</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void ExampleFilteredSimDataAnalysis::InitAnalysis()
{
   // INITIALISATION PERFORMED AT BEGINNING OF ANALYSIS
   // Here you define:
   //   - global variables
   //   - histograms
   //   - trees
   //
   // NB: no access to multidetector array or reaction
   //     kinematics yet: see InitRun()

   // DEFINITION OF GLOBAL VARIABLES FOR ANALYSIS

   // charged particle multiplicity
   KVVarGlob* v = AddGV("KVVGSum", "Mcha");
   v->SetOption("mode", "mult");
   v->SetSelection("_NUC_->GetZ()>0");

   AddGV("KVZtot", "ZTOT");//total charge
   AddGV("KVZVtot", "ZVTOT");//total pseudo-momentum
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
   t->Branch("array", array, "array[mult]/I");
   t->Branch("idcode", idcode, "idcode[mult]/I");
   t->Branch("ecode", ecode, "ecode[mult]/I");
   t->Branch("Ameasured", Ameasured, "Ameasured[mult]/I");
   t->Branch("Vper", Vper, "Vper[mult]/D");
   t->Branch("Vpar", Vpar, "Vpar[mult]/D");
   t->Branch("ELab", ELab, "ELab[mult]/D");
   t->Branch("ThetaLab", ThetaLab, "ThetaLab[mult]/D");
   t->Branch("PhiLab", PhiLab, "PhiLab[mult]/D");

   AddTree(t);

   // check if we can access the original simulated events before filtering
   // (activated when selecting both filtered & simulated files in kaliveda-sim GUI)
   link_to_unfiltered_simulation = IsOptGiven("AuxFiles");
}

//____________________________________________________________________________________

void ExampleFilteredSimDataAnalysis::InitRun()
{
   // INITIALISATION PERFORMED JUST BEFORE ANALYSIS
   // In this method the multidetector array/setup used to filter
   // the simulation is available (gMultiDetArray)
   // The kinematics of the reaction is available (KV2Body*)
   // using gDataAnalyser->GetKinematics()

   // normalize ZVtot to projectile Z*v
   const KV2Body* kin = gDataAnalyser->GetKinematics();
   GetGV("ZVTOT")->SetParameter("Normalization",
                                kin->GetNucleus(1)->GetVpar()*kin->GetNucleus(1)->GetZ());
}

//____________________________________________________________________________________

Bool_t ExampleFilteredSimDataAnalysis::Analysis()
{
   // EVENT BY EVENT ANALYSIS

   // Reject events with less good particles than acquisition trigger for run
   if (!GetEvent()->IsOK()) return kTRUE;

   mult = GetEvent()->GetMult("ok");

   // if we can access the events of the unfiltered simulation, read in the event corresponding
   // to the currently analysed reconstructed event
   if (link_to_unfiltered_simulation) GetFriendTreeEntry(GetEvent()->GetParameters()->GetIntValue("SIMEVENT_TREE_ENTRY"));

   for (int i = 0; i < mult; i++) {
      KVReconstructedNucleus* part = (KVReconstructedNucleus*)ZMAX->GetZmax(i);
      Z[i] = part->GetZ();
      A[i] = part->GetA();
      idcode[i] = part->GetIDCode();
      ecode[i] = part->GetECode();
      Ameasured[i] = part->IsAMeasured();
      // Example for events filtered with FAZIA@INDRA set-up
      if (part->GetParameters()->GetTStringValue("ARRAY") == "INDRA") array[i] = 0;
      else if (part->GetParameters()->GetTStringValue("ARRAY") == "FAZIA") array[i] = 1;
      else array[i] = -1;
      Vper[i] = part->GetFrame("cm")->GetVperp();
      Vpar[i] = part->GetFrame("cm")->GetVpar();
      ELab[i] = part->GetEnergy();
      ThetaLab[i] = part->GetTheta();
      PhiLab[i] = part->GetPhi();
      // if we can access the events of the unfiltered simulation, and if Gemini++ was used
      // to decay events before filtering, this is how you can access the "parent" nucleus
      // of the current detected decay product
      // KVSimNucleus* papa = (KVSimNucleus*)GetFriendEvent()->GetParticle( part->GetParameters()->GetIntValue("GEMINI_PARENT_INDEX") );
   }

   GetGVList()->FillBranches();
   FillTree();

   return kTRUE;
}

