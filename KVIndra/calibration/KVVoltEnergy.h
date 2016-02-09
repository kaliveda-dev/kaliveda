/***************************************************************************
                          KVVoltEnergy.h  -  description
                             -------------------
    begin                : jeu oct 17 2002
    copyright            : (C) 2002 by Alexis Mignon
    email                : mignon@ganil.fr

$Id: KVVoltEnergy.h,v 1.2 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KV_VOLT_ENERGY_H
#define KV_VOLT_ENERGY_H

#include "KVCalibrator.h"

class KVVoltEnergy: public KVCalibrator {
public:
   KVVoltEnergy();
   KVVoltEnergy(KVDetector* kvd);
   virtual ~ KVVoltEnergy()
   {
   };

   Double_t Compute(Double_t volts) const;
   Double_t operator()(Double_t volts);
   Double_t Invert(Double_t energy);

   ClassDef(KVVoltEnergy, 1)    //Volt-Energy calibration for Si and ChIo detectors.
};
#endif
