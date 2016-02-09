//Created by KVClassFactory on Wed Jan 28 09:54:34 2015
//Author: Eric Bonnet

#include "KVFAZIASelector.h"
#include "KVReconstructedNucleus.h"
#include "KVBatchSystem.h"
#include "KVFAZIA.h"
#include "KVDataSet.h"
#include "KVDataAnalyser.h"
#include "KVFAZIADetector.h"
#include "KVSignal.h"
#include "KVClassFactory.h"

ClassImp(KVFAZIASelector)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIASelector</h2>
<h4>selector to analyze FAZIA reconstructed data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

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
// E.g. if you want to access informations on the FAZIA multidetector
// through the global pointer gIndra, you must AT LEAST add the line
//   #include "KVFAZIA.h"

void KVFAZIASelector::SlaveBegin(TTree* tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   SetReadingOfRawData(kFALSE);
   gDataAnalyser->preInitAnalysis();

   //
   // ParseOptions : Manage options passed as arguments
   // done in KVEventSelector::SlaveBegin
   KVEventSelector::SlaveBegin(tree);

}

void KVFAZIASelector::Init(TTree* tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer

   Event = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   if (strcmp(GetBranchName(), "")  && fChain->GetBranch(GetBranchName())) {
      Info("Init", "Analysing data in branch : %s", GetBranchName());
      fChain->SetBranchAddress(GetBranchName() , &Event, &b_Event);
   }
   if (NeedToReadRawData())
      LinkRawData();

   //user additional branches addressing
   SetAdditionalBranchAddress();
   fEventsRead = 0;

}

Bool_t KVFAZIASelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   if (fNotifyCalled) return kTRUE; // avoid multiple calls at beginning of analysis
   fNotifyCalled = kTRUE;

   Info("Notify", "Beginning analysis of file %s (%lld events)", fChain->GetCurrentFile()->GetName(), fChain->GetTree()->GetEntries());
   gDataAnalyser->preInitRun();

   InitRun();  //user initialisations for run

   return kTRUE;
}

//_____________________________________
void KVFAZIASelector::LinkRawData()
{
   Info("LinkRawData", "Enable reading of raw data : %s\n", rawdatabranchname.Data());
   RawEvent = 0;
   fChain->SetBranchAddress(rawdatabranchname.Data(), &RawEvent);
}

//_____________________________________
void KVFAZIASelector::ConnectSignalsToDetectors()
{
   KVFAZIADetector* det = 0;
   KVSignal* sig = 0;

   TIter next_s(RawEvent->GetSignals());
   while ((sig = (KVSignal*)next_s())) {
      sig->DeduceFromName();
      det = (KVFAZIADetector*)gFazia->GetDetector(sig->GetDetectorName());
      det->SetSignal(sig, sig->GetType());
   }

}

//_____________________________________
void KVFAZIASelector::ParseOptions()
{
   KVEventSelector::ParseOptions();

   //check if the reading of raw data is needed
   if (IsOptGiven("ReadRawData")) SetReadingOfRawData(GetOpt("ReadRawData").Atoi());

   //Force the name of the KVEvent branch after the call to ParseOption
   SetBranchName("FAZIAReconEvent");
}

//_____________________________________
void KVFAZIASelector::Make(const Char_t* kvsname)
{
   // Automatic generation of KVSelector-derived class for KaliVeda analysis
   KVClassFactory cf(kvsname, "User analysis class", Class_Name(), kTRUE);
   cf.AddImplIncludeFile("KVReconstructedNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");
   cf.AddImplIncludeFile("KVFAZIA.h");
   cf.GenerateCode();
}
