/*
$Id: KVFAZIAReconDataAnalyser.h,v 1.2 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.2 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Wed Apr  5 23:50:04 2006
//Author: John Frankland

#ifndef __KVFAZIAReconDataAnalyser_H
#define __KVFAZIAReconDataAnalyser_H

#include "KVDataSetAnalyser.h"
#include "TSelector.h"
#include <KVDataPatchList.h>
#include <KVEventSelector.h>
class TChain;

/**
\class KVFAZIAReconDataAnalyser
\brief Pilot analysis of reconstructed FAZIA data
\ingroup FAZIAnal
*/

class KVFAZIAReconDataAnalyser: public KVDataSetAnalyser {

protected:

   TTree* theTree;//tree to be analysed
   Int_t fRunNumber;             //run number of current file
   Bool_t fLinkRawData;
   KVEventSelector* fEventSelector;

public:

   void RegisterUserClass(TObject* o)
   {
      fEventSelector = dynamic_cast<KVEventSelector*>(o);
   }

   KVFAZIAReconDataAnalyser();
   virtual ~ KVFAZIAReconDataAnalyser();

   virtual void preInitAnalysis();
   virtual void preInitRun();
   virtual void preAnalysis();

   void SubmitTask();
   void ProcessRun();
   Int_t GetCurrentRunNumber() const
   {
      return fRunNumber;
   }


   ClassDef(KVFAZIAReconDataAnalyser, 1) //For analysing reconstructed FAZIA data
};

#endif
