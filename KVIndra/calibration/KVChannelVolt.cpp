/***************************************************************************
$Id: KVChannelVolt.cpp,v 1.17 2008/12/08 14:07:37 franklan Exp $
                          KVChannelVolt.cpp  -  description
                             -------------------
    begin                : mer sep 18 2002
    copyright            : (C) 2002 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVChannelVolt.h"
#include "TMath.h"

ClassImp(KVChannelVolt);

//___________________________________________________________________________
//Calibration object dedicated to handling the electronic calibration (conversion
//channel->volts) of Si and ChIo detectors. The transfer function is a quadratic
//function
//
//        Signal(Volts) = a0 + a1*Channel + a2*Channel^2
//
//determined by injecting a pre-determined sequence of impulsions
//of different amplitudes into the "test" entry of the detector pre-amplifiers and
//recording the corresponding channels in the coders.
//
//The pedestal for the corresponding run is not subtracted before fitting,
//therefore the Inverse operation (GetChannel(V=0)) does not return 0 but rather
//an estimation of the pedestal for the run.
//When used for different runs where the pedestal has changed, the change in pedestal
//should be corrected (this is done in KVSilicon and KVChIo).
//
//Depending on the signal type ("PG" or "GG") the type of the calibrator will
//be "Channel-Volt PG" or "Channel-Volt GG".
//___________________________________________________________________________

KVChannelVolt::KVChannelVolt(): KVCalibrator(3)
{
   SetType("Channel-Volt");
   SetGainRef(1);
}

//___________________________________________________________________________
KVChannelVolt::KVChannelVolt(const Char_t* signal, KVDetector* kvd): KVCalibrator
   (3)
{
   //Create an electronic calibration object for a specific detector (*kvd)
   //specifying the signal type ("PG" - petit "low" gain - or "GG" - grand "high" gain)
   SetType("Channel-Volt");
   SetDetector(kvd);
   SetSignal(signal);
   SetGainRef(1);
}

//___________________________________________________________________________
Double_t KVChannelVolt::Compute(Double_t chan) const
{
   Double_t gain = 1.;
   KVDetector* det = GetDetector();
   if (det)
      gain = det->GetGain();
   //Calculate the calibrated signal strength in volts for a given channel number.
   if (fReady) {
      return (fPar[0] + fPar[1] * chan + fPar[2] * chan * chan) * gain_ref / gain;
   } else {
      return 0.;
   }
}


//___________________________________________________________________________
Double_t KVChannelVolt::operator()(Double_t chan)
{
   //Overloading of "()" to allow syntax such as:
   //
   //        KVChannelVolt calibrator;
   //           ....
   //        Double_t calibrated_volts = calibrator(channel);
   //
   //equivalently to:
   //
   //        Double_t calibrated_volts = calibrator.Compute(channel);
   return Compute(chan);
}

//___________________________________________________________________________
void KVChannelVolt::SetSignal(const Char_t* signal)
{
   //Set the type of signal treated ("PG" - petit "low" gain - or "GG" - grand "high" gain)
   strcpy(fSignal, signal);
   fName.Append("_");
   fName.Append(signal);
   fTitle.Append(" ");
   fTitle.Append(signal);
}

//___________________________________________________________________________
Double_t KVChannelVolt::Invert(Double_t volts)
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
         c = fPar[1] * fPar[1] - 4. * fPar[2] * (fPar[0] - gain / gain_ref * volts);
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
         channel = (Int_t)(0.5 + (gain / gain_ref * volts - fPar[0]) / fPar[1]);
      }
   } else {
      Warning("Compute", "Parameters not correctly initialized");
   }
   return (Double_t) channel;
}

//___________________________________________________________________________
Double_t KVChannelVolt::InvertDouble(Double_t volts)
{
   Double_t gain = 1.;
   KVDetector* det = GetDetector();
   if (det)
      gain = det->GetGain();
   Double_t channel = 0;

   if (fReady) {
      if (fPar[2]) {
         // quadratic transfer function
         Double_t c;
         c = fPar[1] * fPar[1] - 4. * fPar[2] * (fPar[0] - gain / gain_ref * volts);
         if (c < 0.0)
            return -1;
         c = (-fPar[1] + TMath::Sqrt(c)) / (2.0 * fPar[2]);
         if (c < 0.0
               && ((-fPar[1] - TMath::Sqrt(c)) / (2.0 * fPar[2])) > 0.0) {
            c = (-fPar[1] - TMath::Sqrt(c)) / (2.0 * fPar[2]);
         }
         channel = c;
      } else {
         // linear transfer function
         channel = (gain / gain_ref * volts - fPar[0]) / fPar[1];
      }
   } else {
      Warning("Compute", "Parameters not correctly initialized");
   }
   return (Double_t) channel;
}

//___________________________________________________________________________
void KVChannelVolt::SetGainRef(Double_t ref)
{
   gain_ref = ref;
}

//___________________________________________________________________________
Double_t KVChannelVolt::GetGainRef(void)
{
   return gain_ref;
}
