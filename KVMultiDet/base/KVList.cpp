#include "KVList.h"

ClassImp(KVList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVList</h2>
<h4>Extended TList class which owns its objects by default</h4>
This is an extended version of the ROOT TList class, with all of the extensions
defined in KVSeqCollection. The specificity of KVList is that it owns it objects by default:
any objects placed in a KVList will be deleted when the KVList is deleted, goes out of scope, or if
methods Clear() or Delete() are called. To avoid this, either use the
argument to the constructor:
<pre>
<code class=C++>KVList list_not_owner(kFALSE);//list does not own its objects
</code>
</pre>
or use the method SetOwner(kFALSE) to change the ownership.
<h3>Sorting the list</h3>
As TList::Sort() is defined, we implemented a KVList::Sort() method which takes exactly
the same argument as TList::Sort().
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVList::KVList(Bool_t owner) : KVSeqCollection("TList")
{
   // Default ctor.
   // By default, a KVList owns its objects.
   SetOwner(owner);
}

KVList::~KVList()
{
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
      } else
         R__b.ReadClassBuffer(KVList::Class(), this, R__v, R__s, R__c);
   } else {
      R__b.WriteClassBuffer(KVList::Class(), this);
   }
}
