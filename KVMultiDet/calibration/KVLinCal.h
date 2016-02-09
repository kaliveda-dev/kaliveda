/***************************************************************************
$Id: KVLinCal.h,v 1.3 2007/03/05 14:09:03 franklan Exp $
                          KVChannelEnergyChIo.h  -  description
                             -------------------
    begin                : May 12th 2004
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
#ifndef KV_LINCAL_H
#define KV_LINCAL_H

#include "KVCalibrator.h"
#include "KVString.h"
#include "KVACQParam.h"

class KVLinCal : public KVCalibrator {

   KVString fACQParam;//acquisition parameter to use for calibration
   Bool_t      fPedCorr;//true if pedestal correction is required
   KVACQParam* fPar;//!non-persistent pointer to DAQ parameter named in fACQParam

   KVACQParam* GetParam() const
   {
      return (fPar ? fPar : (const_cast<KVLinCal*>(this)->fPar = GetDetector()->GetACQParam(fACQParam.Data())));
   };

public:
   KVLinCal();
   KVLinCal(KVDetector* kvd);
   virtual ~ KVLinCal()
   {
   };

   virtual Double_t Compute(Double_t) const;
   virtual Double_t operator()(Double_t);
   virtual Double_t Invert(Double_t e);

   void WithPedestalCorrection(Bool_t yes)
   {
      fPedCorr = yes;
   };
   Bool_t WithPedestalCorrection() const
   {
      return fPedCorr;
   };
   void SetChannelParameter(const Char_t* type);
   const Char_t* GetChannelParameter()
   {
      return fACQParam.Data();
   };

   ClassDef(KVLinCal, 1)//Simple linear calibration E = a + b*channel
};

#endif
