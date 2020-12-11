//Created by KVClassFactory on Thu Jun  9 10:34:54 2011
//Author: frankland,,,,

#ifndef __KVINDRATELESCOPE_H
#define __KVINDRATELESCOPE_H

#include "KVTelescope.h"

class KVRing;

/**
  \class KVINDRATelescope
  \ingroup INDRAGeometry
  \brief Base class for telescopes in INDRA array
 */

class KVINDRATelescope : public KVTelescope {

public:
   KVINDRATelescope();
   virtual ~KVINDRATelescope();

   const Char_t* GetName() const;
   virtual const Char_t* GetArrayName();
   Int_t GetRingNumber() const;
   KVRing* GetRing() const;
   KVGroup* GetGroup() const;
   Bool_t IsSortable() const
   {
      return kTRUE;
   };
   Int_t Compare(const TObject* obj) const;

   ClassDef(KVINDRATelescope, 1) //A detector telescope in the INDRA array
};

#endif
