//Created by KVClassFactory on Fri Jan 23 14:17:23 2015
//Author: ,,,

#ifndef __KVFAZIARAWDATARECONSTRUCTOR_H
#define __KVFAZIARAWDATARECONSTRUCTOR_H

#include "KVFAZIAReader.h"
#include "KVReconstructedEvent.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"

/**
\class KVFAZIARawDataReconstructor
\brief Reconstruction of raw FAZIA data
\ingroup FAZIARecon
*/

class KVFAZIARawDataReconstructor : public KVFAZIAReader {
protected:

   TFile* file;
   TTree* tree;

   KVReconstructedEvent* recev;

   Int_t nb_recon;//number of reconstructed events
   TString taskname;
   TString datatype;

public:
   KVFAZIARawDataReconstructor();
   virtual ~KVFAZIARawDataReconstructor();

   virtual void InitAnalysis() {}
   virtual void InitRun();
   virtual Bool_t Analysis();
   virtual void EndRun();
   virtual void EndAnalysis() {};

   virtual void ExtraProcessing();

   ClassDef(KVFAZIARawDataReconstructor, 1) //Handle reconstruction of FAZIA events
};

#endif
