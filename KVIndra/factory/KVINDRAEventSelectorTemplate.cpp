// This class is derived from the KaliVeda class KVINDRAEventSelector.
// It is to be used for analysis of reconstructed INDRA data.
//
// The following members functions are called by the TTree::Process() functions:
//
//    InitRun():       called everytime a run starts
//    EndRun():        called everytime a run ends
//    InitAnalysis():  called at the beginning of the analysis
//                     a convenient place to create your histograms.
//    Analysis():      called for each event. In this function you
//                     fill your histograms.
//    EndAnalysis():   called at the end of a loop on the tree,
//                     a convenient place to draw/fit your histograms.
//
// Modify these methods as you wish in order to create your analysis class.
// Don't forget that for every class used in the analysis, you must put a
// line '#include' at the beginning of this file.

#include "KVDataAnalyser.h"

void KVINDRAEventSelectorTemplate::InitAnalysis(void)
{
   // Declaration of histograms, global variables, etc.
   // Called at the beginning of the analysis
   // The examples given are compatible with interactive, batch,
   // and PROOFLite analyses.

   /*** ADDING GLOBAL VARIABLES TO THE ANALYSIS ***/
   /* These will be automatically calculated for each event before
      your Analysis() method will be called                        */
   AddGV("KVZtot", "ztot");                             // total charge
   AddGV("KVZVtot", "zvtot")->SetMaxNumBranches(1);     // total Z*vpar
   AddGV("KVEtransLCP", "et12");                        // total LCP transverse energy
   AddGV("KVFlowTensor", "tensor")->SetOption("weight", "RKE");  // relativistic CM KE tensor


   /*** DECLARING SOME HISTOGRAMS ***/
   AddHisto(new TH1F("zdist", "Charge distribution", 100, -.5, 99.5));
   AddHisto(new TH2F("zvpar", "Z vs V_{par} in CM", 100, -15., 15., 75, .5, 75.5));

   /*** USING A TREE ***/
   CreateTreeFile();//<--- essential
   TTree* t = new TTree("myTree", "");
   AddTree(t);
   GetGVList()->MakeBranches(t); // store global variable values in branches
   t->Branch("Mult", &Mult, "Mult/I");
   t->Branch("Z", Z, "Z[Mult]/I");
   t->Branch("A", A, "A[Mult]/I");
   t->Branch("E", E, "E[Mult]/D");
   t->Branch("Theta", Theta, "Theta[Mult]/D");
   t->Branch("Phi", Phi, "Phi[Mult]/D");
   t->Branch("Vx", Vx, "Vx[Mult]/D");
   t->Branch("Vy", Vy, "Vy[Mult]/D");
   t->Branch("Vz", Vz, "Vz[Mult]/D");

   /*** DEFINE WHERE TO SAVE THE RESULTS ***/
   // When running in batch/PROOF mode, we use the job name
   if (gDataAnalyser->GetBatchSystem())
      SetCombinedOutputFile(Form("%s.root", gDataAnalyser->GetBatchSystem()->GetJobName()));
   else
      SetCombinedOutputFile(Form("KVINDRAEventSelectorTemplate_results.root"));
}

//_____________________________________
void KVINDRAEventSelectorTemplate::InitRun(void)
{
   // Initialisations for each run
   // Called at the beginning of each run

   // You no longer need to define the correct identification/calibration codes for particles
   // which will be used in your analysis, they are automatically selected using the default
   // values in variables INDRA.ReconstructedNuclei.AcceptID/ECodes.
   // However, if you want to change the default settings, it can be done here.
   // The following example is the standard definition for INDRA data.
   // See classes KVINDRACodes/KVINDRACodeMask for more details.
   //GetEvent()->AcceptIDCodes(kIDCode2 | kIDCode3 | kIDCode4 | kIDCode6);//particle identification codes
   //GetEvent()->AcceptECodes(kECode1 | kECode2);                         //particle calibration codes

   // You can also perform more fine-grained selection of particles using class KVParticleCondition.
   // For example:
   KVParticleCondition pc_z("_NUC_->GetZ()>0&&_NUC_->GetZ()<=92");  // remove any strange Z identifications
   KVParticleCondition pc_e("_NUC_->GetE()>0.");                    // remove any immobile nuclei
   KVParticleCondition pc_tm("_NUC_->GetTimeMarker()>=80 && _NUC_->GetTimeMarker()<=140");
   // eliminate random coincidences
   KVParticleCondition pc = pc_z && pc_e && pc_tm;                  // combine all previous selections
   SetParticleConditions(pc);

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());
}

//_____________________________________
Bool_t KVINDRAEventSelectorTemplate::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVINDRAReconEvent documentation for the available methods.

   // Do not remove the following line - reject events with less identified particles than
   // the acquisition multiplicity trigger
   if (!GetEvent()->IsOK()) return kTRUE;

   GetGVList()->FillBranches(); // update values of all global variable branches

   /*** LOOP OVER PARTICLES OF EVENT ***/
   KVINDRAReconNuc* particle;
   while ((particle = GetEvent()->GetNextParticle("OK"))) {  // "OK" => using selection criteria of InitRun()
      // fill Z distribution
      FillHisto("zdist", particle->GetZ());
      // fill Z-Vpar(cm)
      FillHisto("zvpar", particle->GetFrame("CM")->GetVpar(), particle->GetZ());
   }

   // fill arrays with particle properties for TTree
   GetEvent()->FillArraysEThetaPhi(Mult, Z, A, E, Theta, Phi, "CM", "OK");
   GetEvent()->FillArraysV(Mult, Z, A, Vx, Vy, Vz, "CM", "OK");

   // write new results in TTree
   FillTree();

   return kTRUE;
}

