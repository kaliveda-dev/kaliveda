#define KVINDRAGeneDataSelector_cxx

#include "KVINDRAGeneDataSelector.h"
#include <TH2.h>
#include <TStyle.h>
#include "KVClassFactory.h"

ClassImp(KVINDRAGeneDataSelector)



void KVINDRAGeneDataSelector::Begin(TTree* /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

   InitAnalysis();              //user initialisations for analysis

}

void KVINDRAGeneDataSelector::SlaveBegin(TTree* /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

Bool_t KVINDRAGeneDataSelector::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either KVINDRAGeneDataSelector::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   fTreeEntry = entry;
   GetEntry(entry);

   Analysis();

   // Testing whether EndRun() should be called
   if (AtEndOfRun()) {
      EndRun();                 //user routine end of run
      delete gIndra;            // Absolutely necessay to keep the coherence between
      gIndra = 0;               // the pointers to the detectors and the TRef's
      needToCallEndRun = kFALSE;
   }

   return kTRUE;
}

void KVINDRAGeneDataSelector::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void KVINDRAGeneDataSelector::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   if (needToCallEndRun) {
      EndRun();
      delete gIndra;            // Absolutely necessay to keep the coherence between
      gIndra = 0;               // the pointers to the detectors and the TRef's
   }
   EndAnalysis();
}

//____________________________________________________________________________
Bool_t KVINDRAGeneDataSelector::AtEndOfRun(void)
{
//
// Check whether the end of run is reached for the current tree
//

   Bool_t ok = (fTreeEntry + 1 == fChain->GetTree()->GetEntries());

   return ok;
}

//_______________________________________________________________________//

void KVINDRAGeneDataSelector::Make(const Char_t* kvsname)
{
   //Automatic generation of derived class for gene data analysis

   KVClassFactory cf(kvsname, "User gene data analysis class", "KVINDRAGeneDataSelector");
   cf.AddMethod("InitAnalysis", "void");
   cf.AddMethod("InitRun", "void");
   cf.AddMethod("Analysis", "Bool_t");
   cf.AddMethod("EndRun", "void");
   cf.AddMethod("EndAnalysis", "void");
   KVString body;
   //initanalysis
   body = "   //Initialisation of e.g. histograms, performed once at beginning of analysis";
   cf.AddMethodBody("InitAnalysis", body);
   //initrun
   body = "   //Initialisation performed at beginning of each run\n";
   body += "   //Int RunNumber holds number of current run\n";
   cf.AddMethodBody("InitRun", body);
   //Analysis
   body = "   //Analysis method called for each event\n";
   body += "   //Int RunNumber holds number of current run\n";
   body += "   //Int EventNumber holds number of current event\n";
   body += "   //KVINDRATriggerInfo* TriggerInfo holds info on INDRA trigger for current event\n";
   body += "   return kTRUE;";
   cf.AddMethodBody("Analysis", body);
   //endrunù
   body = "   //Method called at end of each run";
   cf.AddMethodBody("EndRun", body);
   //endanalysis
   body = "   //Method called at end of analysis: save/display histograms etc.";
   cf.AddMethodBody("EndAnalysis", body);
   cf.GenerateCode();
}
