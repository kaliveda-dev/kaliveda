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
   AddGV("KVZtot", "ztot");                             // total charge
   AddGV("KVZVtot", "zvtot")->SetMaxNumBranches(1);     // total Z*vpar
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
   AddGV("KVMultLeg", "Mlcp");

   /*** USING A TREE ***/
   CreateTreeFile();//<--- essential
   TTree* t = new TTree("myTree", "");
   AddTree(t);
   GetGVList()->MakeBranches(t); // store global variable values in branches
   t->AddVar(Mult, I);
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

   // You can also perform more fine-grained selection of particles using class KVParticleCondition.
   // For example:
   KVParticleCondition pc_z("_NUC_->GetZ()>0&&_NUC_->GetZ()<=92");  // remove any strange Z identifications
   KVParticleCondition pc_e("_NUC_->GetE()>0.");                    // remove any immobile nuclei
   KVParticleCondition pc_tm("_NUC_->GetTimeMarker()>=80 && _NUC_->GetTimeMarker()<=120");
   // eliminate random coincidences
   KVParticleCondition pc = pc_z && pc_e && pc_tm;                  // combine all previous selections
   SetParticleConditions(pc);

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());

   // store current run number
   Run = GetCurrentRun()->GetNumber();

   // set normalisation for KVZVtot: use Z*v of projectile
   const KV2Body* kin = gDataAnalyser->GetKinematics();
   GetGV("zvtot")->SetParameter("Normalization", kin->GetNucleus(1)->GetVpar()*kin->GetNucleus(1)->GetZ());

   // store trigger of current run
   Trigger = GetCurrentRun()->GetTrigger();
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

   // avoid pile-up events
   if (GetGV("ztot")->GetValue() > GetCurrentRun()->GetSystem()->GetZtot() + 4
         || GetGV("zvtot")->GetValue() > 1.2) return kTRUE;

   GetGVList()->FillBranches(); // update values of all global variable branches

   Mult = GetEvent()->GetMult("OK");
   EventNumber = GetEventNumber();
   MTensor = GetGV("tensor")->GetValue("NumberParts");
   // write new results in TTree
   FillTree();

   return kTRUE;
}

