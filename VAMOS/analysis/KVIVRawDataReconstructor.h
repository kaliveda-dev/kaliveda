/*
$Id: KVIVRawDataReconstructor.h,v 1.3 2009/01/14 15:59:11 franklan Exp $
$Revision: 1.3 $
$Date: 2009/01/14 15:59:11 $
*/

//Created by KVClassFactory on Fri Jun  1 14:46:26 2007
//Author: e_ivamos

#ifndef __KVIVRAWDATARECONSTRUCTOR_H
#define __KVIVRAWDATARECONSTRUCTOR_H

#include "KVINDRARawDataReconstructor.h"
#include "GTGanilData.h"
#include "KVGANILDataReader.h"
#include "KVIVReconEvent.h"

class KVIVRawDataReconstructor : public KVINDRARawDataReconstructor {

protected:

   KVDetectorEvent* fINDRADetEv;  //list of hit group for INDRA event
   KVDetectorEvent* fVAMOSDetEv;  //list of hit group for VAMOS event
   KVIVReconEvent*  fIVevent;
   Int_t            fNbVAMOSrecon;//number of reconstructed VAMOS events

public:

   KVIVRawDataReconstructor();
   virtual ~KVIVRawDataReconstructor();

   virtual void postInitRun()
   {
      KVINDRARawDataReconstructor::postInitRun(); // initialise event counters
      ((KVGANILDataReader*)fRunFile)->GetGanTapeInterface()->SetUserTree(tree);
   };
   virtual void   InitRun();
   virtual void   preAnalysis();
   virtual Bool_t Analysis();
   virtual void   postAnalysis();
   virtual void   EndRun();

   ClassDef(KVIVRawDataReconstructor, 1) //Reconstructs raw data from INDRA-VAMOS experiments
};

#endif
