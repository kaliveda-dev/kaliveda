#include "KVList.h"

ClassImp(KVList)

KVList::KVList(Bool_t owner) : KVSeqCollection("TList")
{
   // Default ctor.
   // By default, a KVList owns its objects.
   SetOwner(owner);
}

#include "Riostream.h"
void KVList::Streamer(TBuffer& R__b)
{
   // Backwards-compatible streamer

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v < 3) {
         // read in old KVList object which inherited from TList
         TList list;
         list.Streamer(R__b);
         TQObjSender fQObj;
         fQObj.Streamer(R__b);
         SetOwner(list.IsOwner()); // was list owner of objects ?
         list.SetOwner(kFALSE); // make sure old list doesn't delete objects when it goes out of scope
         TIter next(&list);
         TObject* obj;
         while ((obj = next())) Add(obj);
         list.Clear();
      }
      else
         R__b.ReadClassBuffer(KVList::Class(), this, R__v, R__s, R__c);
   }
   else {
      R__b.WriteClassBuffer(KVList::Class(), this);
   }
}
