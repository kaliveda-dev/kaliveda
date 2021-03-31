#ifndef KVLIST_H
#define KVLIST_H

#include "KVSeqCollection.h"
#include "TList.h"

/**
\class KVList
\brief Extended TList class which owns its objects by default
\ingroup Core

This is an extended version of the ROOT TList class, with all of the extensions
defined in KVSeqCollection. The specificity of KVList is that it owns it objects by default:
any objects placed in a KVList will be deleted when the KVList is deleted, goes out of scope, or if
methods Clear() or Delete() are called.

To avoid this, either use the argument to the constructor:
~~~~{.cpp}
KVList list_not_owner(kFALSE); //list does not own its objects
~~~~
or use the method SetOwner() to change the ownership after object creation.

#### Sorting the list
As TList::Sort() is defined, we implemented a KVList::Sort() method which takes exactly
the same argument as TList::Sort().
 */
class KVList: public KVSeqCollection {

public:

   KVList(Bool_t owner = kTRUE);
   virtual ~ KVList() {}

   void Sort(Bool_t order = kSortAscending)
   {
      ((TList*)fCollection)->Sort(order);
   };

   ClassDef(KVList, 3)//Extended version of ROOT TList
};
#endif
