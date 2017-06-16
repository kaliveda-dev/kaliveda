/***************************************************************************
                          kvdetectedparticle.h  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVFAZIAReconNuc.h,v 1.39 2009/04/03 14:28:37 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVFAZIAReconNuc_H
#define KVFAZIAReconNuc_H


#include "KVReconstructedNucleus.h"

class KVFAZIADetector;

class KVFAZIAReconNuc: public KVReconstructedNucleus {

protected:
   Float_t fECSI;//csi contribution to energy
   Float_t fESI1;//si1 contribution to energy
   Float_t fESI2;//si2 contribution to energy

public:

   KVFAZIAReconNuc();
   KVFAZIAReconNuc(const KVFAZIAReconNuc&);
   void init();
   virtual ~ KVFAZIAReconNuc();

   virtual void Copy(TObject&) const;

   virtual void Clear(Option_t* t = "");
   void Print(Option_t* option = "") const;
   virtual void Identify();

   virtual Bool_t CoherencySi(KVIdentificationResult& theID);
   virtual Bool_t CoherencySiSi(KVIdentificationResult& theID);
   virtual Bool_t CoherencySiCsI(KVIdentificationResult& theID);

   virtual void Calibrate();

   Float_t GetEnergySI1()
   {
      // Return the calculated SI1 contribution to the particle's energy
      //
      // This may be negative, in case the SI1 contribution was calculated
      // because either (1) the SI1 was not calibrated, or (2) coherency check
      // between all identifications indicates contribution
      // of several particles to SI1 energy

      return fESI1;
   };
   Float_t GetEnergySI2()
   {
      // Return the calculated SI2 contribution to the particle's energy
      //
      // This may be negative, in case the SI2 contribution was calculated
      // because all identifications indicates contribution
      // of several particles to SI2 energy

      return fESI2;
   };
   Float_t GetEnergyCSI()
   {
      // Return the calculated CsI contribution to the particle's energy
      return fECSI;
   };

   KVFAZIADetector* Get(const Char_t* label) const;
   KVFAZIADetector* GetSI1() const;
   KVFAZIADetector* GetSI2() const;
   KVFAZIADetector* GetCSI() const;

   Int_t GetIndex() const;
   Int_t GetIdentifierOfStoppingDetector() const;

   Bool_t StoppedIn(const Char_t* dettype) const;
   Bool_t StoppedInSI1() const;
   Bool_t StoppedInSI2() const;
   Bool_t StoppedInCSI() const;

   void ComputePSA();

   ClassDef(KVFAZIAReconNuc, 3) //Nucleus identified by FAZIA array
};

#endif
