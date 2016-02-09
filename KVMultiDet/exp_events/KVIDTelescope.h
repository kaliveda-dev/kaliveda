/***************************************************************************
$Id: KVIDTelescope.h,v 1.33 2009/04/01 15:58:10 ebonnet Exp $
                          KVIDTelescope.h  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by J.D Frankland
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

#ifndef KVIDTELESCOPE_H
#define KVIDTELESCOPE_H

#include "KVBase.h"
#include "KVDetector.h"
#include "KVRList.h"
#include "TGraph.h"

class KVReconstructedNucleus;
class KVGroup;
class KVIDGraph;
class KVIDGrid;
class KVMultiDetArray;
class KVIdentificationResult;
class TH2;

class KVIDTelescope: public KVBase {

protected:
   KVList* fDetectors;          //->list of detectors in telescope
   KVGroup* fGroup;             //group to which telescope belongs
   KVList* fIDGrids;           //->identification grid(s)
   enum {
      kMassID = BIT(15),         //set if telescope is capable of mass identification i.e. isotopic resolution
      kReadyForID = BIT(16)         //set if telescope is ready and able for identification. set in Initialize()
   };
   Int_t fCalibStatus;//!temporary variable holding status code for last call to Calibrate(KVReconstructedNucleus*)

   void SetLabelFromURI(const Char_t* uri);

   KVString fVarX;
   KVString fVarY;

public:

   // status of particle calibration after Calibrate(KVReconstructedNucleus*) is called
   enum {
      kCalibStatus_OK,                    // fine, OK, all detectors calculated and functioning properly
      kCalibStatus_Calculated,        // one or more detectors not calibrated/functioning, energy loss calculated
      kCalibStatus_NoCalibrations,  // no calibrations available for any detectors
      kCalibStatus_Multihit,  // one or more detectors hit by more than one particle, energy loss calculated for this detector
      kCalibStatus_Coherency  // comparison of calculated and measured energy loss in one or more detectors reveals presence of other particles
   };

   KVIDTelescope();
   virtual ~ KVIDTelescope();
   void init();
   virtual void AddDetector(KVDetector* d);
   KVList* GetDetectors() const
   {
      return fDetectors;
   };
   KVDetector* GetDetector(UInt_t n) const
   {
      //returns the nth detector in the telescope structure
      if (n > GetSize() || n < 1) {
         Error("GetDetector(UInt_t n)", "n must be between 1 and %u",
               GetSize());
         return 0;
      }
      KVDetector* d = (KVDetector*) fDetectors->At((n - 1));
      return d;
   };
   KVDetector* GetDetector(const Char_t* name) const;
   UInt_t GetDetectorRank(KVDetector* kvd);
   UInt_t GetSize() const
   {
      //returns number of detectors in telescope
      return (fDetectors ? fDetectors->GetSize() : 0);
   };

   KVGroup* GetGroup() const;
   void SetGroup(KVGroup* kvg);
   UInt_t GetGroupNumber();

   const Char_t* GetVarX() const
   {
      return fVarX.Data();
   }
   const Char_t* GetVarY() const
   {
      return fVarY.Data();
   }


   virtual TGraph* MakeIDLine(KVNucleus* nuc, Double_t Emin, Double_t Emax,
                              Double_t Estep = 0.0);

   virtual void Initialize(void)
   {
      // Default initialisation for ID telescopes. kReadyForID is set to kFALSE.
      // To implement identification, make a class derived from KVIDTelescope
      // and set kReadyForID to kTRUE in Initialize() method of derived class.
      ResetBit(kReadyForID);
   };

   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual void CalculateParticleEnergy(KVReconstructedNucleus* nuc);
   virtual Int_t GetCalibStatus() const
   {
      // When called just after CalculateParticleEnergy(KVReconstructedNucleus*)
      // this method returns a status code which is one of
      //   KVIDTelescope::kCalibStatus_OK,            : fine, OK, all detectors calculated and functioning properly
      //   KVIDTelescope::kCalibStatus_Calculated,    : one or more detectors not calibrated/functioning, energy loss calculated
      //   KVIDTelescope::kCalibStatus_NoCalibrations : no calibrations available for any detectors
      return fCalibStatus;
   };

   virtual void Print(Option_t* opt = "") const;

   void SetIDGrid(KVIDGraph*);
   virtual KVIDGraph* GetIDGrid();
   virtual KVIDGraph* GetIDGrid(Int_t);
   virtual KVIDGraph* GetIDGrid(const Char_t*);
   virtual KVList* GetListOfIDGrids() const
   {
      return fIDGrids;
   }

   virtual void RemoveGrids();

   virtual Double_t GetIDMapX(Option_t* opt = "");
   virtual Double_t GetIDMapY(Option_t* opt = "");
   virtual Double_t GetPedestalX(Option_t* opt = "");
   virtual Double_t GetPedestalY(Option_t* opt = "");

   void SetHasMassID(Bool_t yes = kTRUE)
   {
      SetBit(kMassID, yes);
   };

   // Returns kTRUE if this telescope is capable of identifying particles' mass (A)
   // as well as their charge (Z).
   Bool_t HasMassID() const
   {
      return TestBit(kMassID);
   };

   static KVIDTelescope* MakeIDTelescope(const Char_t* name);

   virtual Bool_t SetIdentificationParameters(const KVMultiDetArray*);
   virtual void RemoveIdentificationParameters();

   // Returns kTRUE if telescope has been correctly initialised for identification.
   // Test after Initialize() method has been called.
   virtual Bool_t IsReadyForID()
   {
      return TestBit(kReadyForID);
   };

   const Char_t* GetDefaultIDGridClass();
   KVIDGrid* CalculateDeltaE_EGrid(const Char_t* Zrange, Int_t deltaMasse, Int_t npoints, Double_t lifetime = -10/*s*/, UChar_t massformula = 0, Double_t xfactor = 1.);
   KVIDGrid* CalculateDeltaE_EGrid(TH2* haa_zz, Bool_t Zonly, Int_t npoints);
   // status codes for GetMeanDEFromID
   enum {
      kMeanDE_OK,       // all OK
      kMeanDE_XtooSmall,  // X-coordinate is smaller than smallest X-coordinate of ID line
      kMeanDE_XtooLarge,   // X-coordinate is larger than largest X-coordinate of ID line
      kMeanDE_NoIdentifier   // No identifier found for Z or (Z,A)
   };
   virtual Double_t GetMeanDEFromID(Int_t& status, Int_t Z, Int_t A = -1, Double_t Eres = -1.0);
   virtual UShort_t GetBadIDCode()
   {
      // return a general identification code (can be a bitmask) for particles badly identified
      // with this type of ID telescope
      // redefine in child classes; default returns 14.
      return 14;
   };
   virtual UShort_t GetCoherencyIDCode()
   {
      // return a general identification code (can be a bitmask) for particles identified
      // with this type of ID telescope after coherency analysis
      // redefine in child classes; default returns 6.
      return 6;
   };
   virtual UShort_t GetMultiHitFirstStageIDCode()
   {
      // return a general identification code (can be a bitmask) for particles which cannot
      // be identified correctly due to pile-up in a delta-E detector
      // redefine in child classes; default returns 8.
      return 8;
   };
   virtual UShort_t GetIDCode()
   {
      // return a general identification code (can be a bitmask) for particles correctly identified
      // with this type of ID telescope
      // redefine in child classes; default returns 4.
      return 4;
   };
   virtual UShort_t GetZminCode()
   {
      // return a general identification code (can be a bitmask) for particles partially identified
      // with an estimated lower-limit for their charge with this type of ID telescope
      // redefine in child classes; default returns 5.
      return 5;
   };
   virtual UChar_t GetECode()
   {
      // return a general calibration code (can be a bitmask) for particles correctly identified
      // and calibrated with this type of ID telescope
      // redefine in child classes; default returns 1.
      return 1;
   };

   virtual void SetIDCode(KVReconstructedNucleus*, UShort_t);

   virtual Bool_t CheckTheoreticalIdentificationThreshold(KVNucleus* /*ION*/, Double_t /*EINC*/ = 0.0);
   virtual Bool_t CanIdentify(Int_t Z, Int_t /*A*/)
   {
      // Used for filtering simulations
      // Returns kTRUE if this telescope is theoretically capable of identifying a given nucleus,
      // without considering thresholds etc.
      // By default this method returns true for Z>0, i.e. all KVIDTelescopes are in principle
      // dE-E telescopes used to identify charged ions.
      return (Z > 0);
   }

   ClassDef(KVIDTelescope, 5)   //A delta-E - E identification telescope
};

#endif
