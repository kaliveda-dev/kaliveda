/***************************************************************************
$Id: KVChannelEnergyAlpha.h,v 1.4 2007/02/27 11:56:33 franklan Exp $
                          KVChannelEnergyAlpha.h  -  description
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
#ifndef KV_CHANNEL_ENERGY_ALPHA_H
#define KV_CHANNEL_ENERGY_ALPHA_H

#include "KVCalibrator.h"

class KVChannelEnergyAlpha: public KVCalibrator {
protected:
   Char_t fSignal[3];
public:
   KVChannelEnergyAlpha();
   KVChannelEnergyAlpha(Char_t* signal, KVDetector* kvd);
   virtual ~ KVChannelEnergyAlpha()
   {
   };

   virtual Double_t Compute(Double_t chan) const;
   virtual Double_t operator()(Double_t chan);
   virtual Double_t Invert(Double_t volts);

   virtual void SetSignal(Char_t* signal);

   ClassDef(KVChannelEnergyAlpha, 1)    //Alpha channel-energy calibration for Si and ChIo detectors.
};
#endif
