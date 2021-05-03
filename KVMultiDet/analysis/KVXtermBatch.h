#ifndef __KVXTERMBATCH_H
#define __KVXTERMBATCH_H

#include "KVBatchSystem.h"

/**
 \class KVXtermBatch
 \brief Run analysis in multiple xterm windows
 \ingroup Infrastructure

 This "batch" system will run jobs as separate processes in xterm windows with logging of
 the output from each job in separate files. The aim is to optimize use of multicore
 PCs when analysing data which is not in a ROOT file/TTree, i.e. for which PROOFLite
 cannot be used.

 When N runs are to be analysed on an M-core machine, this batch system will run
 M jobs concurrently with the N runs shared as evenly as possible among the jobs.

 \author eindra
 \date Fri Apr 30 11:05:39 2021
*/

class KVXtermBatch : public KVBatchSystem {
public:
   KVXtermBatch(const Char_t* name)
      : KVBatchSystem(name)
   {}
   virtual ~KVXtermBatch() {}

   Bool_t MultiJobsMode() const
   {
      return kTRUE;
   }
   void Run();

   ClassDef(KVXtermBatch, 1) //Run analysis in multiple xterm windows
};

#endif
