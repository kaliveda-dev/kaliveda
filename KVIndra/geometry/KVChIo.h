/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVCHIO_H
#define KVCHIO_H

#include "KVINDRADetector.h"
#include "KVUnits.h"

/**
  \class KVChIo
  \ingroup INDRAGeometry
  \brief Ionisation chamber detectors of the INDRA multidetector array

These consist of:
  -    a 2.5 micron mylar window
  -    5 cm of C3F8 (pressure varies) ---> active layer
  -    a 2.5 micron mylar window

Type of detector : "CI"
*/

class KVChIo: public KVINDRADetector {

   void init();

public:

   KVChIo();
   KVChIo(Float_t pressure, Float_t thick = 5.0 * KVUnits::cm);
   virtual ~ KVChIo();

   //void SetACQParams();

   Double_t GetELossMylar(UInt_t z, UInt_t a, Double_t egas = -1.0, Bool_t stopped = kFALSE);

   virtual void SetPressure(Double_t P /* mbar */)
   {
      // Set pressure of gas in mbar
      GetActiveLayer()->SetPressure(P * KVUnits::mbar);
   }
   virtual Double_t GetPressure() const /* mbar */
   {
      // Give pressure of gas in mbar
      return GetActiveLayer()->GetPressure() / KVUnits::mbar;
   }

   void SetMylarThicknesses(Float_t thickF, Float_t thickB);

   void DeduceACQParameters(KVEvent*, KVNumberList&);

   ClassDef(KVChIo, 5)          //The ionisation chamber detectors (ChIo) of the INDRA array
};

#endif
