//Created by KVClassFactory on Thu Dec  3 15:49:33 2009
//Author: John Frankland,,,

#include "KVUniqueNameList.h"

ClassImp(KVUniqueNameList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVUniqueNameList</h2>
<h4>Optimised list in which objects can only be placed once</h4>
<!-- */
// --> END_HTML
// Use this list when you need to check as each object is added that it is not already in
// the list (assuming that objects are the same if they have the same name), in order to
// avoid the same object being added several times.
//
// Using a standard TList/KVList, implementing such a check can become VERY time-consuming
// as the list becomes longer, because for each new object we add we have to perform a
// sequential scan of all objects in the list and check the name of each one. However, using
// a hash list based on object names, the number of objects to check is kept small, even
// as the list becomes very long, thanks to automatic rehashing of the list.
//
// By default, if an object with the same name is already in the list, the
// 'new' object is not added (no warning, no error).
// If option 'replace' is used, then adding an object with the same name
// as an existing object will replace the previous object with the new one.
// In addition, if the list owns its objects and 'replace' is used, the previous
// object will be deleted as well as being replaced.
//
// ============== EXAMPLES ===============
//
// TNamed* a = new TNamed("A1", "an object");
// TNamed* b = new TNamed("B1", "another object");
// TNamed* c = a;
// TNamed* d = new TNamed("A1", "an object with the same name as another");
//
// === Example 0 - normal TList
//
// TList aList;
// aList.Add(a);
// aList.Add(b);
// aList.Add(c);
// aList.Add(d);
// aList.ls();
// OBJ: TList  TList Doubly linked list : 0
//  OBJ: TNamed   A1 an object : 0 at: 0x6c21770
//  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40
//  OBJ: TNamed   A1 an object : 0 at: 0x6c21770
//  OBJ: TNamed   A1 an object with the same name as another : 0 at: 0x6c0acf0
//
// === Example 1 - unique name list, no replace
//
// KVUniqueNameList UNlist1;
// UNlist1.Add(a);
// UNlist1.Add(b);
// UNlist1.Add(c);
// UNlist1.Add(d);
// UNlist1.ls();
// OBJ: KVUniqueNameList   KVUniqueNameList  Optimised list in which objects can only be placed once : 0
//  OBJ: TNamed   A1 an object : 0 at: 0x6c21770
//  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40//
//
// === Example 2 - unique name list, with replace
//
// KVUniqueNameList UNlist2(kTRUE);
// UNlist2.Add(a);
// UNlist2.Add(b);
// UNlist2.Add(c);
// UNlist2.Add(d);
// UNlist2.ls();
// OBJ: KVUniqueNameList   KVUniqueNameList  Optimised list in which objects can only be placed once : 0
//  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40
//  OBJ: TNamed   A1 an object with the same name as another : 0 at: 0x6c0acf0
//
// === Example 3 - unique name list which owns its objects, with replace
//
// KVUniqueNameList UNlist3(kTRUE);
// UNlist3.Add(a);
// UNlist3.Add(b);
// UNlist3.Add(d);
// UNlist3.ls();
// OBJ: KVUniqueNameList   KVUniqueNameList  Optimised list in which objects can only be placed once : 0
//  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40
//  OBJ: TNamed   A1 an object with the same name as another : 0 at: 0x6c0acf0
//
// a->ls();
//
//  *** Break *** segmentation violation  (1st object "A1" has been deleted)
//
// N.B.: in the last example we do NOT do UNlist3.Add(c) which would have
// deleted the object pointed to by both *a and *c, and then tried to
// add this deleted object back in to the list
////////////////////////////////////////////////////////////////////////////////

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
