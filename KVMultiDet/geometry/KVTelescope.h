/***************************************************************************
$Id: KVTelescope.h,v 1.19 2008/12/17 13:01:26 franklan Exp $
// Author: J.D. Frankland, A. Mignon
                          kvtelescope.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVTELESCOPE_H
#define KVTELESCOPE_H
#include "KVGroup.h"
#include "KVPosition.h"
#include "KVDetector.h"
#include "KVList.h"
#include "KVNameValueList.h"

class KVNucleus;
class TGraph;

class KVTelescope: public KVGeoStrucElement, public KVPosition {

protected:
   Int_t fNdets;                //number of detectors in telescope
   Float_t* fDepth;             //[fNdets] depth of each element starting from nearest target
   void init();

public:

   KVTelescope();
   virtual ~ KVTelescope();

   void Add(KVBase* element);

   KVDetector* GetDetector(Int_t n) const
   {
      //returns the nth detector in the telescope structure
      if (n > GetSize() || n < 1) {
         Error("GetDetector(UInt_t n)", "%s n must be between 1 and %u",
               GetName(), GetSize());
         return 0;
      }
      KVDetector* d = (KVDetector*) GetDetectors()->At((n - 1));
      return d;
   };
   Int_t GetDetectorRank(KVDetector* kvd);
   Int_t GetSize() const
   {
      //returns number of detectors in telescope
      return GetDetectors()->GetSize();
   };
   virtual void DetectParticle(KVNucleus* kvp, KVNameValueList* nvl = 0);
   void ResetDetectors();


   void SetDepth(UInt_t ndet, Float_t depth);
   Float_t GetDepth(Int_t ndet) const;
   Double_t GetTotalLengthInCM() const;
   Double_t GetDepthInCM(Int_t ndet) const
   {
      // return depth inside telescope of detector number ndet in centimetres
      return ((Double_t)GetDepth(ndet)) / 10.;
   }
   Float_t GetDepth() const;

   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();

   // override KVPosition methods to apply to all detectors
   virtual void SetAzimuthalAngle(Double_t ph)
   {
      KVPosition::SetAzimuthalAngle(ph);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetAzimuthalAngle)(ph);
   }
   virtual void SetPolarAngle(Double_t th)
   {
      KVPosition::SetPolarAngle(th);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetPolarAngle)(th);
   }
   virtual void SetPolarWidth(Double_t pw)
   {
      KVPosition::SetPolarWidth(pw);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetPolarWidth)(pw);
   }
   virtual void SetPolarMinMax(Double_t min, Double_t max)
   {
      KVPosition::SetPolarMinMax(min, max);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetPolarMinMax)(min, max);
   }
   virtual void SetAzimuthalWidth(Double_t aw)
   {
      KVPosition::SetAzimuthalWidth(aw);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetAzimuthalWidth)(aw);
   }
   virtual void SetAzimuthalMinMax(Double_t min, Double_t max)
   {
      KVPosition::SetAzimuthalMinMax(min, max);
      const_cast<KVSeqCollection*>(GetDetectors())->R__FOR_EACH(KVDetector, SetAzimuthalMinMax)(min, max);
   }

   ClassDef(KVTelescope, 3)     //Multi-layered telescopes composed of different absorbers
};

#endif
