/***************************************************************************
$Id: KVLightEnergyCsI.h,v 1.3 2007/02/27 11:56:33 franklan Exp $
                          KVLightEnergyCsI.h  -  description
                             -------------------
    begin                : 18/5/2004
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
#ifndef KV_LIGHT_ENERGY_CSI_H
#define KV_LIGHT_ENERGY_CSI_H

#include "KVCalibrator.h"

class KVLightEnergyCsI: public KVCalibrator {

protected:
   UInt_t fZ;                   //!Z of nucleus to be calibrated
   UInt_t fA;                   //!A of nucleus to be calibrated

   static TF1 fLight;           //function parameterising light output as function of (energy, Z, A)

   void SetParametersOfLightEnergyFunction() const;

public:
   KVLightEnergyCsI();
   KVLightEnergyCsI(KVDetector* kvd);
   virtual ~ KVLightEnergyCsI()
   {
   };

   void init();
   virtual Double_t Compute(Double_t chan) const;
   virtual Double_t operator()(Double_t chan);
   virtual Double_t Invert(Double_t);

   void SetZ(UInt_t z)
   {
      fZ = z;
   };
   void SetA(UInt_t a)
   {
      fA = a;
   };
   UInt_t GetZ() const
   {
      return fZ;
   };
   UInt_t GetA() const
   {
      return fA;
   };

   TF1* GetLightEnergyFunction(UInt_t Z, UInt_t A);

   ClassDef(KVLightEnergyCsI, 1)        //Light-energy calibration for CsI detectors
};

#endif
