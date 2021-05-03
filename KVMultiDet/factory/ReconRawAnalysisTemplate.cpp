// This class is derived from the KaliVeda class KVReconRawDataAnalyser.
// It is to be used for analysis of events reconstructed from raw data.
//
// The following member functions are called in turn:
//
//    InitAnalysis():  called at the very beginning of the analysis
//    InitRun():       called every time a run starts
//    Analysis():      called for each event
//    EndRun():        called every time a run ends
//    EndAnalysis():   called after all requested data has been read
//
// Modify these methods as you wish in order to create your analysis class.
// Don't forget that for every class used in the analysis, you must put a
// line '#include' at the beginning of this file.

void ReconRawAnalysisTemplate::InitAnalysis()
{
   // Declaration of histograms, trees, etc.
   // Called at the beginning of the analysis
   // The examples given are compatible with interactive and batch analyses.

   /*** DECLARING SOME HISTOGRAMS ***/
   AddHisto(new TH1F("Mult", "Number of reconstructed particles in each event", 200, -.5, 199.5));

   /*** USING A TREE ***/
   CreateTreeFile();//<--- essential
   TTree* t = new TTree("myTree", "");
   AddTree(t);
   t->Branch("ArrayName", &ArrayName);
   t->Branch("Z", &Z);

   /*** DEFINE WHERE TO SAVE THE RESULTS ***/
   // This filename will be used for interactive and PROOFlite jobs.
   // When running in batch mode, this will automatically use the job name.
   SetJobOutputFileName("ReconRawAnalysisTemplate_results.root");
}

//____________________________________________________________________________//

void ReconRawAnalysisTemplate::InitRun()
{
   //Initialisation performed at beginning of each run
   //  GetRunNumber() returns current run number
   //  GetCurrentRun() returns KVDBRun pointer to current run in database
   //When this method is called, the detector geometry has been initialised and
   //can be accessed through global pointer gMultiDetArray

   Info("InitRun", "Beginning analysis of run %d containing %llu events", GetRunNumber(), GetCurrentRun()->GetEvents());

   // Set tree title to name of system being analysed
   GetTree("myTree")->SetTitle(GetSystem()->GetName());
}

//____________________________________________________________________________//

Bool_t ReconRawAnalysisTemplate::Analysis()
{
   //Analysis method called for each event
   //  GetEventNumber() returns current event number
   //  if gMultiDetArray->HandledRawData() returns kTRUE, an event was reconstructed and
   //  can be accessed through GetReconstructedEvent()
   //  Processing will stop if this method returns kFALSE

   if (gMultiDetArray->HandledRawData()) {
      // number of reconstructed nuclei
      Mult = GetReconstructedEvent()->GetMult();
      FillHisto("Mult", Mult);

      // loop over reconstructed nuclei in event
      KVReconstructedNucleus* rnuc;
      while ((rnuc = (KVReconstructedNucleus*)GetReconstructedEvent()->GetNextParticle())) {
         // store name of array particle was detected in and its Z (if known)
         ArrayName = rnuc->GetArrayName();
         Z = -1;
         if (rnuc->IsIdentified() && rnuc->IsZMeasured()) Z = rnuc->GetZ();
         FillTree();
      }
   }
   return kTRUE;
}
