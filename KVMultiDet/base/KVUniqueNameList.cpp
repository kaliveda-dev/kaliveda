//Created by KVClassFactory on Thu Dec  3 15:49:33 2009
//Author: John Frankland,,,

#include "KVUniqueNameList.h"

ClassImp(KVUniqueNameList)



KVUniqueNameList::KVUniqueNameList(Bool_t R)
   : fReplace(R)
{
   // Default constructor
}

KVUniqueNameList::~KVUniqueNameList()
{
   // Destructor
}

Bool_t KVUniqueNameList::checkObjInList(TObject* obj)
{
   // Return kTRUE if 'obj' can be added to list.
   // If fReplace==kFALSE, there must be no object in the list with
   // same name as obj->GetName().
   // If fReplace==kTRUE the previous object is removed from the list.
   // If IsOwner()==kTRUE the previous object is removed AND deleted.
   // In the latter two cases the new object can always be added (returns kTRUE)

   TObject* old_obj = FindObject(obj->GetName());
   if (!old_obj) return kTRUE;
   else {
      if (fReplace) {
         Remove(old_obj);
         if (IsOwner()) delete old_obj;
         return kTRUE;
      }
   }
   return kFALSE;
}
