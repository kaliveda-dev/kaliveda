/*
$Id: KVINDRARawDataReconstructor.h,v 1.4 2008/10/17 10:58:07 franklan Exp $
$Revision: 1.4 $
$Date: 2008/10/17 10:58:07 $
*/

//Created by KVClassFactory on Fri May  4 17:24:08 2007
//Author: franklan

#ifndef __KVINDRARAWDATARECONSTRUCTOR_H
#define __KVINDRARAWDATARECONSTRUCTOR_H

#include "KVINDRARawDataAnalyser.h"
#include "KVINDRAReconEvent.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"

class KVINDRARawDataReconstructor : public KVINDRARawDataAnalyser {
protected:

   TFile* file;
   TTree* tree;
   TTree* genetree;
   TTree* rawtree;
   KVINDRAReconEvent* recev;
   Int_t nb_recon;//number of reconstructed INDRA events
   TString taskname;
   TString datatype;

public:

   KVINDRARawDataReconstructor();
   virtual ~KVINDRARawDataReconstructor();

   virtual void InitAnalysis();
   virtual void InitRun();
   virtual Bool_t Analysis();
   virtual void EndRun();
   virtual void EndAnalysis() {};

   virtual void ExtraProcessing() {};

   ClassDef(KVINDRARawDataReconstructor, 2) //Reconstruction of events from raw data acquisition files
};

#endif
