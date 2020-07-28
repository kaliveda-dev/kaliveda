//Created by KVClassFactory on Tue May  7 09:22:02 2013
//Author: John Frankland,,,

#ifndef __KVGEOSTRUCELEMENT_H
#define __KVGEOSTRUCELEMENT_H

#include "KVBase.h"
#include "KVUniqueNameList.h"
class KVDetector;

/**
 \class KVGeoStrucElement
 \ingroup Geometry
 \brief Base class describing elements of array geometry
 */

class KVGeoStrucElement : public KVBase {
   void init();

protected:
   KVUniqueNameList fDetectors;       //detectors in this structure element
   KVUniqueNameList fStructures;      //daughter structures
   KVUniqueNameList fParentStrucList; //parent structures

   void AddParentStructure(KVGeoStrucElement*);
   void RemoveParentStructure(KVGeoStrucElement*);

public:
   KVGeoStrucElement();
   KVGeoStrucElement(const Char_t* name, const Char_t* type = "");
   virtual ~KVGeoStrucElement();

   void SetOwnsDaughters(Bool_t yes = kTRUE)
   {
      // yes = kTRUE: all daughter structures will be deleted by this
      // yes = kFALSE: daughter structures will not be deleted by this
      fStructures.SetOwner(yes);
   }

   void SetOwnsDetectors(Bool_t yes = kTRUE)
   {
      // yes = kTRUE: all detectors will be deleted by this
      // yes = kFALSE: detectors will not be deleted by this
      fDetectors.SetOwner(yes);
   }

   bool OwnsDaughters() const
   {
      // kTRUE: all daughter structures will be deleted by this
      // kFALSE: daughter structures will not be deleted by this
      return fStructures.IsOwner();
   }

   bool OwnsDetectors() const
   {
      // kTRUE: all detectors will be deleted by this
      // kFALSE: detectors will not be deleted by this
      return fDetectors.IsOwner();
   }

   void Sort(Bool_t order = kSortAscending)
   {
      SortStructures(order);
      SortDetectors(order);
   }
   void SortStructures(Bool_t order = kSortAscending)
   {
      fStructures.Sort(order);
   }
   void SortDetectors(Bool_t order = kSortAscending)
   {
      fDetectors.Sort(order);
   }
   void ClearDetectors(const Char_t* type = "");
   void ClearStructures(const Char_t* type = "");

   virtual void Add(KVBase*);
   virtual void Remove(KVBase*);

   void Clear(Option_t* opt = "");

   virtual Bool_t Contains(KVBase* name) const
   {
      // Returns kTRUE if this structure contains the detector or structure
      // given as argument

      TObject* det = GetDetectors()->FindObject(name);
      if (det != nullptr) return kTRUE;
      return (GetStructures()->FindObject(name) != nullptr);
   }

   KVGeoStrucElement* GetStructure(const Char_t* name) const
   {
      // Return daughter structure with given name
      return (KVGeoStrucElement*)fStructures.FindObject(name);
   }
   KVGeoStrucElement* GetStructure(const Char_t* type, Int_t num) const;
   KVGeoStrucElement* GetStructure(const Char_t* type, const Char_t* name) const;
   KVSeqCollection* GetStructureTypeList(const Char_t* type) const;
   KVDetector* GetDetector(const Char_t* name) const;

   KVDetector* GetDetectorByType(const Char_t* type) const;

   KVSeqCollection* GetDetectorTypeList(const Char_t* type) const;
   KVDetector* GetDetectorAny(const Char_t* name);

   const KVSeqCollection* GetDetectors() const
   {
      return &fDetectors;
   }
   const KVSeqCollection* GetStructures() const
   {
      return &fStructures;
   }
   const KVSeqCollection* GetParents() const
   {
      return &fParentStrucList;
   }
   virtual Bool_t Fired(Option_t* opt = "any") const;
   KVGeoStrucElement* GetParentStructure(const Char_t* type, const Char_t* name = "") const;

   void Print(Option_t* option = "") const;

   ClassDef(KVGeoStrucElement, 1) //A multidetector array geometry structural element
};

#endif
