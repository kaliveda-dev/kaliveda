//Created by KVClassFactory on Thu Feb 25 14:59:59 2016
//Author: bonnet,,,

#ifndef __KVEXPSETUP_H
#define __KVEXPSETUP_H

#include "KVMultiDetArray.h"
class KVDBRun;

/**
  \class KVExpSetUp
  \ingroup Geometry
  \brief Describe an experimental set-up coupling two or more different detector arrays

KVExpSetUp combines two or more existing KVMultiDetArray objects to describe the
geometry, detectors, etc. of an experiment coupling several different arrays.
Each KVMultiDetArray can have its own
dedicated calibration database (KVExpDB and KVUpdater class)
used to store and set calibration parameters.

\note All detector & structure names in individual arrays must be unique! The group numbering has to be modified (in Build()) to make them unique.
 */
class KVExpSetUp : public KVMultiDetArray {

protected:
   KVList fMDAList;  //list of multidetarrays
   KVString lmultidetarrayclasses;

   void init();

#ifdef WITH_MFM
   Bool_t handle_raw_data_event_mfmframe(const MFMCommonFrame&);
#endif
   void copy_fired_parameters_to_recon_param_list();

   virtual void SetExpectedDetectorSignalNames();

public:

   KVExpSetUp();
   virtual ~KVExpSetUp();
   virtual void Build(Int_t run = -1);
   void Clear(Option_t* opt = "");

   void FillDetectorList(KVReconstructedNucleus* rnuc, KVHashList* DetList, const KVString& DetNames);
   virtual KVMultiDetArray* GetArray(const Char_t* name) const
   {
      // Return pointer to array in set up with given name
      return (KVMultiDetArray*)fMDAList.FindObject(name);
   }
   virtual void AcceptParticleForAnalysis(KVReconstructedNucleus*) const;

   void GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* fired_params = nullptr);
   KVGroupReconstructor* GetReconstructorForGroup(const KVGroup*) const;
   void SetRawDataFromReconEvent(KVNameValueList&);
   void SetReconParametersInEvent(KVReconstructedEvent*) const;

   void GetArrayMultiplicities(KVReconstructedEvent*, KVNameValueList&, Option_t* = "");

   void MakeCalibrationTables(KVExpDB*);
   void SetCalibratorParameters(KVDBRun*, const TString& = "");

   void InitialiseRawDataReading(KVRawDataReader*);
   Bool_t HandleRawDataEvent(KVRawDataReader*);

   ClassDef(KVExpSetUp, 1) //Describe an experimental set-up made of several KVMultiDetArray objects
};

#endif
