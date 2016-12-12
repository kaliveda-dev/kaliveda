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

void KVSelectorTemplate::InitAnalysis(void)
{
   // Declaration of histograms, global variables, etc.
   // Called at the beginning of the analysis

   //
   // Enter your code here
   //
   // To access some information about INDRA, use the global pointer gIndra
   // e.g. to access the list of silicon detetors
   //      KVList *sil_list = gIndra->GetListOfSi();
   //
   // When running in batch mode, the global pointer gBatchSystem gives access
   // to the batch system running the analysis. You can use this to name files
   // according to the jobname, for example:
   //
   if (gBatchSystem)   // running in batch
      my_file = new TFile(Form("%s.root", gBatchSystem->GetJobName()), "recreate");
   else  // interactive
      my_file = new TFile("KVSelectorTemplate_results.root", "recreate");

   mult = new TH1F("mult", "Multiplicity distribution", 100, -.5, 99.5);
   zdist = new TH1F("zdist", "Charge distribution", 100, -.5, 99.5);
}

//_____________________________________
void KVSelectorTemplate::InitRun(void)
{
   // Initialisations for each run
   // Called at the beginning of each run

   //
   // Enter your code here
   //
   // You need to define here the correct identification/calibration codes for particles
   // which will be used in your analysis. The following is the standard definition for
   // INDRA data. See classes KVINDRACodes/KVINDRACodeMask for more details.
   GetEvent()->AcceptIDCodes(kIDCode2 | kIDCode3 | kIDCode4 | kIDCode6);//particle identification codes
   GetEvent()->AcceptECodes(kECode1 | kECode2);//particle calibration codes

   // You can also perform more fine-grained selection of particles using class KVParticleCondition.
   // For example, to eliminate any particles with timestamps outside of a given window,
   //
   //   KVParticleCondition c("_NUC_->GetTimeMarker()>=80 && _NUC_->GetTimeMarker()<=120");
   //   SetParticleCondtions(c);
   //
   // If you want the angles of particles to be calculated using the
   // centre of each detector (instead of the randomised angles used by default):
   //   GetEvent()->UseMeanAngles();
   //
   //The run number of the currently analysed run is given by
   //   gIndra->GetCurrentRunNumber();

}

//_____________________________________
Bool_t KVSelectorTemplate::Analysis(void)
{
   // Analysis method called event by event.
   // The current event can be accessed by a call to method GetEvent().
   // See KVINDRAReconEvent documentation for the available methods.
   //
   // Do not remove the following line
   if (!GetEvent()->IsOK()) return kTRUE;

   // Enter your code here
   //
   // Example: In order to loop over particles in the event selected according to
   // the criteria set in method InitRun() (identification codes etc.):
   KVINDRAReconNuc* particle;
   while ((particle = GetEvent()->GetNextParticle("ok"))) {  // "ok" => using selection criteria
      zdist->Fill(particle->GetZ());
   }
   // The total multiplicity of the correctly identified particles is given by
   mult->Fill(GetEvent()->GetMult("ok"));

   return kTRUE;
}

//_____________________________________
void KVSelectorTemplate::EndRun(void)
{
   // Called at the end of each run
   //
   // Enter your code here if required
}

//_____________________________________
void KVSelectorTemplate::EndAnalysis(void)
{
   // Called at the end of the analysis
   // Global calculations, saving of histograms in files
   //
   // Enter your code here if required
   my_file->Write();
   my_file->Close();
}
