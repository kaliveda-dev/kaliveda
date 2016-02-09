/***************************************************************************
$Id: KVLinCal.cpp,v 1.3 2007/03/05 14:09:03 franklan Exp $
                          KVLinCal.cpp  -  description
                             -------------------
    begin                :  May 12th 2004
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
#include "KVLinCal.h"
#include "TMath.h"

ClassImp(KVLinCal);

////////////////////////////////////////////////////////////////////////////
//KVLinCal
//
//Linear calibration function:
//
//   E  = par(1)  +  par(2) * channel
//
//The channel number used is taken from the acquisition parameter
//whose type is set using the SetChannelParameter() method.
//
//If WithPedestalCorrection(kTRUE) is called, the corresponding
//pedestal value for the acquisition parameter in question will be
//subtracted from the channel number, as
//
//  E  = par(1)  +  par(2)  *  (channel - pedestal)
//
//By default, no pedestal correction is performed.

KVLinCal::KVLinCal(): KVCalibrator(2)
{
   SetType("Linear calibration");
   fPedCorr = kFALSE;
   fPar = 0;
}

//___________________________________________________________________________
KVLinCal::KVLinCal(KVDetector* kvd): KVCalibrator(2)
{
   //Create a calibration object for a specific detector (*kvd)
   SetType("Linear calibration");
   SetDetector(kvd);
   fPedCorr = kFALSE;
   fPar = 0;
}

//___________________________________________________________________________

void KVLinCal::SetChannelParameter(const Char_t* type)
{
   //Set type of acquisition parameter associated with detector which is
   //to be used for the calibration
   fACQParam = type;
   SetType(Form("Linear calibration %s", type));
}

//___________________________________________________________________________
Double_t KVLinCal::Compute(Double_t chan) const
{
   //Calculate the calibrated energy.
   //if chan<0 channel number is taken directly from the chosen acquisition parameter of the associated detector
   //   -  returns -99 if the parameter has not fired (KVACQParam::Fired==kFALSE)
   //if chan>=0 we use value of chan
   //
   //Returns -99 if calibrator is not ready (parameters not set)

   if (fReady) {
      Double_t chan_corr;
      if (chan < 0) {
         if (!GetParam()->Fired()) return -99.;
         chan_corr = GetParam()->GetData();
      } else chan_corr = chan;

      if (fPedCorr) chan_corr -= GetParam()->GetPedestal();

      return (GetParameter(0) + GetParameter(1) * chan_corr);
   }
   return -99.;
}


//___________________________________________________________________________
Double_t KVLinCal::operator()(Double_t chan)
{
   //Overloading of "()" to allow syntax such as:
   //
   //        KVLinCal calibrator;
   //           ....
   //        Float_t calibrated_volts = calibrator(channel);
   //
   //equivalently to:
   //
   //        Float_t calibrated_volts = calibrator.Compute(channel);
   return Compute(chan);
}

//___________________________________________________________________________
Double_t KVLinCal::Invert(Double_t energy)
{
   //Given the calibrated (or simulated) energy in MeV,
   //calculate the corresponding channel number according to the
   //calibration parameters (useful for filtering simulations).

   KVDetector* det = GetDetector();
   Int_t channel = 0;

   if (fReady) {
      // linear transfer function
      if (fPedCorr) {
         channel = (Int_t)(0.5 + (energy - GetParameter(0)) / GetParameter(1)
                           + det->GetPedestal("GG"));
      } else {
         channel = (Int_t)(0.5 + (energy - GetParameter(0)) / GetParameter(1));
      }
   }
   return (Double_t) channel;
}
