#define KVFAZIAReader_cxx
// The class definition in KVFAZIAReader.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("KVFAZIAReader.C")
// Root > T->Process("KVFAZIAReader.C","some options")
// Root > T->Process("KVFAZIAReader.C+")
//

#include "KVFAZIAReader.h"
#include "KVDataAnalyser.h"

#include "KVFAZIA.h"

#include <KVClassFactory.h>

ClassImp(KVFAZIAReader)

void KVFAZIAReader::Begin(TTree* /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).
   Info("Begin", "Start");
   TString option = GetOption();

}

void KVFAZIAReader::SlaveBegin(TTree* /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   Info("SlaveBegin", "Start");
   TString option = GetOption();

}

Int_t KVFAZIAReader::GetEntry(Long64_t entry, Int_t getall)
{

   return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;

}

Bool_t KVFAZIAReader::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either KVFAZIAReader::GetEntry() or TBranch::GetEntry()
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
   fReadEntries += 1;

   GetEntry(entry);
   fEventNumber = RawEvent->GetNumber();

   gFazia->GetDetectorEvent(GetDetectorEvent(), RawEvent->GetSignals());

   if (fReadEntries % 10000 == 0)
      Info("Process", "%d read entries", fReadEntries);

   return Analysis();
}

void KVFAZIAReader::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.
   Info("SlaveTerminate", "Start");

}

void KVFAZIAReader::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
   Info("Terminate", "%d events read", GetNumberOfReadEntries());
   EndRun();
   EndAnalysis();
}

void KVFAZIAReader::Init(TTree* tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers

   Info("Init", "Start");
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);
   InitAnalysis();

}

Bool_t KVFAZIAReader::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.
   if (fCurrentRun != -1)
      EndRun();

   fReadEntries = 0;
   fCurrentRun = gDataAnalyser->GetRunNumberFromFileName(fChain->GetCurrentFile()->GetName());
   Info("Notify", "Traitement du run %d", fCurrentRun);
   fChain->SetBranchAddress("rawevent", &RawEvent);
   InitRun();
   return kTRUE;
}

void KVFAZIAReader::Make(const Char_t* kvsname)
{
   // Automatic generation of KVFAZIAReader-derived class for KaliVeda analysis

   KVClassFactory cf(kvsname, "User analysis class", "KVFAZIAReader", kTRUE);
   cf.AddImplIncludeFile("KVReconstructedNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");
   cf.AddImplIncludeFile("KVFAZIA.h");
   cf.GenerateCode();
}
