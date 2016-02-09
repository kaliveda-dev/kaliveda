/***************************************************************************
$Id: KVChannelEnergyAlpha.cpp,v 1.5 2007/02/27 11:56:33 franklan Exp $
                          KVChannelEnergyAlpha.cpp  -  description
                             -------------------
    begin                : ven mai 30 2003
    copyright            : (C) 2003 by J.D. Frankland
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
#include "KVChannelEnergyAlpha.h"
#include "TMath.h"

ClassImp(KVChannelEnergyAlpha);

//___________________________________________________________________________
//Quadratic channel-energy(Mev) calibration.
//According to the signal type ("PG" or "GG") the type of the calibrator will be
//"Channel-Energy alpha PG" or "Channel-Energy alpha GG".
//___________________________________________________________________________

KVChannelEnergyAlpha::KVChannelEnergyAlpha(): KVCalibrator(3)
{
   SetType("Channel-Energy alpha");
}

//___________________________________________________________________________
KVChannelEnergyAlpha::KVChannelEnergyAlpha(Char_t* signal, KVDetector* kvd): KVCalibrator
   (3)
{
   //Create an electronic calibration object for a specific detector (*kvd)
   //specifying the signal type ("PG" - petit "low" gain - or "GG" - grand "high" gain)
   SetType("Channel-Energy alpha");
   SetDetector(kvd);
   SetSignal(signal);
}

//___________________________________________________________________________
Double_t KVChannelEnergyAlpha::Compute(Double_t chan) const
{
   Double_t gain = 1.;
   KVDetector* det = GetDetector();
   if (det)
      gain = det->GetGain();
   //Calculate the calibrated signal strength in volts for a given channel number.
   if (fReady) {
      return (fPar[0] + fPar[1] * chan + fPar[2] * chan * chan) / gain;
   } else {
      return 0.;
   }
}


//___________________________________________________________________________
Double_t KVChannelEnergyAlpha::operator()(Double_t chan)
{
   //Overloading of "()" to allow syntax such as:
   //
   //        KVChannelEnergyAlpha calibrator;
   //           ....
   //        Double_t calibrated_volts = calibrator(channel);
   //
   //equivalently to:
   //
   //        Double_t calibrated_volts = calibrator.Compute(channel);
   return Compute(chan);
}

//___________________________________________________________________________
void KVChannelEnergyAlpha::SetSignal(Char_t* signal)
{
   //Set the type of signal treated ("PG" - petit "low" gain - or "GG" - grand "high" gain)
   strcpy(fSignal, signal);
   fName.Append("_");
   fName.Append(signal);
   fTitle.Append(" ");
   fTitle.Append(signal);
}

//___________________________________________________________________________
Double_t KVChannelEnergyAlpha::Invert(Double_t volts)
{
   //Given the calibrated (or simulated) signal amplitude in volts,
   //calculate the corresponding channel number according to the
   //calibration parameters (useful for filtering simulations).

   Double_t gain = 1.;
   KVDetector* det = GetDetector();
   if (det)
      gain = det->GetGain();
   Int_t channel = 0;

   if (fReady) {
      if (fPar[2]) {
         // quadratic transfer function
         Double_t c;
         c = fPar[1] * fPar[1] - 4. * fPar[2] * (fPar[0] - gain * volts);
         if (c < 0.0)
            return -1;
         c = (-fPar[1] + TMath::Sqrt(c)) / (2.0 * fPar[2]);
         if (c < 0.0
               && ((-fPar[1] - TMath::Sqrt(c)) / (2.0 * fPar[2])) > 0.0) {
            c = (-fPar[1] - TMath::Sqrt(c)) / (2.0 * fPar[2]);
         }
         channel = (Int_t)(c + 0.5);
      } else {
         // linear transfer function
         channel = (Int_t)(0.5 + (gain * volts - fPar[0]) / fPar[1]);
      }
   } else {
      Warning("Compute", "Parameters not correctly initialized");
   }
   return (Double_t) channel;
}
