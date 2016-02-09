// This class is derived from the KaliVeda class KVIVSelector.
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
// Same thing to access information on the VAMOS spectrometer through the
// global pointer gVamos, you must AT LEAST add the line
//   #include "KVVAMOS.h"

void KVIVSelectorTemplate::InitAnalysis(void)
{
//
// Declaration of histograms, global variables, etc.
// Called at the beginning of the analysis
//

//
// Enter your code here
//
// To access some information about INDRA or VAMOS, use the global pointers
// gIndra and gVamos e.g. to access the list of INDRA's silicon detetors
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
//      my_file = new TFile( "KVIVSelectorTemplate.root", "create");
}

//_____________________________________
void KVIVSelectorTemplate::InitRun(void)
{
//
// Initialisations for each run
// Called at the beginning of each run
//

//
// Enter your code here
//
// To define which identification/calibration codes for INDRA you want
// to use in your analysis:
//     GetEvent()->AcceptIDCodesINDRA(kIDCode2 | kIDCode3 | kIDCode4 | kIDCode6);//VEDA id-codes code(i)=2,3,4 and 6 accepted
//     GetEvent()->AcceptECodesINDRA( kECode1|kECode2 );//VEDA ecode(i)=1&2 accepted
// To define which Focal plan Position reconstruction/identification/calibration/Time of Flight
// codes for VAMOS you want to use in your analysis, the method is similar:
//     GetEvent()->AcceptIDCodesVAMOS(kIDCode2 | kIDCode3 | kIDCode4 | kIDCode6);
//     GetEvent()->AcceptECodesVAMOS( kECode1|kECode2 );//ecode(i)=1&2 accepted
//     GetEvent()->AcceptFPCodesVAMOS( kFPCode1|kFPCode2 );//the better FP code is kFPCode1
//     GetEvent()->AcceptTCodesVAMOS( kTCode1|kTCode2 );//the better T code is kTCode1
//
// The meaning of the available FP/T codes is shown by the static methods
//      KVVAMOSCodes::ShowAvailableFPCodes().
//      KVVAMOSCodes::ShowAvailableTCodes().
//
// If you want the angles of particles to be calculated using the
// centre of each detector in INDRA (instead of the randomised angles used by default):
//     GetEvent()->UseMeanAngles();
//
//The run number of the currently analysed run is given by
//   gIndra->GetCurrentRunNumber();

}

//_____________________________________
Bool_t KVIVSelectorTemplate::Analysis(void)
{
//
// Analysis method.
// The event pointer can be retrieved by a call to GetEvent().
// See KVIVReconEvent documentation for the available methods.
//
// In order to loop over all correctly identified particles in the INDRA event
// (assuming the corresponding code masks were set in InitRun):
//      KVINDRAReconNuc* particle;
//      while ( (particle = GetEvent()->GetNextParticle("ok")) ){
//            Int_t z = particle->GetZ();
//            ...etc. etc.
//     }
//
// In order to loop over all correctly reconstructed and identified nuclei in the
// VAMOS event (assuming the corresponding code masks were set in InitRun), use the lines:
//     KVVAMOSReconNuc* nuc;
//     while ( (nuc = GetEvent()->GetNextNucleus("ok")) ){
//
//           Int_t z       = nuc->GetZ();
//           Double_t Brho = nuc->GetBrho();
//           Double_t M    = nuc->GetMass();
//     }

//
// Enter your code here
//

   return kTRUE;
}

//_____________________________________
void KVIVSelectorTemplate::EndRun(void)
{
//
// Called at the end of each run
//

//
// Enter your code here
//

}

//_____________________________________
void KVIVSelectorTemplate::EndAnalysis(void)
{
//
// Global calculations, saving of histograms in files
// Called at the end of the analysis
//

//
// Enter your code here
//

}
