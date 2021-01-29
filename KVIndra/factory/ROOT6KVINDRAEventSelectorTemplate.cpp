#include "KVDataAnalyser.h"

void ROOT6KVINDRAEventSelectorTemplate::InitAnalysis(void)
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

   /*** DEFINE WHERE TO SAVE THE RESULTS ***/
   SetJobOutputFileName("ROOT6KVINDRAEventSelectorTemplate_results.root");
}

//_____________________________________
void ROOT6KVINDRAEventSelectorTemplate::InitRun(void)
{
   // Initialisations for each run
   // Called at the beginning of each run

   // You no longer need to define the correct identification/calibration codes for particles
   // which will be used in your analysis, they are automatically selected using the default
   // values in variables INDRA.ReconstructedNuclei.AcceptID/ECodes.
   // However, if you want to change the default settings, it can be done here.

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());

   // Do not remove the following line - reject events with less identified particles than
   // the acquisition multiplicity trigger
   SetTriggerConditionsForRun(GetCurrentRun()->GetNumber());
}

//_____________________________________
Bool_t ROOT6KVINDRAEventSelectorTemplate::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVINDRAReconEvent documentation for the available methods.

   GetGVList()->FillBranches(); // update values of all global variable branches

   /*** LOOP OVER PARTICLES OF EVENT ***/
   for (auto& particle : OKEventIterator(*GetEvent())) {
      // "OK" => using selection criteria of InitRun()
      // fill Z distribution
      FillHisto("zdist", particle.GetZ());
      // fill Z-Vpar(cm)
      FillHisto("zvpar", particle.GetFrame("CM")->GetVpar(), particle.GetZ());
   }

   // write new results in TTree
   FillTree();

   return kTRUE;
}

