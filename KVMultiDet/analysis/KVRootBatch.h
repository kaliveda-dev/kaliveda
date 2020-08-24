/*
$Id: KVRootBatch.h,v 1.5 2007/11/20 16:45:22 franklan Exp $
$Revision: 1.5 $
$Date: 2007/11/20 16:45:22 $
*/

//Created by KVClassFactory on Thu Apr 13 13:08:06 2006
//Author: John Frankland

#ifndef __KVRootBatch_H
#define __KVRootBatch_H

#include <KVBatchSystem.h>

/**
  \class KVRootBatch
  \brief Run job in separate xterm window
  \ingroup Infrastructure
 */

class KVRootBatch: public KVBatchSystem {
public:

   KVRootBatch(const Char_t* name);
   virtual ~ KVRootBatch();

   virtual void PrintJobs(Option_t* opt = "");

   ClassDef(KVRootBatch, 1)  //Run KaliVeda analysis task in an Xterm window
};

#endif
