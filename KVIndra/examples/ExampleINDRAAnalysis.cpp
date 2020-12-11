//Created by KVClassFactory on Mon Feb 19 14:32:51 2018
//Author: John Frankland

#include "ExampleINDRAAnalysis.h"
#include "KVINDRAReconNuc.h"
#include "KVBatchSystem.h"
#include "KVINDRA.h"

ClassImp(ExampleINDRAAnalysis)

#include "KVDataAnalyser.h"

void ExampleINDRAAnalysis::InitAnalysis(void)
{
   // Declaration of histograms, global variables, etc.
   // Called at the beginning of the analysis
   // The examples given are compatible with interactive, batch,
   // and PROOFLite analyses.

   /*** ADDING GLOBAL VARIABLES TO THE ANALYSIS ***/
   /* These will be automatically calculated for each event before
      your Analysis() method will be called                        */
   KVVarGlob* vg = AddGV("KVZtot", "ztot");                             // total charge
#ifdef USING_ROOT6
   // define event selection to reject pile-up events
   vg->SetEventSelection([&](const KVVarGlob * v) {
      return (v->GetValue() <= current_run_system_ztot + 4);
   });
#endif

   vg = AddGV("KVZVtot", "zvtot");                                     // total Z*vpar
   vg->SetMaxNumBranches(1);
#ifdef USING_ROOT6
   // refine event selection to reject pile-up events
   vg->SetEventSelection([&](const KVVarGlob * v) {
      return (v->GetValue() < 1.2);
   });
#endif

   AddGV("KVMult", "Mult");                 // total mult
   AddGV("KVEtransLCP", "et12");                        // total LCP transverse energy

   AddGV("KVFlowTensor", "tensor")->SetOption("weight", "RKE");  // relativistic CM KE tensor
#ifdef USING_ROOT6
   GetGV("tensor")->SetSelection({"Z>2", [](const KVNucleus * N)
   {
      return N->GetZ() > 2;
   }
                                 });
#else
   GetGV("tensor")->SetSelection("_NUC_->GetZ()>2");
#endif
   GetGV("tensor")->SetMaxNumBranches(2);               // FlowAngle & Sphericity branches

   AddGV("KVMultLeg", "Mlcp");           // mult LCP

   /*** USING A TREE ***/
   CreateTreeFile();//<--- essential
   TTree* t = new TTree("myTree", "");
   AddTree(t);
   GetGVList()->MakeBranches(t); // store global variable values in branches
   t->AddVar(MTensor, I);
   t->AddVar(Run, I);
   t->AddVar(Trigger, I);
   t->AddVar(EventNumber, I);

   /*** DEFINE WHERE TO SAVE THE RESULTS ***/
   // When running in batch/PROOF mode, we use the job name
   if (gDataAnalyser->GetBatchSystem())
      SetCombinedOutputFile(Form("%s.root", gDataAnalyser->GetBatchSystem()->GetJobName()));
   else
      SetCombinedOutputFile(Form("ExampleINDRAAnalysis_results.root"));
}

//_____________________________________
void ExampleINDRAAnalysis::InitRun(void)
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

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());

   // store current run number
   Run = GetCurrentRun()->GetNumber();

   // set normalisation for KVZVtot: use Z*v of projectile
   const KV2Body* kin = gDataAnalyser->GetKinematics();
   GetGV("zvtot")->SetNormalization(kin->GetNucleus(1)->GetVpar()*kin->GetNucleus(1)->GetZ());

   // store trigger of current run
   Trigger = GetCurrentRun()->GetTrigger();

#ifdef USING_ROOT6
   current_run_system_ztot = GetCurrentRun()->GetSystem()->GetZtot();
#endif
}

//_____________________________________
Bool_t ExampleINDRAAnalysis::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVINDRAReconEvent documentation for the available methods.

   // Do not remove the following line - reject events with less identified particles than
   // the acquisition multiplicity trigger
   if (!GetEvent()->IsOK()) return kTRUE;

#ifndef USING_ROOT6
   // avoid pile-up events
   if (GetGV("ztot")->GetValue() > GetCurrentRun()->GetSystem()->GetZtot() + 4
         || GetGV("zvtot")->GetValue() > 1.2) return kTRUE;
#endif

   GetGVList()->FillBranches(); // update values of all global variable branches

   EventNumber = GetEventNumber();
   MTensor = GetGV("tensor")->GetValue("NumberParts");
   // write new results in TTree
   FillTree();

   return kTRUE;
}

