//Created by KVClassFactory on Mon Feb 25 09:01:43 2013
//Author: Guilain ADEMARD

#ifndef __KVIVRECONDATAANALYSER_H
#define __KVIVRECONDATAANALYSER_H

#include "KVINDRAReconDataAnalyser.h"

class KVIVReconDataAnalyser : public KVINDRAReconDataAnalyser {

public:
   KVIVReconDataAnalyser();
   virtual ~KVIVReconDataAnalyser();

   void preAnalysis();
   void preInitRun();

   ClassDef(KVIVReconDataAnalyser, 1) //For analysing reconstructed INDRA+VAMOS data
};

#endif
