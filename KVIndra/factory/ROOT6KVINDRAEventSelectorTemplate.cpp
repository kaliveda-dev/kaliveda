#include "KVDataAnalyser.h"
#include "KVFlowTensor.h"

void ROOT6KVINDRAEventSelectorTemplate::InitAnalysis(void)
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
      return var->GetValue() >= 0.8 * ztot_sys; // ztot_sys will be set in InitRun
   });

   auto zvtot = AddGV("KVZVtot", "zvtot");              // total Z*vpar
   zvtot->SetMaxNumBranches(1);    // only write "Z" component in TTree
   // complete event selection: total pseudo-momentum
   zvtot->SetEventSelection([&](const KVVarGlob * var) {
      return var->GetValue() >= 0.8 * zvtot_sys
             && var->GetValue() <= 1.1 * zvtot_sys; // zvtot_sys will be set in InitRun
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
   //
   // You can change the selection (or deactivate it) here by doing:
   // gMultiDetArray->AcceptECodes(""); => accept all calibration codes
   // gMultiDetArray->AcceptIDCodes("12,33"); => accept only ID codes in list
   //
   // If the experiment used a combination of arrays, codes have to be set for
   // each array individually:
   // gMultiDetArray->GetArray("[name]")->Accept.. => setting for array [name]

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());

   // Reject events with less identified particles than the acquisition multiplicity trigger
   SetTriggerConditionsForRun(GetCurrentRun()->GetNumber());

   // retrieve system parameters for complete event selection
   const KV2Body* kin = gDataAnalyser->GetKinematics();
   zvtot_sys = kin->GetNucleus(1)->GetVpar() * kin->GetNucleus(1)->GetZ();
   ztot_sys = GetCurrentRun()->GetSystem()->GetZtot();
}

//_____________________________________
Bool_t ROOT6KVINDRAEventSelectorTemplate::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVINDRAReconEvent documentation for the available methods.

   GetGVList()->FillBranches(); // update values of all global variable branches

   /*** LOOP OVER PARTICLES OF EVENT ***/
   for (auto& particle : ReconEventOKIterator(GetEvent())) {
      // "OK" => using selection criteria of InitRun()
      // fill Z distribution
      FillHisto("zdist", particle.GetZ());
      // fill Z-Vpar(ellipsoid)
      FillHisto("zvpar", particle.GetFrame("EL")->GetVpar(), particle.GetZ());
   }

   // write new results in TTree
   FillTree();

   return kTRUE;
}

