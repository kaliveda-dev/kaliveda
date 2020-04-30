//Created by KVClassFactory on Fri Jun 19 18:51:28 2009
//Author: John Frankland,,,

#ifndef __KVSEQCOLLECTION_H
#define __KVSEQCOLLECTION_H

#include "TSeqCollection.h"
#include <RQ_OBJECT.h>
#include "TFile.h"
#include "KVConfig.h"

/**
\class KVSeqCollection
\brief KaliVeda extensions to ROOT collection classes
\ingroup Core

This class adds functionalities such as
 - FindObjectByType()
 - FindObjectByLabel()
 - GetSublistWithType()

etc. to the standard ROOT collection classes. The actual collection is embedded and
referenced through a TSeqCollection base pointer. The class of the embedded object for any given
instance is passed as an argument to the constructor:
~~~~{.cpp}
KVSeqCollection my_coll("THashList");
~~~~
Any collection class derived from TSeqCollection is valid: this means all ordered collections, for which the order in which
objects are added is preserved. Some of these lists can also be sorted (see below).

Any object derived from TObject can be stored in the collection, but only objects
derived from KVBase can be sought using their 'type', 'label', or 'number' attributes
(these characteristics are not defined for TObject).

Note that we can test the 'KVBase'-ness of any object through a
TObject* base pointer using the 'KaliVeda' bit, KVBase::kIsKaliVedaObject:
~~~~{.cpp}
TObject *obj = [address of some object];
if( obj->TestBit( KVBase::kIsKaliVedaObject ) ){
   // => object derives from KVBase
}
~~~~

### Signals & slots: monitoring list modification
If SendModifiedSignals() is called, then every time an object is added to or removed from the list, it will emit
the signal Modified(). This can be used to monitor any changes to the
state of the list.

### FindObjectBy...
In addition to the standard TList::FindObject(const Char_t*) and
TList::FindObject(TObject*) methods, KVSeqCollection adds methods to find
objects based on several different properties, such as type, label,
number, or class. Note that type, label and number are only defined for
objects derived from KVBase; if the list contains a mixture of TObject-
and KVBase-derived objects, the TObject-derived objects will be ignored
if type, label or number are used to search.

Finally, the very general FindObjectWithMethod() method can search for an
object using any valid method.

### Sublists
The GetSubListWith... methods allow to generate new lists containing a
subset of the objects in the main list, according to their name, label,
type, etc. These sublists do not own their objects (they are supposed
to be owned by the main list), and must be deleted by the user in order
to avoid memory leaks.

### Execute
The Execute methods can be used to execute a given method with the same
arguments for every object in the list.

### Sorting lists
No Sort() method is defined in base class TSeqCollection, although the TCollection::IsSortable()
method returns kTRUE by default (and is not overridden in child classes which do not implement
sorting). Therefore whether or not a KVSeqCollection can be sorted depends on the collection
class which is being used. We do not define a KVSeqCollection::Sort() method. If you want to sort
a KVSeqCollection and you know that the embedded collection class has a valid Sort() method,
you can do as follows:
~~~~{.cpp}
KVSeqCollection seqlist("TList");
seqlist.Add(...);
...
((TList*)seqlist.GetCollection())->Sort();
~~~~
or perhaps more cautiously you should do:
~~~~{.cpp}
TList* tlist = dynamic_cast<TList*>(seqlist.GetCollection());
if(tlist) tlist->Sort();
~~~~
*/
class KVSeqCollection : public TSeqCollection {
   RQ_OBJECT("KVSeqCollection")

   enum {
      kSignals = BIT(15), // bit flag for sending 'Modified()' signal on changes
      // in ROOT v6 BIT(16) is used by TCollection - without changing the class version
      kCleanup = BIT(17) // set when objects in list are in ROOT cleanup list
   };

   static Long64_t fSCCounter; // counter used to give unique names to all lists
   void init();
   static Int_t fgCounter;// counts instances
   static TSeqCollection* fgCleanups;// regroup all lists which are to be cleaned up

protected:
   TSeqCollection* fCollection;//Pointer to embedded ROOT collection

   virtual void Changed()
   {
      // Called by Add/RemoveObject, Clear and Delete.
      // We make it emit the "Modified()" signal, if
      // the kSignals bit has been set using SendModifiedSignals(kTRUE).

      TSeqCollection::Changed();
      if (TestBit(kSignals)) Modified();
   };
   virtual void  PrintCollectionHeader(Option_t* option) const;
   virtual KVSeqCollection* NewCollectionLikeThisOne() const;
   virtual void SetCollection(const Char_t*);
   virtual void _GetSubListWithMethod(KVSeqCollection*, TCollection*, const Char_t*, const Char_t*) const;
   void _GetSubListWithClass(KVSeqCollection*, TCollection*, const TClass*) const;

public:
   KVSeqCollection();
   KVSeqCollection(const KVSeqCollection&);
   KVSeqCollection(const Char_t* collection_classname);
   virtual ~KVSeqCollection();
   virtual void Copy(TObject& obj) const;
   KVSeqCollection& operator=(const KVSeqCollection&);

   virtual void Modified()
   {
      Emit("Modified()");
   }  // *SIGNAL*
   virtual void SendModifiedSignals(Bool_t yes = kTRUE)
   {
      // yes=kTRUE: turns on signals-and-slots mechanism, list will
      //           emit 'Modified()' signal every time a change
      //           occurs (objects added or removed).
      // yes=kFALSE: turns off signals-and-slots mechanism
      //
      // By default, the 'Modified()' signal is NOT enabled.
      SetBit(kSignals, yes);
   }
   virtual Bool_t IsSendingModifiedSignals() const
   {
      // returns kTRUE if 'Modified()' signal is active
      // (see SendModifiedSignals).
      return TestBit(kSignals);
   }

   virtual TObject* At(Int_t idx) const
   {
      return fCollection->At(idx);
   }
   virtual void      AddFirst(TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddFirst(obj);
      Changed();
   }
   virtual void      AddLast(TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddLast(obj);
      Changed();
   }
   virtual void      AddAt(TObject* obj, Int_t idx)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddAt(obj, idx);
      Changed();
   }
   virtual void      AddAfter(const TObject* after, TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddAfter(after, obj);
      Changed();
   }
   virtual void      AddBefore(const TObject* before, TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddBefore(before, obj);
      Changed();
   }
   virtual TObject*  Before(const TObject* obj) const
   {
      return fCollection->Before(obj);
   }
   virtual TObject*  After(const TObject* obj) const
   {
      return fCollection->After(obj);
   }
   virtual TObject*  First() const
   {
      return fCollection->First();
   }
   virtual TObject*  Last() const
   {
      return fCollection->Last();
   }
   virtual void       Add(TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->Add(obj);
      Changed();
   }
   virtual void       Clear(Option_t* option = "");
   virtual void       Delete(Option_t* option = "");
   virtual TObject**  GetObjectRef(const TObject* obj) const;
   virtual TIterator* MakeIterator(Bool_t dir = kIterForward) const;
   virtual TObject*   Remove(TObject* obj);
   virtual void RecursiveRemove(TObject* obj);
   virtual Int_t GetSize() const
   {
      return fCollection ? fCollection->GetSize() : 0;
   }
   virtual void SetOwner(Bool_t enable = kTRUE)
   {
      //if(enable&&IsCleanup()) Warning("SetOwner","List %s will be both owner & cleanup",GetName());
      TSeqCollection::SetOwner(enable);
      fCollection->SetOwner(enable);
   }
   const Char_t* CollectionClassName() const
   {
      // Return classname of embedded collection object
      return fCollection->ClassName();
   }
   virtual void SetCleanup(Bool_t enable = kTRUE);
   virtual Bool_t IsCleanup() const
   {
      // Returns kTRUE if the ROOT cleanup mechanism is used to ensure that any objects in the list
      // which get deleted elsewhere are removed from this list
      return TestBit(kCleanup);
   }
   static void RehashCleanupList();

   virtual TObject* FindObject(const char* name) const
   {
      return fCollection->FindObject(name);
   }
   template
   <typename T>
   T* get_object(const TString& name) const
   {
      return dynamic_cast<T*>(FindObject(name));
   }
   virtual TObject* FindObject(const TObject* obj) const
   {
      return fCollection->FindObject(obj);
   }
   virtual TObject* FindObjectByName(const Char_t* name) const
   {
      return fCollection->FindObject(name);
   }
   virtual TObject* FindObjectByType(const Char_t*) const;
   virtual TObject* FindObjectByTitle(const Char_t*) const;
   TObject* FindObjectByClass(const Char_t*) const;
   TObject* FindObjectByClass(const TClass*) const;
   virtual TObject* FindObjectByLabel(const Char_t*) const;
   virtual TObject* FindObjectByNumber(UInt_t num) const;
   virtual TObject* FindObjectWithNameAndType(const Char_t* name, const Char_t* type) const;
   virtual TObject* FindObjectWithMethod(const Char_t* retvalue, const Char_t* method) const;
   virtual TObject* FindObjectAny(const Char_t* att, const Char_t* keys, Bool_t contains_all = kFALSE, Bool_t case_sensitive = kTRUE) const;

#ifdef __WITHOUT_TCOLLECTION_GETENTRIES
   //we add the GetEntries method added to TCollection from ROOT v4.03/04 onwards
   virtual Int_t GetEntries() const
   {
      return GetSize();
   };
#endif
   virtual void Execute(const char* method, const char* params,
                        Int_t* error = 0);
   virtual void Execute(TMethod* method, TObjArray* params,
                        Int_t* error = 0);
   KVSeqCollection* GetSubListWithMethod(const Char_t* retvalue, const Char_t* method) const;

   KVSeqCollection* GetSubListWithClass(const TClass* _class) const;
   KVSeqCollection* GetSubListWithClass(const Char_t* class_name) const;

   KVSeqCollection* GetSubListWithName(const Char_t* retvalue) const;
   KVSeqCollection* GetSubListWithLabel(const Char_t* retvalue) const;
   KVSeqCollection* GetSubListWithType(const Char_t* retvalue) const;

   static KVSeqCollection* MakeListFromFile(TFile* file);
   static KVSeqCollection* MakeListFromFileWithMethod(TFile* file, const Char_t* retvalue, const Char_t* method);
   static KVSeqCollection* MakeListFromFileWithClass(TFile* file, const TClass* _class);
   static KVSeqCollection* MakeListFromFileWithClass(TFile* file, const Char_t* class_name);

   virtual Bool_t IsSortable() const
   {
      return fCollection->IsSortable();
   }
   virtual Bool_t IsSorted() const
   {
      return fCollection->IsSorted();
   }

   TSeqCollection* GetCollection() const
   {
      return fCollection;
   }

   ClassDef(KVSeqCollection, 3) //KaliVeda extensions to ROOT collections
};

#if ROOT_VERSION_CODE < ROOT_VERSION(5,11,2)

//---- R__FOR_EACH macro -----------------------------------------------------------

// Macro to loop over all elements of a list of type "type" while executing
// procedure "proc" on each element

#define R__FOR_EACH(type,proc) \
   SetCurrentCollection(); \
   TIter _NAME3_(nxt_,type,proc)(TCollection::GetCurrentCollection()); \
   type *_NAME3_(obj_,type,proc); \
   while ((_NAME3_(obj_,type,proc) = (type*) _NAME3_(nxt_,type,proc)())) \
      _NAME3_(obj_,type,proc)->proc

#endif

#endif
