/***************************************************************************
                          kvlayer.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVLayer.h,v 1.19 2009/03/03 14:27:15 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVLAYER_H
#define KVLAYER_H
#include "KVGeoStrucElement.h"

#include <TGeoVolume.h>

/**
 \class KVLayer
 \ingroup Geometry
 \brief Set of detectors at a similar distance from target (obsolete)
 */

class KVLayer : public KVGeoStrucElement {

public:

   KVLayer();
   virtual ~ KVLayer();

   Bool_t IsSortable() const
   {
      return kTRUE;
   };
   Int_t Compare(const TObject* obj) const;

   const Char_t* GetName() const;

   virtual TGeoVolume* GetGeoVolume();
   virtual void AddToGeometry();

   ClassDef(KVLayer, 3)         //Layer of telescopes at the "same distance" from the target in a multidetector array
};

#endif
