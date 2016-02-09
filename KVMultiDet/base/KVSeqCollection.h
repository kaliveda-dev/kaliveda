//Created by KVClassFactory on Fri Jun 19 18:51:28 2009
//Author: John Frankland,,,

#ifndef __KVSEQCOLLECTION_H
#define __KVSEQCOLLECTION_H

#include "TSeqCollection.h"
#include <RQ_OBJECT.h>
#include "TFile.h"
#include "KVConfig.h"

class KVSeqCollection : public TSeqCollection {
   RQ_OBJECT("KVSeqCollection")

   enum {
      kSignals = BIT(15), // bit flag for sending 'Modified()' signal on changes
      kCleanup = BIT(16) // set when objects in list are in ROOT cleanup list
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
   };  // *SIGNAL*
   virtual void SendModifiedSignals(Bool_t yes = kTRUE)
   {
      // yes=kTRUE: turns on signals-and-slots mechanism, list will
      //           emit 'Modified()' signal every time a change
      //           occurs (objects added or removed).
      // yes=kFALSE: turns off signals-and-slots mechanism
      //
      // By default, the 'Modified()' signal is NOT enabled.
      SetBit(kSignals, yes);
   };
   virtual Bool_t IsSendingModifiedSignals() const
   {
      // returns kTRUE if 'Modified()' signal is active
      // (see SendModifiedSignals).
      return TestBit(kSignals);
   };

   virtual TObject* At(Int_t idx) const
   {
      return fCollection->At(idx);
   };
   virtual void      AddFirst(TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddFirst(obj);
      Changed();
   };
   virtual void      AddLast(TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddLast(obj);
      Changed();
   };
   virtual void      AddAt(TObject* obj, Int_t idx)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddAt(obj, idx);
      Changed();
   };
   virtual void      AddAfter(const TObject* after, TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddAfter(after, obj);
      Changed();
   };
   virtual void      AddBefore(const TObject* before, TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->AddBefore(before, obj);
      Changed();
   };
   virtual TObject*  Before(const TObject* obj) const
   {
      return fCollection->Before(obj);
   };
   virtual TObject*  After(const TObject* obj) const
   {
      return fCollection->After(obj);
   };
   virtual TObject*  First() const
   {
      return fCollection->First();
   };
   virtual TObject*  Last() const
   {
      return fCollection->Last();
   };
   virtual void       Add(TObject* obj)
   {
      // Add an object to the list.
      // If the cleanup mechanism is in use, we set the TObject::kMustCleanup bit
      // of the object before adding it to the list.

      if (IsCleanup()) obj->SetBit(kMustCleanup);
      fCollection->Add(obj);
      Changed();
   };
   virtual void       Clear(Option_t* option = "");
   virtual void       Delete(Option_t* option = "");
   virtual TObject**  GetObjectRef(const TObject* obj) const;
   virtual TIterator* MakeIterator(Bool_t dir = kIterForward) const;
   virtual TObject*   Remove(TObject* obj);
   virtual void RecursiveRemove(TObject* obj);
   virtual Int_t GetSize() const
   {
      return fCollection->GetSize();
   };
   virtual void SetOwner(Bool_t enable = kTRUE)
   {
      TSeqCollection::SetOwner(enable);
      fCollection->SetOwner(enable);
   };
   const Char_t* CollectionClassName() const
   {
      // Return classname of embedded collection object
      return fCollection->ClassName();
   };
   virtual void SetCleanup(Bool_t enable = kTRUE);
   virtual Bool_t IsCleanup() const
   {
      // Returns kTRUE if the ROOT cleanup mechanism is used to ensure that any objects in the list
      // which get deleted elsewhere are removed from this list
      return TestBit(kCleanup);
   };

   virtual TObject* FindObject(const char* name) const
   {
      return fCollection->FindObject(name);
   };
   virtual TObject* FindObject(const TObject* obj) const
   {
      return fCollection->FindObject(obj);
   };
   virtual TObject* FindObjectByName(const Char_t* name) const
   {
      return fCollection->FindObject(name);
   };
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
   virtual KVSeqCollection* GetSubListWithMethod(const Char_t* retvalue, const Char_t* method) const;

   KVSeqCollection* GetSubListWithClass(const TClass* _class) const;
   KVSeqCollection* GetSubListWithClass(const Char_t* class_name) const;

   virtual KVSeqCollection* GetSubListWithName(const Char_t* retvalue) const;
   virtual KVSeqCollection* GetSubListWithLabel(const Char_t* retvalue) const;
   virtual KVSeqCollection* GetSubListWithType(const Char_t* retvalue) const;

   static KVSeqCollection* MakeListFromFile(TFile* file);
   static KVSeqCollection* MakeListFromFileWithMethod(TFile* file, const Char_t* retvalue, const Char_t* method);
   static KVSeqCollection* MakeListFromFileWithClass(TFile* file, const TClass* _class);
   static KVSeqCollection* MakeListFromFileWithClass(TFile* file, const Char_t* class_name);

   virtual Bool_t IsSortable() const
   {
      return fCollection->IsSortable();
   };
   virtual Bool_t IsSorted() const
   {
      return fCollection->IsSorted();
   };

   TSeqCollection* GetCollection() const
   {
      return fCollection;
   };

   ClassDef(KVSeqCollection, 2) //KaliVeda extensions to ROOT collections
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
