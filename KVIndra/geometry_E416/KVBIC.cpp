/***************************************************************************
                          KVBIC.cpp  -  description
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

#include "KVBIC.h"
#include "KVEBYEDAT_ACQParam.h"
#include "KVLinCal.h"
#include "TClass.h"

ClassImp(KVBIC)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//KVBIC
//Blocking Ionisation Chambers (BIC) for E416/E416a
//
//Here is the structure used to describe the Blocking Ionisation Chambers:
//Begin_Html
//<img src="http://indra.in2p3.fr/KaliVedaDoc/images/bic.gif">
//End_Html
//
//structure as absorbers:
//GetAbsorber(0) - 1.5µm mylar entrance window
//GetAbsorber(1) - CF4 at x Torr with thickness given by GetEffectiveEntryThickness()
//GetAbsorber(2) - 1 µm mylar internal window
//GetAbsorber(3) - 60 mm of CF4 at x Torr
//GetAbsorber(4) - 1 µm mylar internal window
//GetAbsorber(5) - CF4 at x Torr with thickness given by GetEffectiveEntryThickness()
//GetAbsorber(6) - 1.5µm mylar exit window
//
//Bombage:
//      La valeur du bombage des fenetres externes a ete "estimee" par C. Mazur
//      pour des fenetres neuves. Il est peut-etre prudent de la passer a 5mm,
//      ce qui conduirait a une epaisseur moyenne de 4mm environ de CF4 a
//      l'entree.
//
//The bombage can be set/modified using SetBombage(). The default value is 5mm.
//The effective thickness of CF4 between the two mylar windows, "averaged" over the whole surface of the
//entry window, is then "estimated" according to the formula (see GetEffectiveEntryThickness() ):
//      e = bombage/2 + 1.5  [mm]
//With the default bombage of 5mm, this gives e=4mm.
KVBIC::KVBIC()
{
   //default ctor
   //Segmentation index = 1
   //because particles passing through BIC can only hit 1 detector - SIB - behind it
   //this is analogous to the Si-CsI telescopes of ring 1.
   SetType("BIC");
   SetSegment(1);
   fLinCal = 0;
}

//___________________________________________________________________________

KVBIC::KVBIC(Float_t pressure, Float_t bomb): KVChIo()
{
   //BIC detector.
   //Give pressure in Torr
   //
   //Segmentation index = 1
   //because particles passing through BIC can only hit 1 detector - SIB - behind it
   //this is analogous to the Si-CsI telescopes of ring 1.
   //The type of these detectors is "BIC"
   //Pressure units are Torr

   SetType("BIC");
   SetSegment(1);
   fLinCal = 0;

   fBomb = bomb;
   //build detector
   AddAbsorber(new KVMaterial("Myl", 1.5 * KVUnits::um));   //mylar entry window
   Float_t e = GetEffectiveEntryThickness();
   KVMaterial* gas = new KVMaterial("CF4", e * KVUnits::mm, pressure);
   AddAbsorber(gas);            //gas between two windows
   AddAbsorber(new KVMaterial("Myl", 1.0 * KVUnits::um));   //interior window
   gas = new KVMaterial("CF4", 60 * KVUnits::mm, pressure);
   AddAbsorber(gas);            //main body of gas
   SetActiveLayer(gas);         //active layer - energy loss is measured
   AddAbsorber(new KVMaterial("Myl", 1.0 * KVUnits::um));   //2nd interor window
   gas = new KVMaterial("CF4", e * KVUnits::mm, pressure);
   AddAbsorber(gas);            //gas between two exit windows
   AddAbsorber(new KVMaterial("Myl", 1.5 * KVUnits::um));   //exit window

}

//___________________________________________________________________________

KVBIC::~KVBIC()
{
}

//___________________________________________________________________________

const Char_t* KVBIC::GetArrayName()
{
   //Give name of detector as BIC_x with x=number of blocking telescope

   fFName = TString("BIC_") + Form("%d", GetTelescope()->GetNumber());
   return fFName.Data();
}

//_______________________________________________________________________________

void KVBIC::AddACQParam(const Char_t* type)
{
   //Add an acquisition parameter of given type to this detector
   //The parameters for the BIC in blocking telescopes 1, 2, and 3
   //were called CI_1601_x, CI_1602_x, and CI_1603_x respectively,
   //so here we have to override the KVDetector default behaviour.

   if (!fACQParams)
      fACQParams = new KVList();
   KVEBYEDAT_ACQParam* par = new KVEBYEDAT_ACQParam();
   TString name;
   name.Form("CI_16%02d_%s", GetTelescope()->GetNumber(), type);
   par->SetName(name);
   par->SetDetector(this);
   par->SetType(type);
   fACQParams->Add(par);
}

//________________________________________________________________________________

KVEBYEDAT_ACQParam* KVBIC::GetACQParam(const Char_t* type) const
{
   //
   //Access acquisition parameter of given type
   //
   if (!fACQParams) {
      // Warning("GetACQParam",
      // "No acquisition parameters set for %s",GetName());
      return 0;
   }
   TString name;
   name.Form("CI_16%02d_%s", GetTelescope()->GetNumber(), type);
   return ((KVEBYEDAT_ACQParam*) fACQParams->FindObjectWithNameAndType(name, type));
}

//__________________________________________________________________________________

Float_t KVBIC::GetEffectiveEntryThickness() const
{
   //Returns "estimated" average thickness of CF4 between the two mylar windows
   //Calculated from bombage of external window according to:
   //  e = bombage/2 + 1.5

   return (fBomb / 2. + 1.5);
}

//__________________________________________________________________________________________________________________________

Double_t KVBIC::GetELossMylar(UInt_t, UInt_t, Double_t)
{
   //This returns zero.
   //It is so that the KVChIo method doesn't get used by accident
   return 0.;
}

//__________________________________________________________________________________________________________________________

Double_t KVBIC::GetCorrectedEnergy(KVNucleus* nuc, Double_t e, Bool_t transmit)
{
   //For a particle which passes through the BIC (i.e. is stopped in SIB behind it)
   //this will give the total energy loss of the particle in the BIC, i.e. including all
   //the dead zones.
   //If the particle stops in the BIC, this will not be correct, but in this case no
   //identification of the particle is possible, Z and A will not be known, so
   //this function should not be called.
   //If argument 'egas' not given, KVBIC::GetEnergy() is used

   if (!transmit) return 0.;

   e = ((e < 0.) ? GetEnergy() : e);

   if (e <= 0.)
      return 0.0;               //check measured (calibrated) energy in gas is reasonable (>0)

   Bool_t maxDE = kFALSE;

   //egas > max possible ?
   if (e > GetMaxDeltaE(nuc->GetZ(), nuc->GetA())) {
      e = GetMaxDeltaE(nuc->GetZ(), nuc->GetA());
      maxDE = kTRUE;
   }
   //calculate incident energy
   Double_t e_inc = GetIncidentEnergy(nuc->GetZ(), nuc->GetA(), e);

   //calculate residual energy
   Double_t e_res = GetERes(nuc->GetZ(), nuc->GetA(), e_inc);

   Double_t eloss = e_inc - e_res;
   if (maxDE)
      return (-eloss);
   return eloss;
}

//__________________________________________________________________________________________________________________________

void KVBIC::SetBombage(Float_t x)
{
   //Change bombage of entrance/exit windows
   //This changes the effective thickness of gas between the external & internal entrance/exit windows
   fBomb = x;
   Float_t e = GetEffectiveEntryThickness();
   GetAbsorber(1)->SetThickness(e);
   GetAbsorber(5)->SetThickness(e);
}

//__________________________________________________________________________________________________________________________

void KVBIC::SetPressure(Float_t x)
{
   //Changes pressure of all 3 gas-filled zones in the BIC
   GetAbsorber(1)->SetPressure(x);
   GetAbsorber(3)->SetPressure(x);
   GetAbsorber(5)->SetPressure(x);
}

//__________________________________________________________________________________________________________________________

void KVBIC::SetTemperature(Double_t x)
{
   //Changes temperature of all 3 gas-filled zones in the BIC
   GetAbsorber(1)->SetTemperature(x);
   GetAbsorber(3)->SetTemperature(x);
   GetAbsorber(5)->SetTemperature(x);
}

//__________________________________________________________________________________________________________________________

void KVBIC::SetCalibrators()
{
   //Adds linear calibration for PG channel
   KVLinCal* cal = new KVLinCal(this);
   cal->SetChannelParameter("PG");
   cal->WithPedestalCorrection(kFALSE);
   if (!AddCalibrator(cal)) delete cal;
}

//______________________________________________________________________________

void KVBIC::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVBIC.
   // We set the pointers to the calibrator objects
   // We add the linear calibrator if it is missing

   if (R__b.IsReading()) {
      KVBIC::Class()->ReadBuffer(R__b, this);
      fLinCal = (KVLinCal*)GetCalibrator("Linear calibration PG");
   }
   else {
      KVBIC::Class()->WriteBuffer(R__b, this);
   }
}

//__________________________________________________________________________________________________________________________

Double_t KVBIC::GetEnergy() const
{
   //Redefinition of KVChIo::GetEnergy().
   //If energy lost in active layer is already set (e.g. by calculation of energy loss
   //of charged particles), return its value.
   //If not, we calculate it and set it using the values read from coders
   //and the linear channel-energy calibration if initialised
   //
   //Returns -99 if (i) no calibration present (ii) calibration present but no data in acquisition parameters

   //fELoss already set, return its value
   if (KVDetector::GetEnergy())
      return KVDetector::GetEnergy();
   Double_t ELoss = -99;
   //is detector calibration available and ready ?
   if (IsCalibrated()) {
      ELoss = fLinCal->Compute(-1);
      if (ELoss > -99.) {
         SetEnergy(ELoss);
      }
   }
   return ELoss;
}
