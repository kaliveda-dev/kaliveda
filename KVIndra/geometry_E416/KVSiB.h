/***************************************************************************
                          KVSiB.h  -  description
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

#ifndef KVSIB_H
#define KVSIB_H

#include "KVSilicon.h"
#include "KVLinCal.h"

class KVBIC;

class KVSiB: public KVSilicon {

   KVLinCal* fLinCal;//!linear channel-energy calibration

public:
   KVSiB();
   KVSiB(Float_t thickness);
   virtual ~ KVSiB();

   void SetACQParams();
   const Char_t* GetArrayName();
   virtual Bool_t Fired(Option_t* opt = "any") const;

   Float_t GetP1()
   {
      return GetACQData("P1");
   };
   Float_t GetP2()
   {
      return GetACQData("P2");
   };
   Float_t GetP3()
   {
      return GetACQData("P3");
   };
   Float_t GetP4()
   {
      return GetACQData("P4");
   };
   Float_t GetE()
   {
      return GetACQData("E");
   };

   KVBIC* GetBIC() const;

   void SetCalibrators();
   Double_t GetEnergy() const;
   //returns kTRUE if linear calibrator fLinCal is present and has been initialised
   Bool_t IsCalibrated() const
   {
      return (fLinCal && fLinCal->GetStatus());
   };

   ClassDef(KVSiB, 2)           //Position sensitive silicon detector for E416 blocking telescope
};

#endif
