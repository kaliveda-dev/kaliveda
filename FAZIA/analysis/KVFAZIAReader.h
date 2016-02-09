//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Nov 29 14:37:16 2013 by ROOT version 5.34/08
// from TTree tree_PSA_signals/Tree for PSA
// found on file: root://ccxroot.in2p3.fr:1999//hpss/in2p3.fr/group/fazia/LNS_2011/raw/run_4023.root
//////////////////////////////////////////////////////////

#ifndef KVFAZIAReader_h
#define KVFAZIAReader_h

#include <TChain.h>
#include <TSelector.h>
#include <KVFAZIARawEvent.h>
#include <KVDetectorEvent.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class KVFAZIAReader : public TSelector {

   TTree* fChain;   //!pointer to the analyzed TTree or TChain
   Int_t fEventNumber;
   Int_t fCurrentRun;
   Int_t fReadEntries;
   KVFAZIARawEvent* RawEvent;
   KVDetectorEvent* fDetEv;

public :
   KVFAZIAReader() : fChain(0)
   {
      fCurrentRun = -1;
      RawEvent = 0;
      fDetEv = new KVDetectorEvent();
   }
   virtual ~KVFAZIAReader()
   {
      SafeDelete(fDetEv);
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
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0);

   virtual void    SlaveTerminate();
   virtual void    Terminate();

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

   /* methods to be derived in child classes*/
   virtual void InitAnalysis() {}            //Called by Init()
   virtual void InitRun() {}                 //Called by Notify()
   virtual Bool_t Analysis()
   {
      return kTRUE;  //Called by Process()
   }
   virtual void EndRun() {}                  //Called by Notify() and Terminate()
   virtual void EndAnalysis() {}             //Called by Terminate()
   //

   Int_t GetEventNumber()
   {
      return fEventNumber;
   }
   Int_t GetCurrentRunNumber()
   {
      return fCurrentRun;
   }
   Int_t GetNumberOfReadEntries()
   {
      return fReadEntries;
   }

   KVFAZIARawEvent* GetEvent() const
   {
      return RawEvent;
   }
   KVDetectorEvent* GetDetectorEvent() const
   {
      return fDetEv;
   }

   static void Make(const Char_t* kvsname = "MyOwnKVFAZIAReader");

   ClassDef(KVFAZIAReader, 0);
};

#endif
