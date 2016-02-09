#ifndef KVLIST_H
#define KVLIST_H

#include "KVSeqCollection.h"
#include "TList.h"

class KVList: public KVSeqCollection {

public:

   KVList(Bool_t owner = kTRUE);
   virtual ~ KVList();

   void Sort(Bool_t order = kSortAscending)
   {
      ((TList*)fCollection)->Sort(order);
   };

   ClassDef(KVList, 3)//Extended version of ROOT TList
};
#endif
