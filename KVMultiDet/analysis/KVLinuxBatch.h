/*
$Id: KVLinuxBatch.h,v 1.3 2007/11/20 16:45:22 franklan Exp $
$Revision: 1.3 $
$Date: 2007/11/20 16:45:22 $
*/

//Created by KVClassFactory on Thu Apr 13 13:08:06 2006
//Author: John Frankland

#ifndef __KVLinuxBatch_H
#define __KVLinuxBatch_H

#include <KVRootBatch.h>

/**
  \class KVLinuxBatch
  \brief Run KaliVeda analysis task in background on Linux system with output stored in logfile
  \ingroup Infrastructure
 */
class KVLinuxBatch: public KVRootBatch {
public:

   KVLinuxBatch(const Char_t* name);
   virtual ~ KVLinuxBatch();

   ClassDef(KVLinuxBatch, 1)  //Run KaliVeda analysis task in background on Linux system with output stored in logfile
};

#endif
