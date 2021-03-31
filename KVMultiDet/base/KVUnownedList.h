#ifndef KVUNOWNEDLIST_H
#define KVUNOWNEDLIST_H

#include "KVList.h"

/**
\class KVUnownedList
\brief Extended TList class which does not own its objects by default
\ingroup Core

This is an extended version of the ROOT TList class, with all of the extensions
defined in KVSeqCollection. Unlike KVList, it does not own its objects by default
they will not be deleted when this list goes out of scope, Clear() is called, etc.
 creation.
 */
class KVUnownedList : public KVList {
public:
   KVUnownedList()
      : KVList(false)
   {}

   ClassDef(KVUnownedList, 1)
};

#endif // KVUNOWNEDLIST_H
