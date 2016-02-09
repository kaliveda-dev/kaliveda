/***************************************************************************
                          KVIDCsI.h  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
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

#ifndef KVIDCsI_H
#define KVIDCsI_H

#include "KVINDRAIDTelescope.h"
#include "KVIDGCsI.h"

class KVIDCsI: public KVINDRAIDTelescope {

   KVIDGCsI* CsIGrid;//! telescope's grid
   KVDetector* fCsI;//!

public:

   KVIDCsI();
   virtual ~ KVIDCsI();

   const Char_t* GetArrayName();

   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");

   virtual void Initialize();
   virtual Bool_t CanIdentify(Int_t Z, Int_t /*A*/)
   {
      // Used for filtering simulations
      // Returns kTRUE if this telescope is theoretically capable of identifying a given nucleus,
      // without considering thresholds etc.
      // For INDRA CsI Rapide-Lente detectors, identification is possible up to Z=4
      return (Z < 5);
   }

   ClassDef(KVIDCsI, 3)        //INDRA identification using CsI R-L matrices
};

#endif
