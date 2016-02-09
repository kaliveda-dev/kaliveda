/***************************************************************************
$Id: KVChannelVolt.h,v 1.14 2008/12/08 14:07:37 franklan Exp $
                         KVChannelVolt.h  -  description
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
#ifndef KV_CHANNEL_VOLT_H
#define KV_CHANNEL_VOLT_H

#include "KVCalibrator.h"

class KVChannelVolt: public KVCalibrator {
protected:
   Char_t fSignal[3];
   Double_t gain_ref;
public:
   KVChannelVolt();
   KVChannelVolt(const Char_t* signal, KVDetector* kvd);
   virtual ~ KVChannelVolt()
   {
   };

   virtual Double_t Compute(Double_t chan) const;
   virtual Double_t operator()(Double_t chan);
   virtual Double_t Invert(Double_t volts);
   virtual Double_t InvertDouble(Double_t volts);
   void SetGainRef(Double_t ref);
   Double_t GetGainRef(void);

   virtual void SetSignal(const Char_t* signal);

   ClassDef(KVChannelVolt, 1)   //Electronic calibration for Si and ChIo detectors.
};
#endif
