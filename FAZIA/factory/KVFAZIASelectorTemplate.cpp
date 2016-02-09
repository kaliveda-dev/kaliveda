// This class is derived from the KaliVeda class KVFAZIASelector.
// The following members functions are called by the TTree::Process() functions:
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
// E.g. if you want to access informations on the FAZIA multidetector
// through the global pointer gIndra, you must AT LEAST add the line
//   #include "KVFAZIA.h"

ClassImp(KVFAZIASelectorTemplate)

void KVFAZIASelectorTemplate::InitAnalysis(void)
{
//
// Declaration of histograms, global variables, etc.
// Called at the beginning of the analysis
//

//
// Enter your code here
//
// To access some information about FAZIA, use the global pointer gFazia
// e.g. to access the list of silicon detetors
//      KVList *list = gFazia->GetListOfDetectors();
//
//When running in batch mode, the global pointer gBatchSystem gives access
//to the batch system running the analysis. You can use this to name files
//according to the jobname, for example:
//
//   TFile *my_file;
//   if( gBatchSystem )  // running in batch
//      my_file = new TFile( Form("%s.root", gBatchSystem->GetJobName()), "create");
//   else  // interactive
//      my_file = new TFile( "KVFAZIASelectorTemplate.root", "create");
}

//_____________________________________
void KVFAZIASelectorTemplate::InitRun(void)
{
//
// Initialisations for each run
// Called at the beginning of each run
//

//
// Enter your code here
//
//
// If you want the angles of particles to be calculated using the
// centre of each detector (instead of the randomised angles used by default):
//     GetEvent()->UseMeanAngles();
//
//The run number of the currently analysed run is given by
//   gFazia->GetCurrentRunNumber();

}

//_____________________________________
Bool_t KVFAZIASelectorTemplate::Analysis(void)
{
//
// Analysis method.
// The event pointer can be retrieved by a call to GetEvent().
// See KVReconstructedEvent documentation for the available methods.
//
// In order to loop over all correctly identified particles in the event
// (assuming the corresponding code masks were set in InitRun):
//      KVReconstructedNucleus* particle;
//      while ( (particle = (KVReconstructedNucleus* )GetEvent()->GetNextParticle("ok")) ){
//            int z = particle->GetZ();
//            ...etc. etc.
//     }

//
// Enter your code here
//

   return kTRUE;
}

//_____________________________________
void KVFAZIASelectorTemplate::EndRun(void)
{
//
// Called at the end of each run
//

//
// Enter your code here
//

}

//_____________________________________
void KVFAZIASelectorTemplate::EndAnalysis(void)
{
//
// Global calculations, saving of histograms in files
// Called at the end of the analysis
//

//
// Enter your code here
//

}
