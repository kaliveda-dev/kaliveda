// This class is derived from the KaliVeda class KVSelector.
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
// E.g. if you want to access informations on the INDRA multidetector
// through the global pointer gIndra, you must AT LEAST add the line
//   #include "KVINDRA.h"

void KVSelectorTemplate::InitAnalysis(void)
{
//
// Declaration of histograms, global variables, etc.
// Called at the beginning of the analysis
//

//
// Enter your code here
//
// To access some information about INDRA, use the global pointer gIndra
// e.g. to access the list of silicon detetors
//      KVList *sil_list = gIndra->GetListOfSi();
//
//When running in batch mode, the global pointer gBatchSystem gives access
//to the batch system running the analysis. You can use this to name files
//according to the jobname, for example:
//
//   TFile *my_file;
//   if( gBatchSystem )  // running in batch
//      my_file = new TFile( Form("%s.root", gBatchSystem->GetJobName()), "create");
//   else  // interactive
//      my_file = new TFile( "KVSelectorTemplate.root", "create");
}

//_____________________________________
void KVSelectorTemplate::InitRun(void)
{
//
// Initialisations for each run
// Called at the beginning of each run
//

//
// Enter your code here
//
// To define which identification/calibration codes you want
// to use in your analysis:
//     GetEvent()->AcceptIDCodes(kIDCode2 | kIDCode3 | kIDCode4 | kIDCode6);//VEDA id-codes code(i)=2,3,4 and 6 accepted
//     GetEvent()->AcceptECodes( kECode1|kECode2 );//VEDA ecode(i)=1&2 accepted
//
// If you want the angles of particles to be calculated using the
// centre of each detector (instead of the randomised angles used by default):
//     GetEvent()->UseMeanAngles();
//
//The run number of the currently analysed run is given by
//   gIndra->GetCurrentRunNumber();

}

//_____________________________________
Bool_t KVSelectorTemplate::Analysis(void)
{
//
// Analysis method.
// The event pointer can be retrieved by a call to GetEvent().
// See KVINDRAReconEvent documentation for the available methods.
//
// In order to loop over all correctly identified particles in the event
// (assuming the corresponding code masks were set in InitRun):
//      KVINDRAReconNuc* particle;
//      while ( (particle = GetEvent()->GetNextParticle("ok")) ){
//            int z = particle->GetZ();
//            ...etc. etc.
//     }

//
// Enter your code here
//

   return kTRUE;
}

//_____________________________________
void KVSelectorTemplate::EndRun(void)
{
//
// Called at the end of each run
//

//
// Enter your code here
//

}

//_____________________________________
void KVSelectorTemplate::EndAnalysis(void)
{
//
// Global calculations, saving of histograms in files
// Called at the end of the analysis
//

//
// Enter your code here
//

}
