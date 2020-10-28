/***************************************************************************
$Id: KVDBPeak.h,v 1.4 2007/04/02 17:43:38 ebonnet Exp $
                          KVDBPeak.h  -  description
                             -------------------
    begin                : december 5th 2003
    copyright            : (C) 2003 by John D. Frankland
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
#ifndef __KVDBPEAK_H
#define __KVDBPEAK_H

#include "KVDBParameterSet.h"
#include "KVINDRARRMValidator.h"
#include "TString.h"

/**
\class KVDBPeak
\brief Calibration peak in database
\ingroup INDRADB

Wrapper for KVDBParameterSet used as base class for classes
handling calibration peaks: KVDBAlphaPeak, KVDBElasticPeak, (KVDBBrhoPeak ?)
 */

class KVDBPeak: public KVDBParameterSet, public KVINDRARRMValidator {

private:

   Bool_t fAvecGaz;             //=kTRUE if gaz present in chio
   TString fDetector;           //name of detector
   TString fSignal;             //=signal type
   Float_t fGain;               //gain of ampli for Si = 1.00 or 1.41
   Float_t fEnergy;             //theoretical energy of peak
   UChar_t fRing;               //ring number of Si or CsI (in coinc with ChIo if peak is in ChIo)
   UChar_t fModule;             //module number of Si or CsI (in coinc with ChIo if peak is in ChIo)

public:

   KVDBPeak();
   KVDBPeak(const Char_t* detname, const Char_t* type, UShort_t pnum);
   virtual ~ KVDBPeak()
   {
   };

   void init();

   void SetGas(Bool_t avec = kTRUE)
   {
      fAvecGaz = avec;
   };
   Bool_t WithGas() const
   {
      return fAvecGaz;
   };

   void SetDetector(const Char_t* name)
   {
      fDetector = name;
   };
   const Char_t* GetDetector() const
   {
      return fDetector.Data();
   };

   void SetSigType(const Char_t* sig)
   {
      fSignal = sig;
   };
   const Char_t* GetSigType() const
   {
      return fSignal.Data();
   };

   void SetGain(Float_t gain)
   {
      fGain = gain;
   };
   Float_t GetGain() const
   {
      return fGain;
   };

   void SetEnergy(Float_t e)
   {
      fEnergy = e;
   };
   Float_t GetEnergy() const
   {
      return fEnergy;
   };

   void SetRing(UChar_t ring)
   {
      fRing = ring;
   };
   UChar_t GetRing() const
   {
      return fRing;
   };

   void SetModule(UChar_t mod)
   {
      fModule = mod;
   };
   UChar_t GetModule() const
   {
      return fModule;
   };

   Double_t GetCentroid()
   {
      return GetParameter(0);
   };
   Double_t GetError()
   {
      return GetParameter(1);
   };
   Double_t GetPeakWidth()
   {
      return GetParameter(2);
   }

   ClassDef(KVDBPeak, 2)        //calibration points such as alpha, elastic, Brho
};

#endif
