//Created by KVClassFactory on Mon Jul 16 15:20:21 2018
//Author: eindra

#ifndef __KVRAWDATARECONSTRUCTOR_H
#define __KVRAWDATARECONSTRUCTOR_H

#include "KVEventReconstructor.h"
#include "KVRawDataAnalyser.h"

/**
   \class KVRawDataReconstructor
 \brief Manage task of reconstruction of physical events from raw data
 \ingroup Reconstruction
  */

class KVRawDataReconstructor : public KVRawDataAnalyser {

   unique_ptr<KVEventReconstructor> fEvRecon;
   KVReconstructedEvent* fRecev;
   TFile* fRecFile;
   TTree* fRecTree;

public:
   KVRawDataReconstructor();
   virtual ~KVRawDataReconstructor();

   virtual void InitAnalysis();
   virtual void InitRun();
   virtual Bool_t Analysis();
   virtual void EndRun();
   virtual void EndAnalysis();

   ClassDef(KVRawDataReconstructor, 1) //Reconstruct physical events from raw data
};

#endif
