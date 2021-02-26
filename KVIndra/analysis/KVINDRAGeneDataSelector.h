//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jun  4 18:46:15 2007 by ROOT version 5.14/00
// from TChain GeneData/
//////////////////////////////////////////////////////////

#ifndef KVINDRAGeneDataSelector_h
#define KVINDRAGeneDataSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include "KVINDRATriggerInfo.h"
#include "Riostream.h"
#include "KVINDRA.h"
#include "KVACQParam.h"

/**
\class KVINDRAGeneDataSelector
\brief For analysing INDRA pulser & laser data
\ingroup INDRAnalysis

<p>User analysis classes derived from this one have access to the
following variables :</p>
<ul>
<li>Int_t  RunNumber;</li>
<li>Int_t  EventNumber;</li>
<li>KVINDRATriggerInfo *TriggerInfo;</li>
</ul>
<p>For each event, the values of the acquisition parameters are read and stored
in the corresponding KVACQParam objects of the INDRA detectors. Therefore to
access the value in your Analysis() method, use something like this:</p>
<pre>gIndra->GetACQParam("CI_0201_PG")->GetCoderData()</pre>
or
<pre>gIndra->GetDetector("CI_0201")->GetACQParam("PG")->GetCoderData()</pre>
<p>Use TriggerInfo to check whether the event corresponds to pulser or laser,
i.e.</p>
<pre>if( TriggerInfo->IsLaser() ) { ... }</pre>
<pre>if( TriggerInfo->IsPulser() ) { ... }</pre>
*/

class KVINDRAGeneDataSelector : public TSelector {
protected :
   TTree*          fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leave types
   Int_t           RunNumber;
   Int_t           EventNumber;
   KVINDRATriggerInfo* TriggerInfo;

   // List of branches
   TBranch*        b_RunNumber;   //!
   TBranch*        b_EventNumber;   //!
   TBranch*        b_TriggerInfo;   //!

   Long64_t fTreeEntry;         //!this is the current TTree entry number, i.e. the argument passed to TSelector::Process(Long64_t entry)
   Bool_t needToCallEndRun;     //! tells if one needs to call EndRun

public :
   KVINDRAGeneDataSelector(TTree* /*tree*/ = 0)
   {
      needToCallEndRun = kFALSE;
   }
   virtual ~KVINDRAGeneDataSelector()
   {
   }
   virtual Int_t   Version() const
   {
      return 2;
   }
   virtual void    Begin(TTree* tree);
   virtual void    SlaveBegin(TTree* tree);
   virtual void    Init(TTree* tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0)
   {
      return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
   }
   virtual void    SetOption(const char* option)
   {
      fOption = option;
   }
   virtual void    SetObject(TObject* obj)
   {
      fObject = obj;
   }
   virtual void    SetInputList(TList* input)
   {
      fInput = input;
   }
   virtual TList*  GetOutputList() const
   {
      return fOutput;
   }
   virtual void    SlaveTerminate();
   virtual void    Terminate();
   virtual void InitAnalysis()
   {
   };
   virtual void InitRun()
   {
   };
   virtual Bool_t Analysis()
   {
      return kTRUE;
   };
   virtual void EndRun()
   {
   };
   virtual void EndAnalysis()
   {
   };
   virtual Bool_t AtEndOfRun(void);
   static void Make(const Char_t* kvsname);
   void SetBranchAddresses();

   ClassDef(KVINDRAGeneDataSelector, 0) //Base class for analysis of INDRA pulser & laser data
};

#endif

#ifdef KVINDRAGeneDataSelector_cxx
void KVINDRAGeneDataSelector::Init(TTree* tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normaly not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   TriggerInfo = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);
}

Bool_t KVINDRAGeneDataSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normaly not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   needToCallEndRun = kTRUE;
   SetBranchAddresses();
   InitRun();                   //user initialisations for run
   return kTRUE;
}

void KVINDRAGeneDataSelector::SetBranchAddresses()
{

   if (!fChain->GetCurrentFile()) {
      Info("SetBranchAddress", "No file open");
      return;
   }
   // Retrieving the pointer to the INDRA detector
   gIndra = (KVINDRA*) fChain->GetCurrentFile()->Get("INDRA");

   if (!gIndra) {
      Info("SetBranchAddress", "INDRA object not found in file");
      return;
   }

   //set branch addresses
//   TIter next_acqpar(gIndra->GetACQParams());
//   KVACQParam* acqpar;
//   while ((acqpar = (KVACQParam*)next_acqpar())) {
//      fChain->SetBranchAddress(acqpar->GetName(), (Short_t**)acqpar->GetDataAddress());
//   }
   fChain->SetBranchAddress("RunNumber", &RunNumber, &b_RunNumber);
   fChain->SetBranchAddress("EventNumber", &EventNumber, &b_EventNumber);
   fChain->SetBranchAddress("TriggerInfo", &TriggerInfo, &b_TriggerInfo);

   //set run number from INDRA object (for use in InitRun)
   RunNumber = gIndra->GetCurrentRunNumber();
}

#endif // #ifdef KVINDRAGeneDataSelector_cxx
