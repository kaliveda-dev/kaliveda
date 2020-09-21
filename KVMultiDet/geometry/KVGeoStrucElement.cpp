//Created by KVClassFactory on Tue May  7 09:22:02 2013
//Author: John Frankland,,,

#include "KVGeoStrucElement.h"
#include "KVDetector.h"
#include "Riostream.h"
#include <TROOT.h>
using namespace std;

ClassImp(KVGeoStrucElement)

void KVGeoStrucElement::init()
{
   // Default initialisations
   fStructures.SetOwner();
   fStructures.SetCleanup();
   fDetectors.SetCleanup();
   fParentStrucList.SetCleanup();
}

void KVGeoStrucElement::AddParentStructure(KVGeoStrucElement* e)
{
   fParentStrucList.Add(e);
}

void KVGeoStrucElement::RemoveParentStructure(KVGeoStrucElement* e)
{
   fParentStrucList.Remove(e);
}

KVGeoStrucElement::KVGeoStrucElement()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVGeoStrucElement::KVGeoStrucElement(const Char_t* name, const Char_t* type) : KVBase(name, type)
{
   // Create a geometry structure element with given name and type
   init();
}

KVGeoStrucElement::~KVGeoStrucElement()
{
   // Destructor
   Clear();
}

void KVGeoStrucElement::ClearDetectors(const Char_t* type)
{
   // Remove detectors of given type from this structure (default: all detectors)
   // If they belong to the structure, they will be deleted.

   TList dummy;
   dummy.AddAll(&fDetectors);
   TIter next(&dummy);
   KVDetector* d;
   while ((d = (KVDetector*)next())) {
      if (!strcmp(type, d->GetType()) || !strcmp(type, "")) {
         Remove(d);
         if (fDetectors.IsOwner()) delete d;
      }
   }
   // avoid spurious warnings from TList::Clear with ROOT6
   dummy.Clear("nodelete");
}

void KVGeoStrucElement::ClearStructures(const Char_t* type)
{
   // Remove daughter structures of given type from this structure (default: all detectors)
   // If they belong to the structure, they will be deleted.

   TList dummy;
   dummy.AddAll(&fStructures);
   TIter next(&dummy);
   KVGeoStrucElement* e;
   while ((e = (KVGeoStrucElement*)next())) {
      if (!strcmp(type, e->GetType()) || !strcmp(type, "")) {
         Remove(e);
         if (fStructures.IsOwner()) delete e;
      }
   }
   // avoid spurious warnings from TList::Clear with ROOT6
   dummy.Clear("nodelete");
}

void KVGeoStrucElement::Add(KVBase* element)
{
   // Add a detector or a daughter structure to this structure.
   // Note that daughter structures are by defeault owned by their mother and will be
   // deleted by her destructor. Call SetOwnsDaughters(kFALSE) to change.
   // Detectors are not owned by default - call SetOwnsDetectors(kTRUE) to change.

   if (element->InheritsFrom(KVDetector::Class())) {
      fDetectors.Add(element);
      dynamic_cast<KVDetector*>(element)->AddParentStructure(this);
   }
   else if (element->InheritsFrom(KVGeoStrucElement::Class())) {
      fStructures.Add(element);
      dynamic_cast<KVGeoStrucElement*>(element)->AddParentStructure(this);
   }
   else {
      Error("Add", "Cannot add elements of class %s", element->ClassName());
   }
}

void KVGeoStrucElement::Remove(KVBase* element)
{
   // Remove a detector or structure element from this structure
   // User's responsibility to delete the detector or structure element.

   if (element->InheritsFrom(KVDetector::Class())) {
      fDetectors.Remove(element);
      dynamic_cast<KVDetector*>(element)->RemoveParentStructure(this);
   }
   else if (element->InheritsFrom(KVGeoStrucElement::Class())) {
      fStructures.Remove(element);
      dynamic_cast<KVGeoStrucElement*>(element)->RemoveParentStructure(this);
   }
   else {
      Error("Add", "Cannot add elements of class %s", element->ClassName());
   }
}

void KVGeoStrucElement::Clear(Option_t*)
{
   // Empty lists of detectors, daughter structures, and parent structures

   fStructures.Clear();
   fDetectors.Clear();
   fParentStrucList.Clear();
}

KVGeoStrucElement* KVGeoStrucElement::GetStructure(const Char_t* type, Int_t num) const
{
   // Get structure with type and number

   unique_ptr<KVSeqCollection> typelist(GetStructureTypeList(type));
   KVGeoStrucElement* elem = (KVGeoStrucElement*)typelist->FindObjectByNumber(num);
   return elem;
}

KVGeoStrucElement* KVGeoStrucElement::GetStructure(const Char_t* type, const Char_t* name) const
{
   // Get structure with type and name
   return (KVGeoStrucElement*)fStructures.FindObjectWithNameAndType(name, type);
}

KVSeqCollection* KVGeoStrucElement::GetStructureTypeList(const Char_t* type) const
{
   // Create and fill a list with all structures of given type which are daughters
   // of this structure.
   // DELETE LIST AFTER USE - or, better: unique_ptr<KVSeqCollection> list(toto->GetStructureTypeList(...))

   return fStructures.GetSubListWithType(type);
}

KVDetector* KVGeoStrucElement::GetDetector(const Char_t* name) const
{
   // Return detector in this structure with given name
   return (KVDetector*)fDetectors.FindObject(name);
}

KVDetector* KVGeoStrucElement::GetDetectorByType(const Char_t* type) const
{
   // Return detector in this structure with given type
   return (KVDetector*)fDetectors.FindObjectByType(type);
}

KVSeqCollection* KVGeoStrucElement::GetDetectorTypeList(const Char_t* type) const
{
   // Create and fill a list with all detectors of given type in this structure.
   // DELETE LIST AFTER USE - or, better: unique_ptr<KVSeqCollection> list(toto->GetDetectorTypeList(...))

   return fDetectors.GetSubListWithType(type);
}

KVDetector* KVGeoStrucElement::GetDetectorAny(const Char_t* name)
{
   // Return detector in structure with given name.
   // If not found in this structure, we look in all daughter structures
   // for a detector with the name.

   KVDetector* det = (KVDetector*)GetDetectors()->FindObject(name);
   if (!det) {
      TIter next(GetStructures());
      KVGeoStrucElement* elem;
      while ((elem = (KVGeoStrucElement*)next())) {
         det = elem->GetDetectorAny(name);
         if (det) return det;
      }
   }
   return (KVDetector*)nullptr;

}

Bool_t KVGeoStrucElement::Fired(Option_t* opt) const
{
   // Returns kTRUE if any detector or structure element in this structure
   // has 'Fired' with the given option

   TIter nextd(GetDetectors());
   KVDetector* det;
   while ((det = (KVDetector*)nextd())) if (det->Fired(opt)) return kTRUE;
   TIter nexts(GetStructures());
   KVGeoStrucElement* elem;
   while ((elem = (KVGeoStrucElement*)nexts())) if (elem->Fired(opt)) return kTRUE;
   return kFALSE;
}

KVGeoStrucElement* KVGeoStrucElement::GetParentStructure(const Char_t* type, const Char_t* name) const
{
   // Get parent geometry structure element of given type.
   // Give unique name of structure if more than one element of same type is possible.
   KVGeoStrucElement* el = 0;
   if (strcmp(name, "")) {
      KVSeqCollection* strucs = fParentStrucList.GetSubListWithType(type);
      el = (KVGeoStrucElement*)strucs->FindObject(name);
      delete strucs;
   }
   else
      el = (KVGeoStrucElement*)fParentStrucList.FindObjectByType(type);
   return el;
}

void KVGeoStrucElement::Print(Option_t* option) const
{
   TROOT::IndentLevel();
   cout << ClassName() << "::" << GetName() << " [TYPE=" << GetType() << "]" << endl << endl;
   TROOT::IncreaseDirLevel();
   if (GetDetectors()->GetEntries()) {
      TROOT::IndentLevel();
      cout << "DETECTORS : " << endl << endl;
      TIter next(GetDetectors());
      TObject* d;
      TROOT::IncreaseDirLevel();
      while ((d = next())) {
         TROOT::IndentLevel();
         cout << "    " << d->GetName() << endl;
      }
      cout << endl;
      TROOT::DecreaseDirLevel();
   }
   TIter next(GetStructures());
   KVGeoStrucElement* el;
   //TROOT::IncreaseDirLevel();
   while ((el = (KVGeoStrucElement*)next())) {
      el->Print(option);
   }
   TROOT::DecreaseDirLevel();
   cout << endl;
}
