//Created by KVClassFactory on Mon Nov 30 15:00:00 2009
//Author: John Frankland,,,

#ifndef __KVHASHLIST_H
#define __KVHASHLIST_H

#include "KVSeqCollection.h"
#include "THashList.h"
#include "RVersion.h"

/**
\class  KVHashList
\brief Extended version of ROOT THashList
\ingroup Core

This collection class uses a THashList for quick look-up of objects based on
the TString::Hash() value of their name, and adds all the extra functionality
defined in KVSeqCollection.

Automatic rehashing of the list is enabled by default
(with rehash level = 2), unlike THashList (disabled by default).
Beware if objects' names change or are changed after being added to the list: a manual Rehash() is required.

#### Sorting the list
As THashList::Sort() is defined, we implemented a KVHashList::Sort() method which takes exactly
the same argument as THashList::Sort().
 */
class KVHashList : public KVSeqCollection {

public:
   KVHashList(Int_t capacity = TCollection::kInitHashTableCapacity, Int_t rehash = 2);
   virtual ~KVHashList();

   Float_t AverageCollisions() const;
   void Rehash(Int_t newCapacity = 0);
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,5,0)
   const TList*     GetListForObject(const char* name) const;
   const TList*     GetListForObject(const TObject* obj) const;
#else
   TList*     GetListForObject(const char* name) const;
   TList*     GetListForObject(const TObject* obj) const;
#endif
   void Sort(Bool_t order = kSortAscending)
   {
      ((THashList*)fCollection)->Sort(order);
   };
   template<typename T> Bool_t ContainsObjectWithName(const T& o)
   {
      // Returns true if list contains an object with the same name as o
      // class T must have a method const char* T::GetName() const
      return Contains(o.GetName());
   }

   ClassDef(KVHashList, 1) //Extended version of ROOT THashList
};

#endif
