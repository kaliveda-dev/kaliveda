/***************************************************************************
                          kvindra.h  -  description
                             -------------------
    begin                : Mon May 20 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVINDRA.h,v 1.43 2009/01/21 10:05:51 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVINDRA_H
#define KVINDRA_H

#include "TEnv.h"
#include "KVASMultiDetArray.h"
#include "KVList.h"
#include "KVHashList.h"
#include "KVDBSystem.h"
#include "KVUpDater.h"
#include "KVDataSetManager.h"
#include "KVINDRATriggerInfo.h"
#include "KVINDRADetector.h"
#include "KVINDRATelescope.h"

class KVLayer;
class KVNucleus;
class KVChIo;
class KVDetectorEvent;
class KVINDRAReconEvent;

//old BaseIndra type definitions
enum EBaseIndra_type {
   ChIo_GG = 1,
   ChIo_PG,                     //=2
   ChIo_T,                      //=3
   Si_GG,                       //=4
   Si_PG,                       //=5
   Si_T,                        //=6
   CsI_R,                       //=7
   CsI_L,                       //=8
   CsI_T,                       //=9
   Si75_GG,                     //=10
   Si75_PG,                     //=11
   Si75_T,                      //=12
   SiLi_GG,                     //=13
   SiLi_PG,                     //=14
   SiLi_T                       //=15
};
enum EBaseIndra_typePhos {
   Phos_R = 1,
   Phos_L,                      //=2
   Phos_T,                      //=3
};

/**
\class KVINDRA
\brief INDRA multidetector array geometry
\ingroup INDRAGeometry
*/

class KVINDRA: public KVASMultiDetArray {

public:
   static Char_t SignalTypes[16][3];    //Use this static array to translate EBaseIndra_type signal type to a string giving the signal type


private:
   UChar_t fTrigger;           //multiplicity trigger used for acquisition

protected:
   KVHashList* fChIo;              //->List Of ChIo of INDRA
   KVHashList* fSi;                 //->List of Si detectors of INDRA
   KVHashList* fCsI;                //->List of CsI detectors of INDRA
   KVHashList* fPhoswich;           //->List of NE102/NE115 detectors of INDRA

   Bool_t fPHDSet;//set to kTRUE if pulse height defect parameters are set

   KVINDRATriggerInfo* fSelecteur;//infos from DAQ trigger (le Selecteur)

   TEnv fStrucInfos;//! file containing structure of array

   KVNameValueList fEbyedatParamDetMap;//! maps EBYEDAT parameter names to detectors

   Bool_t fEbyedatData;//! set to true when VME/VXI acquisition system is used
   Bool_t fMesytecData;//! set to true when Mesytec acquisition system is used

   virtual void MakeListOfDetectors();
   virtual void BuildGeometry();
   virtual void SetGroupsAndIDTelescopes();
   void FillListsOfDetectorsByType();
   //void SetGGtoPGConversionFactors();
   //void LinkToCodeurs();
   void BuildLayer(const Char_t* name);
   KVRing* BuildRing(Int_t number, const Char_t* prefix);
   KVINDRATelescope* BuildTelescope(const Char_t* prefix, Int_t mod);
   void FillTrajectoryIDTelescopeLists();
   Int_t GetIDTelescopes(KVDetector*, KVDetector*, TCollection*);
   void SetNamesOfIDTelescopes() const;

   void PerformClosedROOTGeometryOperations();
#ifdef WITH_BUILTIN_GRU
   virtual Bool_t handle_raw_data_event_ebyedat(KVGANILDataReader&);
#endif
#ifdef WITH_MFM
   Bool_t handle_raw_data_event_mfmframe_ebyedat(const MFMEbyedatFrame&);
#ifdef WITH_MESYTEC
   virtual Bool_t handle_raw_data_event_mfmframe_mesytec_mdpp(const MFMMesytecMDPPFrame&);
#endif
#endif
   void handle_ebyedat_raw_data_parameter(const char* param_name, uint16_t val);
   void copy_fired_parameters_to_recon_param_list();

public:
   KVINDRA();
   virtual ~ KVINDRA();

   virtual void Build(Int_t run = -1);
   virtual Bool_t ArePHDSet() const
   {
      return fPHDSet;
   }
   virtual void PHDSet(Bool_t yes = kTRUE)
   {
      fPHDSet = yes;
   }

   KVLayer* GetChIoLayer();
   inline KVHashList* GetListOfChIo() const
   {
      return fChIo;
   };
   inline KVHashList* GetListOfSi() const
   {
      return fSi;
   };
   inline KVHashList* GetListOfCsI() const
   {
      return fCsI;
   };
   inline KVHashList* GetListOfPhoswich() const
   {
      return fPhoswich;
   };

   virtual KVChIo* GetChIoOf(const Char_t* detname);
   virtual void cd(Option_t* option = "");
   virtual KVINDRADetector* GetDetectorByType(UInt_t cou, UInt_t mod,
         UInt_t type) const;

   void SetTrigger(UChar_t trig);
   UChar_t GetTrigger() const
   {
      return fTrigger;
   }

   void SetPinLasersForCsI();

   void InitialiseRawDataReading(KVRawDataReader*);
   virtual void GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* fired_dets = 0);

   KVINDRATriggerInfo* GetTriggerInfo()
   {
      return fSelecteur;
   };

   void CreateROOTGeometry();

   virtual void SetROOTGeometry(Bool_t on = kTRUE);
   void SetMinimumOKMultiplicity(KVEvent*) const;
   KVGroupReconstructor* GetReconstructorForGroup(const KVGroup*) const;
   void SetReconParametersInEvent(KVReconstructedEvent*) const;
   void SetRawDataFromReconEvent(KVNameValueList&);

   ClassDef(KVINDRA, 6)        //class describing the materials and detectors etc. to build an INDRA multidetector array
};

//................  global variable
R__EXTERN KVINDRA* gIndra;

//................ inline functions
inline void KVINDRA::cd(Option_t*)
{
   gIndra = this;
}

#endif
