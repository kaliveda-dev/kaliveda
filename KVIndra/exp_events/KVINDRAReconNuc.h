/***************************************************************************
                          kvdetectedparticle.h  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVINDRAReconNuc.h,v 1.39 2009/04/03 14:28:37 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVINDRADETPART_H
#define KVINDRADETPART_H


#include "KVReconstructedNucleus.h"
#include "KVINDRADetector.h"
#include "KVTelescope.h"
#include "KVINDRACodes.h"
#include "KVINDRAIDTelescope.h"

class KVChIo;
class KVSilicon;
class KVSi75;
class KVSiLi;
class KVCsI;

/**
\class KVINDRAReconNuc
\brief Nuclei reconstructed from data measured in the INDRA array.
\ingroup INDRAReconstruction

Most useful methods are already defined in parent classes KVReconstructedNucleus,
KVNucleus and KVParticle. Here we add a few useful INDRA-specific methods :

   GetRingNumber(), GetModuleNumber()
   GetTimeMarker()  -  returns time-marker of detector in which the particle stopped

   StoppedInChIo(), StoppedInSi(), StoppedInCsI()  -  information on the detector in
                                                      which the particle stopped
       ** Note1: to access the detector in question, use GetStoppingDetector() (see KVReconstructedNucleus)
       ** Note2: for a general test of the type of detector in which the particle stopped,
                 you can do one of the following e.g. to test if it stopped in an
                 ionisation chamber (assuming a particle pointer 'part'):

        if( !strcmp(part->GetStoppingDetector()->GetType(), "CI") ) { ... }
               //test the type of the detector - see INDRA detector classes for the different types
        if( part->GetStoppingDetector()->InheritsFrom("KVChIo") ) { ... }
               //test the inheritance of the class of the stopping detector

    GetChIo(), GetSi(), GetCsI()    -  return pointers to the detectors through which the particle passed
    GetEnergyChIo(), GetEnergySi(), GetEnergyCsI()   -  return the calculated contribution of each detector to the
                                                                                      particle's energy

Identification/Calibration status and codes
-------------------------------------------

Identified particles have IsIdentified()=kTRUE.
Calibrated particles have IsCalibrated()=kTRUE.

The KVINDRACodes object fCodes is used to store information on the identification and
calibration of the particle. You can access this object using GetCodes() and then use the
methods of KVINDRACodes/KVINDRACodeMask to obtain the information.

For example, you can obtain the VEDA identification code of a particle 'part' using

       part.GetCodes().GetVedaIDCode()  [ = 0, 1, 2, etc.]

Information on whether the particle's mass was measured :

       part.IsAMeasured()  [ = kTRUE or kFALSE]

Information on whether the particle's charge was measured :

       part.IsZMeasured()  [ = kTRUE or kFALSE]

You can also access information on the status codes returned
by the different identification telescopes used to identify the particle:
see GetIDSubCode() and GetIDSubCodeString().

Masses of reconstructed nuclei
------------------------------

When the nucleus' A is not measured, we estimate it from the identified Z.
By default the mass formula used is that of R.J. Charity (see KVNucleus::GetAFromZ).
     ** Note: for data previous to the 5th campaign converted to the KaliVeda format
     ** from old DSTs, we keep the masses from the Veda programme, corresponding to
     ** KVNucleus mass option kVedaMass.
IN ALL CASES THE RETURNED VALUE OF GetA() IS POSITIVE
*/

class KVINDRAReconNuc: public KVReconstructedNucleus {

   mutable KVINDRACodes fCodes;//VEDA6-style calibration and identification codes
   Bool_t fCoherent;//coherency of CsI & Si-CsI identifications
   Bool_t fPileup;//apparent pileup in Si, revealed by inconsistency between CsI & Si-CsI identifications
   Bool_t fUseFullChIoEnergyForCalib;//decided by coherency analysis
   Float_t fECsI;//csi contribution to energy
   Float_t fESi;//si contribution to energy
   Float_t fEChIo;//chio contribution to energy
   Float_t fESi75;//si75 contribution to energy
   Float_t fESiLi;//sili contribution to energy
   Bool_t fCorrectCalib;//!set to kTRUE in Streamer if calibration needs correction

   Bool_t fCoherentSi75SiLiCsI;// coherency of Si75-SiLi and SiLi-CsI/CsI identifications
   Bool_t fPileupChIo;//apparent pileup in ChIo, revealed by inconsistency between CsI & ChIo-CsI identifications
   Bool_t fPileupSiLi;//apparent pileup in SiLi, revealed by inconsistency between CsI & Si75-SiLi identifications
   Bool_t fPileupSi75;//apparent pileup in Si75, revealed by inconsistency between CsI/SiLi-CsI & ChIo-Si75 identifications
   Bool_t fIncludeEtalonsInCalibration;//for etalon modules:particle passed through Si75/SiLi

   void CheckCsIEnergy();

   void SetBadCalibrationStatus()
   {
      SetECode(kECode15);
      SetEnergy(-1.0);
   }
   void SetNoCalibrationStatus()
   {
      SetECode(kECode0);
      SetEnergy(0.0);
   }
   void DoNeutronCalibration();
   void DoBeryllium8Calibration();
   void DoGammaCalibration();
   Bool_t CalculateSiliconDEFromResidualEnergy();
   void CalculateSiLiDEFromResidualEnergy(Double_t ERES);
   void CalculateSi75DEFromResidualEnergy(Double_t ERES);
   void CalculateChIoDEFromResidualEnergy(Double_t ERES);

public:

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
   Bool_t IsSi75Pileup() const
   {
      // RINGS 10-17
      // Returns result of coherency tests in etalon modules
      // See CoherencyEtalons(KVIdentificationResult&).
      return fPileupSi75;
   };
   Bool_t IsSiLiPileup() const
   {
      // RINGS 10-17
      // Returns result of coherency tests in etalon modules
      // See CoherencyEtalons(KVIdentificationResult&).
      return fPileupSiLi;
   };
   Bool_t IsChIoPileup() const
   {
      // RINGS 10-17
      // Returns result of coherency test between ChIo-CsI and CsI-RL identifications.
      // See CoherencyChIoCsI(KVIdentificationResult&).
      return fPileupChIo;
   };
   Bool_t UseFullChIoEnergyForCalib() const
   {
      // RINGS 1-9
      // Returns result of coherency test between ChIo-Si, Si-CsI and CsI-RL identifications.
      // See CoherencyChIoSiCsI(KVIdentificationResult).
      // RINGS 10-17
      // Returns kTRUE if there is just one particle in the ChIo, kFALSE if more

      return fUseFullChIoEnergyForCalib;
   };
   KVINDRAReconNuc();
   KVINDRAReconNuc(const KVINDRAReconNuc&);
   void init();
   virtual ~ KVINDRAReconNuc();

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   virtual void Clear(Option_t* t = "");
   void Print(Option_t* option = "") const;

   UInt_t GetRingNumber(void) const
   {
      if (GetTelescope()) {
         return GetTelescope()->GetRingNumber();
      }
      else {
         return 0;
      }
   };
   UInt_t GetModuleNumber(void) const
   {
      if (GetTelescope()) {
         return GetTelescope()->GetNumber();
      }
      else {
         return 0;
      }
   };

   virtual void Identify();
   virtual Bool_t CoherencySiCsI(KVIdentificationResult& theID);
   virtual Bool_t CoherencyChIoCsI(KVIdentificationResult& theID);
   virtual Bool_t CoherencyEtalons(KVIdentificationResult& theID);
   virtual Bool_t CoherencyChIoSiCsI(KVIdentificationResult);
   virtual void CalibrateRings1To9();
   virtual void CalibrateRings10To17();
   virtual void Calibrate();

   Float_t GetEnergyChIo()
   {
      // Return the calculated ChIo contribution to the particle's energy
      // (including correction for losses in Mylar windows).
      // This may be negative, in case the ChIo contribution was calculated
      // because either (1) the ChIo was not calibrated, or (2) coherency check
      // between ChIo-Si and Si-CsI/CsI-RL identification indicates contribution
      // of several particles to ChIo energy

      return fEChIo;
   };
   Float_t GetEnergySi()
   {
      // Return the calculated Si contribution to the particle's energy
      // (including correction for pulse height defect).
      // This may be negative, in case the Si contribution was calculated
      // because either (1) the Si was not calibrated, or (2) coherency check
      // indicates pileup in Si, or (3) coherency check indicates measured
      // Si energy is too small for particle identified in CsI-RL

      return fESi;
   };
   Float_t GetEnergySi75()
   {
      // Return the calculated Si75 contribution to the particle's energy

      return fESi75;
   };
   Float_t GetEnergySiLi()
   {
      // Return the calculated SiLi contribution to the particle's energy

      return fESiLi;
   };
   Float_t GetEnergyCsI()
   {
      // Return the calculated CsI contribution to the particle's energy
      return fECsI;
   };

   void SetEnergyChIo(Float_t val)
   {
      // Set the calculated ChIo contribution to the particle's energy
      // (including correction for losses in Mylar windows).
      // in veda6 de1+de_mylar

      fEChIo = val;
   };
   void SetEnergySi(Float_t val)
   {
      // Set the calculated Si contribution to the particle's energy
      // (including correction for pulse height defect).
      // This may be negative, in case the Si contribution was calculated
      // because either (1) the Si was not calibrated, or (2) coherency check
      // indicates pileup in Si, or (3) coherency check indicates measured
      // Si energy is too small for particle identified in CsI-RL

      fESi = val;
   };
   void SetEnergySi75(Float_t val)
   {
      // Set the calculated Si75 contribution to the particle's energy
      fESi75 = val;
   };
   void SetEnergySiLi(Float_t val)
   {
      // Set the calculated SiLi contribution to the particle's energy
      fESiLi = val;
   };
   void SetEnergyCsI(Float_t val)
   {
      // Set the calculated CsI contribution to the particle's energy
      fECsI = val;
   };

   KVChIo* GetChIo();
   KVSilicon* GetSi();
   KVSi75* GetSi75();
   KVSiLi* GetSiLi();
   KVCsI* GetCsI();
   Bool_t StoppedInChIo();
   Bool_t StoppedInSi();
   Bool_t StoppedInSi75();
   Bool_t StoppedInSiLi();
   Bool_t StoppedInCsI();

   KVINDRACodes& GetCodes() const
   {
      return fCodes;
   }
   virtual void SetIDCode(UShort_t code_mask)
   {
      // Sets code for identification
      GetCodes().SetIDCode(code_mask);
      KVReconstructedNucleus::SetIDCode(GetCodes().GetVedaIDCode());
   }

   virtual void SetECode(UChar_t code_mask)
   {
      //Sets code for energy calibration
      GetCodes().SetECode(code_mask);
      KVReconstructedNucleus::SetECode(GetCodes().GetVedaECode());
   }
   virtual Int_t GetIDCode() const
   {
      // Returns value of VEDA ID code
      return GetCodes().GetVedaIDCode();
   }
   virtual Int_t GetECode() const
   {
      // Return value of VEDA E code
      return GetCodes().GetVedaECode();
   }

   Int_t GetIDSubCode(const Char_t* id_tel_type = "") const;
   const Char_t* GetIDSubCodeString(const Char_t* id_tel_type = "") const;
   KVINDRATelescope* GetTelescope() const
   {
      //Return pointer to telescope (detector stack) in which the particle is detected
      return (GetStoppingDetector() ?  dynamic_cast<KVINDRADetector*>(GetStoppingDetector())->GetTelescope() : 0);
   }

   ClassDef(KVINDRAReconNuc, 12) //Nucleus identified by INDRA array
};

#endif
