/***************************************************************************
                          KVBIC.h  -  description
                             -------------------
    begin                : Mon Nov 3 2003
    copyright            : (C) 2003 by John Frankland
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

#ifndef KVBIC_H
#define KVBIC_H

#include "KVChIo.h"
#include "KVLinCal.h"

class KVBIC: public KVChIo {

   Float_t fBomb;               //bombage of entrance/exit windows in mm
   KVLinCal* fLinCal;//!linear channel-energy calibration

public:

   KVBIC();
   KVBIC(Float_t pressure, Float_t bombage = 5.0);
   virtual ~ KVBIC();

   const Char_t* GetArrayName();
   void AddACQParam(const Char_t*);
   KVACQParam* GetACQParam(const Char_t* type);

   void SetPressure(Float_t);
   void SetTemperature(Double_t t);
   void SetBombage(Float_t x);
   Float_t GetBombage() const
   {
      return fBomb;
   };
   Float_t GetEffectiveEntryThickness() const;

   Double_t GetELossMylar(UInt_t z, UInt_t a, Double_t egas = -1.0);
   Double_t GetCorrectedEnergy(KVNucleus*, Double_t e = -1.0, Bool_t transmit = kTRUE);

   void SetCalibrators();

   Double_t GetEnergy();
   //returns kTRUE if linear calibrator fLinCal is present and has been initialised
   Bool_t IsCalibrated() const
   {
      return (fLinCal && fLinCal->GetStatus());
   };
   virtual void SetPressure(Double_t P /* torr*/)
   {
      // Set pressure of gas in torr
      GetActiveLayer()->SetPressure(P);
   };
   virtual Double_t GetPressure() const /* torr */
   {
      // Give pressure of gas in torr
      return GetActiveLayer()->GetPressure();
   };

   ClassDef(KVBIC, 2)          //Blocking ChIo for E416 experiment
};

#endif
