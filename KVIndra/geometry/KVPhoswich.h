/***************************************************************************
$Id: KVPhoswich.h,v 1.9 2006/10/19 14:32:43 franklan Exp $
                          kvphoswich.h  -  description
                             -------------------
    begin                : Fri Oct 4 2002
    copyright            : (C) 2002 by A. Mignon & J.D. Frankland
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

#ifndef KVPHOSWICH_H
#define KVPHOSWICH_H

#include "KVINDRADetector.h"

/**
\class KVPhoswich
\brief Phoswich detector in the INDRA array (first 3 campaigns)
\ingroup INDRAGeometry

Built from two layers of plastic, with thicknesses 0.05cm (NE102) and 25cm (NE115).
Name of detector is PHOS_Telescope-number.

Type of detector: "PHOS"
*/

class KVPhoswich: public KVINDRADetector {
public:
   KVPhoswich();
   KVPhoswich(Float_t);
   virtual ~ KVPhoswich();

   const Char_t* GetArrayName();
   //void Print(Option_t * opt="") const;
   Double_t GetEnergy() const;
   void SetEnergy(Double_t e) const;
   //void SetACQParams();

   ClassDef(KVPhoswich, 2)     //Phoswich telescopes of the INDRA array
};

#endif
