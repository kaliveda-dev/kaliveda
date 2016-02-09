#define KVIVSelector_cxx
// The class definition in KVIVSelector.h has been generated automatically
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
// Root > T->Process("KVIVSelector.C")
// Root > T->Process("KVIVSelector.C","some options")
// Root > T->Process("KVIVSelector.C+")
//

#include "KVIVSelector.h"
#include "KVClassFactory.h"

KVIVSelector::KVIVSelector(TTree* tree)
   : KVSelector(tree)
{
   // Default ctor. Does nothing.
}

KVIVSelector::~KVIVSelector()
{
   // Destructor
}

void KVIVSelector::Make(const Char_t* kvsname)
{
   // Automatic generation of KVIVSelector-derived class for KaliVeda analysis
   KVClassFactory cf(kvsname, "User analysis class", "KVIVSelector", kTRUE);
   cf.AddImplIncludeFile("KVINDRAReconNuc.h");
   cf.AddImplIncludeFile("KVVAMOSReconNuc.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");
   cf.AddImplIncludeFile("KVINDRA.h");
   cf.GenerateCode();
}

Bool_t KVIVSelector::Notify()
{
   // Wrapper for KVSelector::Notify to catch exceptions thrown in
   // KVIVDB_e503::SetParameters() method.

   Bool_t status(kFALSE);

   try {
      status = KVSelector::Notify();
   } catch (const std::exception& e) {
      // We've caught an exception from the SetParameters method! We handle
      // this event by terminating the current run.
      Error("Notify", "%s", e.what());
      EndRun();
   }

   return status;
}

