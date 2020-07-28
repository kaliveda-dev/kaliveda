//Created by KVClassFactory on Mon Nov 30 15:00:00 2009
//Author: John Frankland,,,

#include "KVHashList.h"
#include "THashList.h"

ClassImp(KVHashList)


KVHashList::KVHashList(Int_t capacity, Int_t rehash)
   : KVSeqCollection()
{
   // Create a THashList object. Capacity is the initial hashtable capacity
   // (i.e. number of slots), by default kInitHashTableCapacity = 17, and
   // rehash is the value at which a rehash will be triggered. I.e. when the
   // average size of the linked lists at a slot becomes longer than rehash
   // then the hashtable will be resized and refilled to reduce the collision
   // rate to about 1. The higher the collision rate, i.e. the longer the
   // linked lists, the longer lookup will take.
   // The default value of rehash = 2 (minimum allowed value) - automatic
   // rehashing is enabled by default. If rehash=0 the table will
   // NOT automatically be rehashed. Use Rehash() for manual rehashing.
   // WARNING !!!
   // If the name of an object in the HashList is modified, The hashlist
   // must be Rehashed

   fCollection = new THashList(capacity, rehash);
   fCollection->SetName(GetName());
}

KVHashList::~KVHashList()
{
   // Destructor
}

Float_t KVHashList::AverageCollisions() const
{
   // Return the average collision rate. The higher the number the longer
   // the linked lists in the hashtable, the slower the lookup. If the number
   // is high, or lookup noticeably too slow, perfrom a Rehash()

   return ((THashList*)fCollection)->AverageCollisions();
}

void KVHashList::Rehash(Int_t newCapacity)
{
   // Rehash the hashlist.
   //
   // This needs to be done in two cases:
   //   1. If the collision rate becomes too high (i.e. the average size of the
   //   linked lists become too long - use AverageCollisions() to check if you
   //   need to rehash.) then lookup efficiency decreases since relatively long
   //   lists have to be searched every time.
   //   To improve performance rehash the hashtable, increasing
   //   the number of slots. This method resizes the table to newCapacity slots
   //   and refills the table.
   //   2. If the names of any objects in the list CHANGE, you must
   //   rehash the list (as the lookup of objects depends on the TString::Hash()
   //   value calculated from their name when they are added to the list).
   //   In this case it is not necessary to increase the capacity of the list,
   //   just call Rehash() with no arguments.

   if (!newCapacity) newCapacity = fCollection->GetSize();
   ((THashList*)fCollection)->Rehash(newCapacity);
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,5,0)
const TList* KVHashList::GetListForObject(const char* name) const
#else
TList* KVHashList::GetListForObject(const char* name) const
#endif
{
   // Return the THashTable's list (bucket) in which obj can be found based on
   // its hash; see THashTable::GetListForObject().

   return ((THashList*)fCollection)->GetListForObject(name);
}

//______________________________________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,5,0)
const TList* KVHashList::GetListForObject(const TObject* obj) const
#else
TList* KVHashList::GetListForObject(const TObject* obj) const
#endif
{
   // Return the THashTable's list (bucket) in which obj can be found based on
   // its hash; see THashTable::GetListForObject().

   return ((THashList*)fCollection)->GetListForObject(obj);
}
