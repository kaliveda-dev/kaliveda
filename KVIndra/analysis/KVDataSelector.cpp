/*
$Id: KVDataSelector.cpp,v 1.4 2007/05/31 09:59:21 franklan Exp $
$Revision: 1.4 $
$Date: 2007/05/31 09:59:21 $
*/

//Created by KVClassFactory on Fri Oct 20 13:58:16 2006
//Author: Daniel CUSSOL

#include "KVDataSelector.h"
#include "TEventList.h"
#include "KVOldINDRASelector.h"
#include "KVClassFactory.h"

using namespace std;

ClassImp(KVDataSelector)

KVDataSelector::KVDataSelector()
{
   //Default constructor
   evtl = new TEventList();
}

//_____________________________________
KVDataSelector::~KVDataSelector()
{
   //Destructor
   delete evtl;
}

//_____________________________________
void KVDataSelector::Reset(Int_t nrun)
{
//
// Resets the event list. This method is called when a new TTree is loaded
//
   evtl->Reset();
   evtl->SetName(Form("%s_run%d", this->IsA()->GetName(), nrun));
   evtl->
   SetTitle(Form
            ("TEventList for the \"%s\" KVDataSelector and run %d.",
             this->IsA()->GetName(), nrun));
   cout << evtl->GetName() << " initialised." << endl;
}

//_____________________________________
Bool_t KVDataSelector::ProcessCurrentEntry(void)
{
//
// Process the current entry
// if the SelectCurrentEntry method return kTRUE, the current entry number
// is added to the event list
// This method is called just before the Analysis method
//
   Bool_t ok = kFALSE;
   if (!kvsel) {
      Warning("ProcessCurrentEntry(void)", "The KVOldINDRASelector is not set");
   }
   if (SelectCurrentEntry()) {
      ok = kvsel->Analysis();
      evtl->Enter(kvsel->GetTreeEntry());
   }
   return ok;
}

//_____________________________________
void KVDataSelector::Init(void)
{
//
// Initialisations of the KVDataSelector
// This method is called before the InitAnalysis method of the KVOldINDRASelector
//
}


//_____________________________________
Bool_t KVDataSelector::SelectCurrentEntry(void)
{
//
// returns Ok is the current entry has to be added to the TEventList
//
   Bool_t ok = kFALSE;

   return ok;
}

//_________________________________________________________________
void KVDataSelector::MakeClass(const Char_t* classname,
                               const Char_t* classdesc)
{
   //Creates skeleton '.h' and '.cpp' files for a new data selector class which
   //inherits from this class. Give a name for the new class and a short description
   //which will be used to document the class.
   KVClassFactory cf(classname, classdesc, "KVDataSelector", kTRUE);
   cf.AddImplIncludeFile("KVINDRA.h");
   cf.AddImplIncludeFile("KVOldINDRASelector.h");
   cf.GenerateCode();
}
