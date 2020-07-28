//Created by KVClassFactory on Thu Dec  3 15:49:33 2009
//Author: John Frankland,,,

#ifndef __KVUNIQUENAMELIST_H
#define __KVUNIQUENAMELIST_H

#include "KVHashList.h"

/**
\class KVUniqueNameList
\brief Optimised list in which named objects can only be placed once

 Use this list when you need to check as each object is added that it is not already in
 the list (assuming that objects are the same if they have the same name), in order to
 avoid the same object being added several times.

 Using a standard TList or KVList, implementing such a check can become *very* time-consuming
 as the list becomes longer, because for each new object we add we have to perform a
 sequential scan of all objects in the list and check the name of each one. However, using
 a KVHashList based on object names, the number of objects to check is kept small, even
 as the list becomes very long, thanks to automatic rehashing of the list.

 By default, if an object with the same name is already in the list, the
 'new' object is not added (no warning, no error).

 If option 'replace' is used, then adding an object with the same name
 as an existing object will replace the previous object with the new one.

 In addition, if the list owns its objects and 'replace' is used, the previous
 object will be deleted as well as being replaced.

 After calling Add(TObject*), you can call method ObjectAdded() to know if the object
 was effectively added to the list or not.

 ### EXAMPLES
 ~~~~{.cpp}
 TNamed* a = new TNamed("A1", "an object");
 TNamed* b = new TNamed("B1", "another object");
 TNamed* c = a;
 TNamed* d = new TNamed("A1", "an object with the same name as another");

 === Example 0 - normal TList

 TList aList;
 aList.Add(a);
 aList.Add(b);
 aList.Add(c);
 aList.Add(d);
 aList.ls();
 OBJ: TList  TList Doubly linked list : 0
  OBJ: TNamed   A1 an object : 0 at: 0x6c21770
  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40
  OBJ: TNamed   A1 an object : 0 at: 0x6c21770
  OBJ: TNamed   A1 an object with the same name as another : 0 at: 0x6c0acf0

 === Example 1 - unique name list, no replace

 KVUniqueNameList UNlist1;
 UNlist1.Add(a);
 UNlist1.Add(b);
 UNlist1.Add(c);
 UNlist1.Add(d);
 UNlist1.ls();
 OBJ: KVUniqueNameList   KVUniqueNameList  Optimised list in which objects can only be placed once : 0
  OBJ: TNamed   A1 an object : 0 at: 0x6c21770
  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40//

 === Example 2 - unique name list, with replace

 KVUniqueNameList UNlist2(kTRUE);
 UNlist2.Add(a);
 UNlist2.Add(b);
 UNlist2.Add(c);
 UNlist2.Add(d);
 UNlist2.ls();
 OBJ: KVUniqueNameList   KVUniqueNameList  Optimised list in which objects can only be placed once : 0
  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40
  OBJ: TNamed   A1 an object with the same name as another : 0 at: 0x6c0acf0

 === Example 3 - unique name list which owns its objects, with replace

 KVUniqueNameList UNlist3(kTRUE);
 UNlist3.Add(a);
 UNlist3.Add(b);
 UNlist3.Add(d);
 UNlist3.ls();
 OBJ: KVUniqueNameList   KVUniqueNameList  Optimised list in which objects can only be placed once : 0
  OBJ: TNamed   B1 another object : 0 at: 0x6c0fc40
  OBJ: TNamed   A1 an object with the same name as another : 0 at: 0x6c0acf0

 a->ls();

  *** Break *** segmentation violation  (1st object "A1" has been deleted)
~~~~
 N.B.: in the last example we do NOT do UNlist3.Add(c) which would have
 deleted the object pointed to by both `a` and `c`, and then tried to
 add this deleted object back in to the list
*/
class KVUniqueNameList : public KVHashList {
   Bool_t fReplace;// if kTRUE, objects with same name are replaced
   mutable Bool_t fLastCheck;//! return value of last call to checkobj

   Bool_t checkObjInList(TObject* obj);

public:
   KVUniqueNameList(Bool_t R = kFALSE);
   virtual ~KVUniqueNameList();
   virtual void      AddFirst(TObject* obj)
   {
      // Add an object to the list if it is not already in it
      // (no object with same name in list)

      if ((fLastCheck = checkObjInList(obj))) KVHashList::AddFirst(obj);
   };
   virtual void      AddLast(TObject* obj)
   {
      // Add an object to the list if it is not already in it
      // (no object with same name in list)

      if ((fLastCheck = checkObjInList(obj))) KVHashList::AddLast(obj);
   };
   virtual void      AddAt(TObject* obj, Int_t idx)
   {
      // Add an object to the list if it is not already in it
      // (no object with same name in list)

      if ((fLastCheck = checkObjInList(obj))) KVHashList::AddAt(obj, idx);
   };
   virtual void      AddAfter(const TObject* after, TObject* obj)
   {
      // Add an object to the list if it is not already in it
      // (no object with same name in list)

      if ((fLastCheck = checkObjInList(obj))) KVHashList::AddAfter(after, obj);
   };
   virtual void      AddBefore(const TObject* before, TObject* obj)
   {
      // Add an object to the list if it is not already in it
      // (no object with same name in list)

      if ((fLastCheck = checkObjInList(obj))) KVHashList::AddBefore(before, obj);
   };
   virtual void       Add(TObject* obj)
   {
      // Add an object to the list if it is not already in it
      // (no object with same name in list)

      if ((fLastCheck = checkObjInList(obj))) KVHashList::Add(obj);
   };

   Bool_t ObjectAdded() const
   {
      // returns kTRUE if the last (immediately previous) attempt ot
      // add an object to the list was successful

      Bool_t save = fLastCheck;
      fLastCheck = kFALSE;
      return save;
   }
   void ReplaceObjects(Bool_t yes = kTRUE)
   {
      fReplace = yes;
   }

   ClassDef(KVUniqueNameList, 2) //Optimised list in which objects with the same name can only be placed once
};

#endif
