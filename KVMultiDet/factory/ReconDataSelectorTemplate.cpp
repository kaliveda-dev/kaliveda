#include "KVDataAnalyser.h"
#include "KVMultiDetArray.h"
#include "KVFlowTensor.h"

void ReconDataSelectorTemplate::InitAnalysis(void)
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
   AddGV("KVMult", "mtot");                             // total multiplicity
   AddGV("KVEtransLCP", "et12");                        // total LCP transverse energy
   KVVarGlob* gv = AddGV("KVFlowTensor", "tensor");
   gv->SetOption("weight", "RKE");
   gv->SetFrame("CM");// optional - this is the default frame
   gv->SetSelection("_NUC_->GetZ()>4");          // relativistic CM KE tensor for fragments

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
   SetJobOutputFileName("ReconDataSelectorTemplate_results.root");
}

//_____________________________________
void ReconDataSelectorTemplate::InitRun(void)
{
   // Initialisations for each run
   // Called at the beginning of each run

   // You no longer need to define the correct identification/calibration codes for particles
   // which will be used in your analysis, they are automatically selected using the default
   // values in variables *.ReconstructedNuclei.AcceptID/ECodes.

   // You can also perform more fine-grained selection of particles using class KVParticleCondition.
   // For example:

   KVParticleCondition pc_z("_NUC_->GetZ()>0&&_NUC_->GetZ()<=92");  // remove any strange Z identifications
   KVParticleCondition pc_e("_NUC_->GetE()>0.");                    // remove any immobile nuclei
   SetParticleConditions(pc_z && pc_e);

   // set title of TTree with name of analysed system
   GetTree("myTree")->SetTitle(GetCurrentRun()->GetSystemName());

   // normalize ZVtot to projectile Z*v
   const KV2Body* kin = gDataAnalyser->GetKinematics();
   GetGV("zvtot")->SetNormalization(kin->GetNucleus(1)->GetVpar()*kin->GetNucleus(1)->GetZ());
   // normalize Ztot to system Ztot
   GetGV("ztot")->SetNormalization(GetCurrentRun()->GetSystem()->GetZtot());
}

//_____________________________________
Bool_t ReconDataSelectorTemplate::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVReconstructedEvent documentation for the available methods.

   // reject events with less identified particles than the acquisition multiplicity trigger
   if (!GetEvent()->IsOK()) return kTRUE;

   // Event selection criteria - keep well-measured events
   if (GetGV("ztot") < 0.8 || (GetGV("zvtot") < 0.8 || GetGV("zvtot") > 1.1)) return kTRUE;

   GetGVList()->FillBranches(); // update values of all global variable branches
   FillTree(); // write new results in TTree

   // Define ellipsoid frame (wrt axes of flow tensor ellipsoid)
   GetEvent()->SetFrame("EL", "CM", ((KVFlowTensor*)GetGV("tensor"))->GetFlowReacPlaneRotation());

   /*** LOOP OVER PARTICLES OF EVENT ***/
   KVReconstructedNucleus* n;
   while ((n = (KVReconstructedNucleus*)GetEvent()->GetNextParticle("OK"))) {
      // "OK" particles => using selection criteria of InitRun() + any KVParticleConditions
      // fill Z distribution
      FillHisto("zdist", n->GetZ());
      // fill Z-Vpar(ellipsoid)
      FillHisto("zvpar", n->GetFrame("EL")->GetVpar(), n->GetZ());
   }

   return kTRUE;
}

