//Created by KVClassFactory on Sat Oct  3 14:18:09 2009
//Author: John Frankland,,,

#ifndef __KVINDRADETECTOR_H
#define __KVINDRADETECTOR_H

#include "KVDetector.h"
#include "KVINDRATelescope.h"

/**
  \class KVINDRADetector
  \ingroup INDRAGeometry
  \brief Base class for detectors of INDRA array
 */

class KVINDRADetector : public KVDetector {
protected:

   KVINDRADetector* fChIo;//!pointer to ionisation chamber in group associated to this detector
   KVINDRADetector* FindChIo();

public:
   KVINDRADetector()
      : fChIo(nullptr)
   {
   }
   virtual ~KVINDRADetector() {}
   KVINDRADetector(const Char_t* type, const Float_t thick = 0.0)
      : KVDetector(type, thick), fChIo(nullptr)
   {
   }

   KVINDRATelescope* GetTelescope() const
   {
      // Return pointer to telescope containing this detector
      return (KVINDRATelescope*)GetParentStructure("TELESCOPE");
   }

   virtual void SetSegment(UShort_t)
   {
      // Overrides KVDetector method.
      // 'Segmentation' of INDRA detectors is defined in ctor of dedicated
      // detector classes
   }

   void SetType(const Char_t* t)
   {
      // Detector types for INDRA are uppercase
      TString T(t);
      T.ToUpper();
      KVDetector::SetType(T);
   }

   const Char_t* GetArrayName();
   UInt_t GetRingNumber() const
   {
      if (GetTelescope()) return GetTelescope()->GetRingNumber();
      // if no telescope, deduce from name
      KVString name(GetName());
      name.Begin("_");
      KVString type = name.Next(kTRUE);
      KVString index = name.Next(kTRUE);
      if (type == "SILI" || type == "SI75") return index.Atoi();
      return index.Atoi() / 100;
   }
   UInt_t GetModuleNumber() const
   {
      if (GetTelescope()) return GetTelescope()->GetNumber();
      // if no telescope, deduce from name
      KVString name(GetName());
      name.Begin("_");
      KVString type = name.Next(kTRUE);
      KVString index = name.Next(kTRUE);
      if (type == "SILI" || type == "SI75") return 0; //no idea
      return index.Atoi() % 100;
   }

   KVINDRADetector* GetChIo() const
   {
      return (fChIo ? fChIo : const_cast<KVINDRADetector*>(this)->FindChIo());
   }

   void SetThickness(Double_t thick);

   ClassDef(KVINDRADetector, 2) //Detectors of INDRA array
};

#endif
