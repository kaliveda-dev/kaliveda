/***************************************************************************
                          KVSiB.cpp  -  description
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
#include "KVSiB.h"
#include "KVTelescope.h"
#include "KVLinCal.h"
#include "KVPulseHeightDefect.h"
#include "TClass.h"

ClassImp(KVSiB)

KVSiB::KVSiB(): fLinCal(0)
{
   //Default ctor. Make a silicon detector with type="SIB"
   SetType("SIB");
}

KVSiB::KVSiB(Float_t thickness): KVSilicon(thickness), fLinCal(0)
{
   // Make a 'thickness' um thick silicon detector with type="SIB"
   SetType("SIB");
}

//___________________________________________________________________________

KVSiB::~KVSiB()
{
}

//___________________________________________________________________________

const Char_t* KVSiB::GetArrayName()
{
   //Give name of detector as SIB_x with x=number of blocking telescope

   fFName = TString("SIB_") + Form("%d", GetTelescope()->GetNumber());
   return fFName.Data();
}

//____________________________________________________________________________

void KVSiB::SetACQParams()
{
   //Set up data acquisition parameters for this blocking silicon
   AddACQParamType("P1");
   AddACQParamType("P2");
   AddACQParamType("P3");
   AddACQParamType("P4");
   AddACQParamType("E");
   AddACQParamType("P1_GG");
   AddACQParamType("P2_GG");
   AddACQParamType("P3_GG");
   AddACQParamType("P4_GG");
   AddACQParamType("E_GG");
}

//____________________________________________________________________________

Bool_t KVSiB::Fired(Option_t* opt) const
{
   //opt="any" (default):
   //Returns true if ANY of the acquisition parameters associated with the detector were fired in an event & the associated BIC fired (any)
   //opt="all" :
   //Returns true if ALL of the acquisition parameters associated with the detector were fired in an event & the associated BIC fired (all)

   return (KVDetector::Fired(opt) && GetChIo()->Fired(opt));
}

//____________________________________________________________________________

KVBIC* KVSiB::GetBIC() const
{
   //Returns pointer to BIC associated to this SIB i.e. placed directly in front
   return (KVBIC*)GetChIo();
}

//__________________________________________________________________________________________________________________________

void KVSiB::SetCalibrators()
{
   //Adds linear calibration for E channel
   KVLinCal* cal = new KVLinCal(this);
   cal->SetChannelParameter("E");
   cal->WithPedestalCorrection(kFALSE);
   if (!AddCalibrator(cal)) delete cal;
   fLinCal = (KVLinCal*)GetCalibrator("Linear calibration E");
   KVCalibrator* c = new KVPulseHeightDefect(this);
   if (!AddCalibrator(c)) delete c;
   fPHD  = (KVPulseHeightDefect*) GetCalibrator("Pulse Height Defect");
}

//______________________________________________________________________________

void KVSiB::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVSiB.
   // We set the pointers to the calibrator objects
   // We add the linear calibrator if it is missing

   if (R__b.IsReading()) {
      KVSiB::Class()->ReadBuffer(R__b, this);
      fLinCal = (KVLinCal*)GetCalibrator("Linear calibration E");
      fPHD  = (KVPulseHeightDefect*) GetCalibrator("Pulse Height Defect");
   }
   else {
      KVSiB::Class()->WriteBuffer(R__b, this);
   }
}

//__________________________________________________________________________________________________________________________

Double_t KVSiB::GetEnergy() const
{
   //Redefinition of KVSilicon::GetEnergy().
   //If energy lost in active layer is already set (e.g. by calculation of energy loss
   //of charged particles), return its value.
   //If not, we calculate it and set it using the values read from coders
   //and the linear channel-energy calibration if initialised
   //
   //Returns -99 if (i) no calibration present (ii) calibration present but no data in acquisition parameters

   //fELoss already set, return its value
   if (KVDetector::GetEnergy()) {
      return KVDetector::GetEnergy();
   }
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
