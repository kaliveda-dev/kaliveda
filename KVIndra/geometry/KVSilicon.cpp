/***************************************************************************
$Id: KVSilicon.cpp,v 1.55 2009/04/15 09:49:19 ebonnet Exp $
                          kvsilicon.cpp  -  description
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
#include "Riostream.h"
#include "KVSilicon.h"
#include "TClass.h"
#define MAX_CANAL_GG 4000

ClassImp(KVSilicon)

//////////////////////////////////////////////////////////////////
//KVSilicon
//
//Used to describe Silicon detectors of the INDRA array.
//In order to create a detector, use the KVSilicon::KVSilicon(Float_t thick)
//constructor with "thick" the thickness in centimetres
//
//Type of detector : "SI"

void KVSilicon::init()
{
   //initialise non-persistent pointers
   fChIo = 0;
   fChVoltGG = 0;
   fChVoltPG = 0;
   fVoltE = 0;
   fPHD = 0;
   fSegment = 1;
}

//______________________________________________________

KVSilicon::KVSilicon()
{
   //Default ctor
   //This ctor (which in turn calls the KVDetector default ctor) must exist in
   //order for Cloning of detectors to work (as used in KVTelescope::AddDetector).
   //Do not replace this ctor by giving default arguments to KVSilicon(Float_t).
   //
   init();
}

//______________________________________________________________________________
KVSilicon::KVSilicon(Float_t thick): KVINDRADetector("Si", thick * KVUnits::um)
{
   // constructor for silicon detector, thickness in microns
   // Type of detector: "SI"
   SetType("SI");
   init();
}

//____________________________________________________________________________________________

KVSilicon::~KVSilicon()
{
}

//____________________________________________________________________________________________
Int_t KVSilicon::GetCanalPGFromVolts(Float_t volts)
{
   //Return raw PG channel number corresponding to a given detector signal in volts
   //
   //Any change in the coder pedestal between the current run and the effective pedestal
   //of the channel-volt calibration (GetCanal(V=0)) is automatically corrected for.
   //
   //Returns -1 if PG <-> Volts calibration is not available


   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return -1;
   Int_t chan = TMath::Nint(fChVoltPG->Invert(volts) + GetPedestal("PG") - fChVoltPG->Invert(0));
   return chan;

}

//____________________________________________________________________________________________
Int_t KVSilicon::GetCanalGGFromVolts(Float_t volts)
{
   //Return raw GG channel number corresponding to a given detector signal in volts
   //
   //Any change in the coder pedestal between the current run and the effective pedestal
   //of the channel-volt calibration (GetCanal(V=0)) is automatically corrected for.
   //
   //Returns GG calculated from PG if GG <-> Volts calibration is not available

   if (!fChVoltGG || !fChVoltGG->GetStatus()) {
//          Info("GetCanalGGFromVolts","%s no calibrator ready for GG...",GetName());
      return GetGGfromPG(GetCanalPGFromVolts(volts));
   }
   Int_t chan = TMath::Nint(fChVoltGG->Invert(volts) + GetPedestal("GG") - fChVoltGG->Invert(0));
   return chan;
}

//____________________________________________________________________________________________
Double_t KVSilicon::GetCanalPGFromVoltsDouble(Float_t volts)
{
   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return -1;
   return fChVoltPG->InvertDouble(volts) + GetPedestal("PG") - fChVoltPG->InvertDouble(0);
}

//____________________________________________________________________________________________
Double_t KVSilicon::GetCanalGGFromVoltsDouble(Float_t volts)
{
   if (!fChVoltGG || !fChVoltGG->GetStatus()) {
      return GetGGfromPG(GetCanalPGFromVoltsDouble(volts));
   }
   return fChVoltGG->InvertDouble(volts) + GetPedestal("GG") - fChVoltGG->InvertDouble(0);
}

//____________________________________________________________________________________________

void KVSilicon::SetACQParams()
{
   //Setup acquistion parameters for this Silicon.
   //Do not call before ChIo name has been set.

   AddACQParamType("GG");
   AddACQParamType("PG");
   AddACQParamType("T");

}

//____________________________________________________________________________________________

void KVSilicon::SetCalibrators()
{
   //Set up calibrators for this detector. Call once name has been set.
   KVCalibrator* c = new KVChannelVolt("GG", this);
   if (!AddCalibrator(c)) delete c;
   c = new KVChannelVolt("PG", this);
   if (!AddCalibrator(c)) delete c;
   c = new KVVoltEnergy(this);
   if (!AddCalibrator(c)) delete c;
   c = new KVPulseHeightDefect(this);
   if (!AddCalibrator(c)) delete c;
   fVoltE = (KVVoltEnergy*) GetCalibrator("Volt-Energy");
   fChVoltPG  = (KVChannelVolt*) GetCalibrator("Channel-Volt PG");
   fChVoltGG  = (KVChannelVolt*) GetCalibrator("Channel-Volt GG");
   fPHD  = (KVPulseHeightDefect*) GetCalibrator("Pulse Height Defect");
}

//__________________________________________________________________________________________

Double_t KVSilicon::GetPHD(Double_t dE, UInt_t Z)
{
   //Calculate Pulse Height Defect in MeV for a given energy loss dE(MeV) and Z.
   //The formula of Moulton is used (see class KVPulseHeightDefect).
   //
   //Returns 0 if PHD is not defined.

   if (!fPHD || !fPHD->GetStatus()) return 0;
   fPHD->SetZ(Z);
   return fPHD->Compute(dE);
}

//__________________________________________________________________________________________
Double_t KVSilicon::GetCalibratedEnergy()
{
   //Calculate energy in MeV from coder values.
   //Returns 0 if calibration not ready or detector not fired
   //(we require that at least one acquisition parameter have a value
   //greater than the current pedestal value)

   if (IsCalibrated() && Fired("Pany"))
      return (fVoltE->Compute(GetVolts()));
   return 0;
}

//____________________________________________________________________________________________

Double_t KVSilicon::GetVoltsFromCanalPG(Double_t chan)
{
   //Return calibrated detector signal in Volts calculated from PG channel number.
   //If "chan" is not given, the value of the "PG" acquisition parameter read from
   //data for this detector is used to calculate the signal.
   //If the PG parameter is not present (=-1) or no calib we return 0.
   //Any change in the coder pedestal between the current run and the effective pedestal
   //of the channel-volt calibration (GetCanal(V=0)) is automatically corrected for.

   if (!fChVoltPG || !fChVoltPG->GetStatus())
      return 0;

   if (!chan) {
      chan = GetPG();
   }
   if (chan < -0.5)
      return 0.;          //PG parameter absent
   //correct for pedestal drift
   chan = chan - (Double_t) GetPedestal("PG") + fChVoltPG->Invert(0.);
   return (fChVoltPG->Compute(chan));
}

//____________________________________________________________________________________________
Double_t KVSilicon::GetVoltsFromCanalGG(Double_t chan)
{
   //Return calibrated detector signal in Volts calculated from GG channel number.
   //If "chan" is not given, the value of the "GG" acquisition parameter read from
   //data for this detector is used to calculate the signal.
   //If the GG parameter is not present (=-1) or no calib we return 0.
   //Any change in the coder pedestal between the current run and the effective pedestal
   //of the channel-volt calibration (GetCanal(V=0)) is automatically corrected for.

   if (!fChVoltGG || !fChVoltGG->GetStatus())
      return 0;

   if (!chan) {
      chan = GetGG();
   }
   if (chan < -0.5)
      return 0.;          //GG parameter absent
   //correct for pedestal drift
   chan = chan - (Double_t) GetPedestal("GG") + fChVoltGG->Invert(0);
   return (fChVoltGG->Compute(chan));

}

//____________________________________________________________________________________________

Double_t KVSilicon::GetVolts()
{
   //Returns Volts for this detector calculated from current PG coder values.
   //We only use PG, as the two channel-volt calibrations do not coincide and so the passage
   //from GG to PG produces a discontinuity (unless only GG calibration is available, then
   //we convert PG to GG and use the GG calibration)
   //Returns 0 if no calibration available

   if (fChVoltPG && fChVoltPG->GetStatus()) {
      return GetVoltsFromCanalPG();
   } else if (fChVoltGG && fChVoltGG->GetStatus()) {
      return GetVoltsFromCanalGG(GetGGfromPG());
   }

   return 0;
}

//_______________________________________________________________________________

Double_t KVSilicon::GetVoltsFromEnergy(Double_t e)
{
   //Inverts calibration, i.e. calculates volts for a given energy loss (in MeV)

   if (fVoltE->GetStatus()) {
      return (fVoltE->Invert(e));
   }
   return 0;
}

//____________________________________________________________________________________________

Double_t KVSilicon::GetEnergyFromVolts(Double_t volts)
{
   //Calculate energy in MeV from calibrated detector signal in
   //Volts. If 'volts' is not given, the value in volt returned
   //by GetVolts().

   if (fVoltE && fVoltE->GetStatus()) {
      if (!volts) volts = GetVolts();
      return fVoltE->Compute(volts);
   }
   return 0.;
}

//____________________________________________________________________________________________

Double_t KVSilicon::GetEnergy()
{
   //Redefinition of KVDetector::GetEnergy().
   //If energy lost in active layer is already set (e.g. by calculation of energy loss
   //of charged particles), return its value.
   //If not, we calculate it and set it using the values read from coders (if fired)
   //and the channel-volts/volts-energy calibrations, if present
   //
   //Returns 0 if (i) no calibration present (ii) calibration present but no data in acquisition parameters

   //fELoss already set, return its value
   Double_t ELoss = KVDetector::GetEnergy();
   if (IsSimMode()) return ELoss; // in simulation mode, return calculated energy loss in active layer
   if (ELoss > 0) return ELoss;
   ELoss = GetCalibratedEnergy();
   if (ELoss < 0) ELoss = 0;
   SetEnergy(ELoss);
   return ELoss;
}

//______________________________________________________________________________

void KVSilicon::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVSilicon.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      KVSilicon::Class()->ReadBuffer(R__b, this);
      fVoltE = (KVVoltEnergy*) GetCalibrator("Volt-Energy");
      fChVoltPG  = (KVChannelVolt*) GetCalibrator("Channel-Volt PG");
      fChVoltGG  = (KVChannelVolt*) GetCalibrator("Channel-Volt GG");
      fPHD  = (KVPulseHeightDefect*) GetCalibrator("Pulse Height Defect");
   } else {
      KVSilicon::Class()->WriteBuffer(R__b, this);
   }
}

//______________________________________________________________________________

void KVSilicon::SetMoultonPHDParameters(Double_t a_1, Double_t a_2, Double_t b_1, Double_t b_2)
{
   //Sets parameters of Moulton formula used to calculate PHD for particles
   //stopping in this detector. The parameters are as in the following:
   //
   // log_10(PHD) = b(Z) + a(Z)*log_10(E)
   //
   //  with  a(Z) = a_1*(Z**2/1000) + a_2
   //          b(Z) = b_1*(100/Z) + b_2
   //            E = energy lost by particle
   //
   //See class KVPulseHeightDefect

   if (fPHD) {
      fPHD->SetParameters(a_1, a_2, b_1, b_2);
      fPHD->SetStatus(kTRUE);
   }
}

//______________________________________________________________________________

Short_t KVSilicon::GetCalcACQParam(KVACQParam* ACQ, Double_t ECalc) const
{
   // Calculates & returns value of given acquisition parameter corresponding to
   // given calculated energy loss in the detector
   // Returns -1 if detector is not calibrated

   if (!IsCalibrated()) return -1;
   Double_t volts = const_cast<KVSilicon*>(this)->GetVoltsFromEnergy(ECalc);
   if (ACQ->IsType("PG")) return (Short_t)const_cast<KVSilicon*>(this)->GetCanalPGFromVolts(volts);
   else if (ACQ->IsType("GG")) return (Short_t)const_cast<KVSilicon*>(this)->GetCanalGGFromVolts(volts);
   return -1;
}

//______________________________________________________________________________

TF1* KVSilicon::GetELossFunction(Int_t Z, Int_t A)
{
   // Overrides KVDetector::GetELossFunction
   // If the pulse height deficit (PHD) has been set for this detector,
   // we return an energy loss function which takes into account the PHD,
   // i.e. for an incident energy E we calculate
   //
   //      dEphd(E,Z,A) = dE(E,Z,A) - PHD(dE,Z)
   //
   // If no PHD is set, we return the usual KVDetector::GetELossFunction
   // which calculates dE(E,Z,A)

   if (fPHD && fPHD->GetStatus()) return fPHD->GetELossFunction(Z, A);

   return KVDetector::GetELossFunction(Z, A);
}

void KVSilicon::DeduceACQParameters(Int_t , Int_t)
{

   Double_t volts = GetVoltsFromEnergy(GetEnergy());
   Int_t cipg = (Int_t)GetCanalPGFromVolts(volts);
   Int_t cigg = (Int_t)GetCanalGGFromVolts(volts);
   //cout << "chio: pg = " << cipg << " gg = " << cigg << endl;
   GetACQParam("PG")->SetData((UShort_t)TMath::Min(4095, cipg));
   GetACQParam("GG")->SetData((UShort_t)TMath::Min(4095, cigg));
   GetACQParam("T")->SetData(110);
}


//__________________________________________________________________________________________

ClassImp(KVSi75)
//_________________________________________
//Etalon telescope detector Si75
//(actually an 80 micron thick silicon detector)
//Type of detector: "SI75"
//Array naming convention: "SI75_RR" with RR=ring number

KVSi75::KVSi75(): KVSilicon()
{
   //Default ctor
   //For cloning
}

KVSi75::KVSi75(Float_t thick): KVSilicon(thick)
{
   //Default ctor
   //80 micron silicon detector with type "SI75"
   SetType("SI75");
   SetLabel("SI75");
}

KVSi75::~KVSi75()
{
   //default dtor
}

const Char_t* KVSi75::GetArrayName()
{
   //Redefinition of KVDetector method.
   //Name given as SI75_xx with xx=Ring number

   fFName = Form("SI75_%02d", GetRingNumber());
   return fFName.Data();
}

//__________________________________________________________________________________________

ClassImp(KVSiLi)
//_________________________________________
//Etalon telescope detector Si(Li)
//Nominal thickness: 2mm active layer + 40um dead layer (both silicon)
//Type of detector: "SILI"
//Array naming convention: "SILI_RR" with RR=ring number

KVSiLi::KVSiLi(): KVSilicon()
{
   //Default ctor
   //For cloning
}

KVSiLi::KVSiLi(Float_t thick): KVSilicon(thick)
{
   //Default ctor
   // first layer (active) : 2mm silicon (nominal)
   // second layer (dead) : 40um silicon (nominal)
   AddAbsorber(new KVMaterial("Si", 40.0 * KVUnits::um));
   SetType("SILI");
   SetLabel("SILI");
}

KVSiLi::~KVSiLi()
{
   //default dtor
}

const Char_t* KVSiLi::GetArrayName()
{
   //Redefinition of KVDetector method.
   //Name given as SILI_xx with xx=Ring number

   fFName = Form("SILI_%02d", GetRingNumber());
   return fFName.Data();
}
