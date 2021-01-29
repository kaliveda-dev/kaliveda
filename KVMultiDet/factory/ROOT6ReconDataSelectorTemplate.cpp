// This class is derived from the KaliVeda class KVEventSelector.
// It is to be used for analysis of reconstructed data.
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
#include "KVMultiDetArray.h"
#include "KVFlowTensor.h"

void ROOT6ReconDataSelectorTemplate::InitAnalysis(void)
{
   // Declaration of histograms, global variables, etc.
   // Called at the beginning of the analysis
   // The examples given are compatible with interactive, batch,
   // and PROOFLite analyses.

   /*** ADDING GLOBAL VARIABLES TO THE ANALYSIS ***/
   /* These will be automatically calculated for each event before
      your Analysis() method will be called                        */
   auto ztot = AddGV("KVZtot", "ztot");                 // total charge
   // complete event selection: total charge
   ztot->SetEventSelection([&](const KVVarGlob * var) {
      return var->GetValue() >= 0.8 * ztot_sys;
   });

   auto zvtot = AddGV("KVZVtot", "zvtot");              // total Z*vpar
   zvtot->SetMaxNumBranches(1);    // only write "Z" component in TTree
   // complete event selection: total pseudo-momentum
   zvtot->SetEventSelection([&](const KVVarGlob * var) {
      return var->GetValue() >= 0.8 * zvtot_sys
             && var->GetValue() <= 1.1 * zvtot_sys;
   });
   AddGV("KVMult", "mtot");                             // total multiplicity
   AddGV("KVEtransLCP", "et12");                        // total LCP transverse energy
   auto gv = AddGV("KVFlowTensor", "tensor");
   gv->SetOption("weight", "RKE");
   gv->SetFrame("CM");// optional - this is the default frame
   gv->SetSelection({"Z>4", [](const KVNucleus * n)
   {
      return n->GetZ() > 4;
   }});   // relativistic CM KE tensor for fragments
   // Define ellipsoid frame (wrt axes of flow tensor ellipsoid)
   gv->SetNewFrameDefinition([](KVEvent * e, const KVVarGlob * vg) {
      e->SetFrame("EL", "CM", ((KVFlowTensor*)vg)->GetFlowReacPlaneRotation());
   });

   /*** DECLARING SOME HISTOGRAMS ***/
   AddHisto(new TH1F("zdist", "Charge distribution", 100, -.5, 99.5));
   AddHisto(new TH2F("zvpar", "Z vs V_{par} in ellipsoid", 100, -15., 15., 75, .5, 75.5));

   /*** USING A TREE ***/
   CreateTreeFile();//<--- essential
   TTree* t = new TTree("myTree", "");
   AddTree(t);
   GetGVList()->MakeBranches(t); // store global variable values in branches

   /*** DEFINE WHERE TO SAVE THE RESULTS ***/
   // This filename will be used for interactive and PROOFlite jobs.
   // When running in batch mode, this will automatically use the job name.
   SetJobOutputFileName("ROOT6ReconDataSelectorTemplate_results.root");
}

//_____________________________________
void ROOT6ReconDataSelectorTemplate::InitRun(void)
{
   // Initialisations for each run
   // Called at the beginning of each run

   // You no longer need to define the correct identification/calibration codes for particles
   // which will be used in your analysis, they are automatically selected using the default
   // values in variables *.ReconstructedNuclei.AcceptID/ECodes.

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());

   // retrieve system parameters for complete event selection
   const KV2Body* kin = gDataAnalyser->GetKinematics();
   zvtot_sys = kin->GetNucleus(1)->GetVpar() * kin->GetNucleus(1)->GetZ();
   ztot_sys = GetCurrentRun()->GetSystem()->GetZtot();

   // reject reconstructed events which are not consistent with the DAQ trigger
   SetTriggerConditionsForRun(GetCurrentRun()->GetNumber());
}

//_____________________________________
Bool_t ROOT6ReconDataSelectorTemplate::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVReconstructedEvent documentation for the available methods.

   GetGVList()->FillBranches(); // update values of all global variable branches
   FillTree(); // write new results in TTree

   /*** LOOP OVER PARTICLES OF EVENT ***/
   for (auto& n : OKEventIterator(*GetEvent())) {
      // "OK" particles => using selection criteria of InitRun() + any KVParticleConditions
      // fill Z distribution
      FillHisto("zdist", n.GetZ());
      // fill Z-Vpar(ellipsoid)
      FillHisto("zvpar", n.GetFrame("EL")->GetVpar(), n.GetZ());
   }

   return kTRUE;
}

