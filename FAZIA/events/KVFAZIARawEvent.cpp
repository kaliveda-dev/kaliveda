//Created by KVClassFactory on Wed Jul 15 11:54:35 2015
//Author: ,,,

#include "KVFAZIARawEvent.h"

ClassImp(KVFAZIARawEvent)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIARawEvent</h2>
<h4>Handle list of signals and additional information coming from acquisition file</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIARawEvent::KVFAZIARawEvent(Int_t ntot)
{
   // Default constructor
   fSignals = new TClonesArray("KVSignal", ntot);
   fValues = new KVNameValueList();

   fNumber = -1;
}

KVFAZIARawEvent::KVFAZIARawEvent()
{
   // Default constructor
   fSignals = new TClonesArray("KVSignal", 10);
   fValues = new KVNameValueList();

   fNumber = -1;
}

KVFAZIARawEvent::~KVFAZIARawEvent()
{
   // Destructor
   delete fSignals;
   delete fValues;
}

//________________________________________________________________

void KVFAZIARawEvent::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVFAZIARawEvent::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TObject::Copy(obj);
   //KVFAZIARawEvent& CastedObj = (KVFAZIARawEvent&)obj;
}

//________________________________________________________________
void KVFAZIARawEvent::Clear(Option_t*)
{
   fSignals->Clear();
   fValues->Clear();
}

//________________________________________________________________
KVSignal* KVFAZIARawEvent::AddNewSignal(KVString name, KVString title)
{
   KVSignal* sig = (KVSignal*)fSignals->ConstructedAt(fSignals->GetEntries());
   sig->SetNameTitle(name.Data(), title.Data());
   return sig;

}
