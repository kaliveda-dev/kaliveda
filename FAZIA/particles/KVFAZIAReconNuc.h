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
   Float_t fECsI;//csi contribution to energy
   Float_t fESi1;//si1 contribution to energy
   Float_t fESi2;//si2 contribution to energy

   /*
   void DoNeutronCalibration();
   void DoBeryllium8Calibration();
   void DoGammaCalibration();
   Bool_t CalculateSiliconDEFromResidualEnergy();
   void CalculateSiLiDEFromResidualEnergy(Double_t ERES);
   void CalculateSi75DEFromResidualEnergy(Double_t ERES);
   void CalculateChIoDEFromResidualEnergy(Double_t ERES);
   */
   virtual void MakeDetectorList();

public:

   /*
   Bool_t AreSiCsICoherent() const
   {
      // RINGS 1-9
      // Returns result of coherency test between Si-CsI and CsI-RL identifications.
      // See CoherencySiCsI(KVIdentificationResult&).
      return fCoherent;
   };
    Bool_t IsSiPileup() const
    {
        // RINGS 1-9
        // Returns result of coherency test between Si-CsI and CsI-RL identifications.
        // See CoherencySiCsI(KVIdentificationResult&).
        return fPileup;
    };
   */
   KVFAZIAReconNuc();
   KVFAZIAReconNuc(const KVFAZIAReconNuc&);
   void init();
   virtual ~ KVFAZIAReconNuc();

   virtual void Copy(TObject&) const;

   virtual void Clear(Option_t* t = "");
   void Print(Option_t* option = "") const;
   virtual void Identify();
   virtual void Calibrate();

   Float_t GetEnergySi1()
   {
      // Return the calculated ChIo contribution to the particle's energy
      // (including correction for losses in Mylar windows).
      // This may be negative, in case the ChIo contribution was calculated
      // because either (1) the ChIo was not calibrated, or (2) coherency check
      // between ChIo-Si and Si-CsI/CsI-RL identification indicates contribution
      // of several particles to ChIo energy

      return fESi1;
   };
   Float_t GetEnergySi2()
   {
      // Return the calculated Si contribution to the particle's energy
      // (including correction for pulse height defect).
      // This may be negative, in case the Si contribution was calculated
      // because either (1) the Si was not calibrated, or (2) coherency check
      // indicates pileup in Si, or (3) coherency check indicates measured
      // Si energy is too small for particle identified in CsI-RL

      return fESi2;
   };
   Float_t GetEnergyCsI()
   {
      // Return the calculated CsI contribution to the particle's energy
      return fECsI;
   };

   KVFAZIADetector* Get(const Char_t* label) const;
   KVFAZIADetector* GetSI1() const;
   KVFAZIADetector* GetSI2() const;
   KVFAZIADetector* GetCSI() const;
   Bool_t StoppedIn(const Char_t* dettype) const;
   Bool_t StoppedInSI1() const;
   Bool_t StoppedInSI2() const;
   Bool_t StoppedInCSI() const;

   ClassDef(KVFAZIAReconNuc, 2) //Nucleus identified by FAZIA array
};

#endif
