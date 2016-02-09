/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVSILICON_H
#define KVSILICON_H

#include "KVINDRADetector.h"
#include "TF1.h"
#include "KVACQParam.h"
#include "KVChannelVolt.h"
#include "KVPulseHeightDefect.h"
#include "KVVoltEnergy.h"
#include "KVUnits.h"

class KVChIo;
class KVDBParameterSet;

class KVSilicon: public KVINDRADetector {

protected:

   KVChannelVolt* fChVoltGG;//!channel-volt conversion (GG)
   KVChannelVolt* fChVoltPG;//!channel-volt conversion (PG)
   KVVoltEnergy* fVoltE;//!volt-energy conversion
   KVPulseHeightDefect* fPHD;//!pulse height defect

   void init();

public:

   KVSilicon();
   KVSilicon(Float_t thick /* um */);
   virtual ~ KVSilicon();

   virtual Double_t GetVoltsFromCanalPG(Double_t chan = 0.0);
   Double_t GetVoltsFromCanalGG(Double_t chan = 0.0);
   Double_t GetCalibratedEnergy();
   Double_t GetVolts();
   Double_t GetVoltsFromEnergy(Double_t);

   virtual Int_t GetCanalPGFromVolts(Float_t volts);
   Int_t GetCanalGGFromVolts(Float_t volts);

   virtual Double_t GetCanalPGFromVoltsDouble(Float_t volts);
   Double_t GetCanalGGFromVoltsDouble(Float_t volts);

   Double_t GetEnergyFromVolts(Double_t volts = 0.0);
   virtual Double_t GetEnergy();

   void SetACQParams();
   void SetCalibrators();

   Double_t GetPHD(Double_t dE, UInt_t Z);

   inline Bool_t IsCalibrated() const;

   virtual void SetMoultonPHDParameters(Double_t a1, Double_t a2, Double_t b1, Double_t b2);

   virtual Short_t GetCalcACQParam(KVACQParam*, Double_t) const;
   virtual TF1* GetELossFunction(Int_t Z, Int_t A);

   virtual void SetThickness(Double_t thick /* um */)
   {
      // Sets thickness of active layer in microns
      GetActiveLayer()->SetThickness(thick * KVUnits::um);
   };
   virtual Double_t GetThickness() const /* um */
   {
      // Returns thickness of active layer in microns
      return GetActiveLayer()->GetThickness() / KVUnits::um;
   };
   void DeduceACQParameters(Int_t zz = -1, Int_t aa = -1);

   ClassDef(KVSilicon, 9)       //INDRA forward-rings silicon detector
};

//____________________________________________________________________________________________

inline Bool_t KVSilicon::IsCalibrated() const
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


class KVSi75: public KVSilicon {
public:

   KVSi75();
   KVSi75(Float_t thick);
   virtual ~ KVSi75();
   const Char_t* GetArrayName();

   ClassDef(KVSi75, 1)         //80um silicon detector for INDRA etalon telescopes
};

class KVSiLi: public KVSilicon {
public:

   KVSiLi();
   KVSiLi(Float_t thick);
   virtual ~ KVSiLi();
   const Char_t* GetArrayName();

   ClassDef(KVSiLi, 2)         //2mm + 40um dead zone Si(Li) detector for INDRA etalon telescopes
};
#endif
