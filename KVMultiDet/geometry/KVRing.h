#ifndef KVRING_H
#define KVRING_H

#define KVR_RCPRC_CNXN 1
#define KVR_NORCPRC_CNXN 0

#include "KVPosition.h"
#include "KVGeoStrucElement.h"
#include "KVList.h"
#include "TGeoVolume.h"
class KVTelescope;

/**
 \class KVRing
 \ingroup INDRAGeometry
 \brief Ring in INDRA array (obsolete)
 */

class KVRing : public KVGeoStrucElement, public KVPosition {

   void init();

public:

   KVRing();
   virtual ~ KVRing();

   void Add(KVBase*);

   const KVSeqCollection* GetTelescopes() const
   {
      return GetStructures();
   }
   KVTelescope* GetTelescope(Float_t phi) const;
   KVTelescope* GetTelescope(const Char_t* name) const;

   Bool_t IsSortable() const
   {
      return kTRUE;
   };
   Int_t Compare(const TObject* obj) const;

   Double_t GetSolidAngle(void) const;
   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();

   ClassDef(KVRing, 3)//Class representing one ring of an axially symmetric multidetector array
};

#endif
