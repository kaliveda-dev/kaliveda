/***************************************************************************
                          kvring.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVRing.h,v 1.15 2007/10/23 14:09:02 ebonnet Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVRING_H
#define KVRING_H

#define KVR_RCPRC_CNXN 1
#define KVR_NORCPRC_CNXN 0

#include "KVPosition.h"
#include "KVGeoStrucElement.h"
#include "KVList.h"
#include "KVTelescope.h"

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

   Double_t GetSolidAngle(void)
   {
      // redefinition of KVPosition::GetSolidAngle() to take into account
      // only KVTelescope of the considered KVLayer which define the KVRing
      Double_t sol_ang = 0;
      KVTelescope* tel;
      TIter nxttel(GetTelescopes());
      while ((tel = (KVTelescope*) nxttel())) sol_ang += tel->GetSolidAngle();
      return sol_ang;
   }
   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();

   ClassDef(KVRing, 3)//Class representing one ring of an axially symmetric multidetector array
};

#endif
