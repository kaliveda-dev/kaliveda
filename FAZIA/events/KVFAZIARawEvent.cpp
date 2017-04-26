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
   //Info("Clear","IsCalled");
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
//________________________________________________________________
const Char_t* KVFAZIARawEvent::GetFPGAEnergy(Int_t blk, Int_t qua, Int_t tel, TString signaltype, Int_t idx)
{

   TString sene = "";
//   sene.Form("ENER%d-B%03d-Q%d-T%d-%s", idx, blk, qua, tel, signaltype.Data()); -> old detector names...
   sene.Form("ENER%d-%s-%d", idx, signaltype.Data(), 100 * blk + 10 * qua + tel);
   //rustines for RUTHERFORD Telescope
   if (blk == 0 && qua == 0 && tel == 0)
      sene.Form("ENER%d-RUTH-%s", idx, signaltype.Data());

   if (fValues->HasParameter(sene.Data()))
      return fValues->GetStringValue(sene.Data());
   else
      return "0.0";

}

//________________________________________________________________
void KVFAZIARawEvent::Streamer(TBuffer& R__b)
{
   // Customised Streamer for KVFAZIARawEvent.
   // This is just the automatic Streamer with the addition of a call to the Clear()
   // method before reading a new object (avoid memory leaks with lists of parameters).

   if (R__b.IsReading()) {
      Clear();
      R__b.ReadClassBuffer(KVFAZIARawEvent::Class(), this);
   } else {
      R__b.WriteClassBuffer(KVFAZIARawEvent::Class(), this);
   }
}
