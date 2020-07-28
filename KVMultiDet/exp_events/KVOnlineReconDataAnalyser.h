//Created by KVClassFactory on Mon Apr  1 10:16:46 2019
//Author: eindra

#ifndef __KVONLINERECONDATAANALYSER_H
#define __KVONLINERECONDATAANALYSER_H

#include "KVReconDataAnalyser.h"


/**
\class KVOnlineReconDataAnalyser
\brief Online analysis of reconstructed data
\ingroup Analysis
*/

class KVOnlineReconDataAnalyser : public KVReconDataAnalyser {
   mutable Bool_t fUpdate;
public:
   KVOnlineReconDataAnalyser();
   virtual ~KVOnlineReconDataAnalyser();

   Bool_t CheckTaskVariables()
   {
      // for now, don't check anything
      // to be added: check user analysis class compiles, is valid, etc.
      return kTRUE;
   }
   void RegisterUserClass(TObject*) {}
   void WriteBatchEnvFile(const Char_t*, Bool_t) {}
   void SubmitTask();
   void preInitAnalysis();
   void preInitRun() {}
   void preAnalysis();

   Bool_t CheckStatusUpdateInterval(Int_t) const;

   ClassDef(KVOnlineReconDataAnalyser, 1) //Online analysis of reconstructed data
};

#endif
