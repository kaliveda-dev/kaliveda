/*
$Id: KVRootBatch.cpp,v 1.7 2007/11/20 16:45:22 franklan Exp $
$Revision: 1.7 $
$Date: 2007/11/20 16:45:22 $
*/

//Created by KVClassFactory on Thu Apr 13 13:08:06 2006
//Author: John Frankland

#include "KVRootBatch.h"
#include "TSystem.h"

ClassImp(KVRootBatch)

KVRootBatch::KVRootBatch(const Char_t* name)
   : KVBatchSystem(name)
{
   //Default constructor
}

KVRootBatch::~KVRootBatch()
{
   //Destructor
}

void KVRootBatch::PrintJobs(Option_t*)
{
   //Print list of owner's jobs.
   //opt="" (default) : print all jobs
   KVString cmd("ps -ax | grep root | grep KaliVedaAnalysis");
   gSystem->Exec(cmd.Data());
}

