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
#include "KVACQParam.h"
#include "KVChannelVolt.h"
#include "KVVoltEnergy.h"
#include "KVUnits.h"

class KVDBParameterSet;

class KVChIo: public KVINDRADetector {

protected:

   KVChannelVolt* fChVoltGG;//!channel-volt conversion (GG)
   KVChannelVolt* fChVoltPG;//!channel-volt conversion (PG)
   KVVoltEnergy* fVoltE;//!volt-energy conversion

   void init();

public:

   KVChIo();
   KVChIo(Float_t pressure, Float_t thick = 5.0 * KVUnits::cm);
   virtual ~ KVChIo();

   Double_t GetVoltsFromCanalPG(Double_t chan = 0.0);
   Double_t GetVoltsFromCanalGG(Double_t chan = 0.0);
   Double_t GetCalibratedEnergy();
   Double_t GetVolts();
   Double_t GetVoltsFromEnergy(Double_t);

   Int_t GetCanalPGFromVolts(Float_t volts);
   Int_t GetCanalGGFromVolts(Float_t volts);

   Double_t GetCanalPGFromVoltsDouble(Float_t volts);
   Double_t GetCanalGGFromVoltsDouble(Float_t volts);

   Double_t GetEnergyFromVolts(Double_t volts = 0.0);
   virtual Double_t GetEnergy();

   void SetACQParams();
   void SetCalibrators();

   Double_t GetELossMylar(UInt_t z, UInt_t a, Double_t egas = -1.0, Bool_t stopped = kFALSE);

   inline Bool_t IsCalibrated() const;
   virtual Short_t GetCalcACQParam(KVACQParam*, Double_t) const;

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

   virtual void SetMylarThicknesses(Float_t thickF, Float_t thickB);

   void DeduceACQParameters(Int_t zz = -1, Int_t a = -1);

   ClassDef(KVChIo, 5)          //The ionisation chamber detectors (ChIo) of the INDRA array
};

//____________________________________________________________________________________________

inline Bool_t KVChIo::IsCalibrated() const
{
   //Redefined.
   //Only one of the channel-volts calibrators needs to be ready,
   //plus the volts-energy calibrator

   if (fVoltE && fVoltE->GetStatus()) {
      Bool_t ok_gg = (fChVoltGG != 0) ? fChVoltGG->GetStatus() : 0;
      Bool_t ok_pg = (fChVoltPG != 0) ? fChVoltPG->GetStatus() : 0;
      return (ok_gg || ok_pg);
   }

   return kFALSE;
}

#endif
