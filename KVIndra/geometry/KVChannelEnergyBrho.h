/***************************************************************************
$Id: KVChannelEnergyBrho.h,v 1.5 2007/02/27 11:56:33 franklan Exp $
                          KVChannelEnergyBrho.h  -  description
                             -------------------
    begin                : Feb 7th 2004
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
#ifndef KV_CHANNEL_ENERGY_BRHO_H
#define KV_CHANNEL_ENERGY_BRHO_H

#include "KVCalibrator.h"

class KVChannelEnergyBrho: public KVCalibrator {

public:
   KVChannelEnergyBrho();
   KVChannelEnergyBrho(KVDetector* kvd);
   virtual ~ KVChannelEnergyBrho()
   {
   };

   virtual Double_t Compute(Double_t chan) const;
   virtual Double_t operator()(Double_t chan);
   virtual Double_t Invert(Double_t volts);

   Double_t GetChi2() const
   {
      return GetParameter(2);
   };

   ClassDef(KVChannelEnergyBrho, 1)     //Pedestal-corrected linear channel-energy calibration from Brho data for Si and ChIo detectors.
};

#endif
