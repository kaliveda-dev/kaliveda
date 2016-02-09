//Created by KVClassFactory on Wed Apr 28 12:29:38 2010
//Author: John Frankland,,,

#ifndef __KVDATATRANSFERXRD_H
#define __KVDATATRANSFERXRD_H

#include "KVDataTransfer.h"

class KVDataTransferXRD : public KVDataTransfer {

public:
   KVDataTransferXRD();
   virtual ~KVDataTransferXRD();

   virtual void ExecuteCommand();
   virtual void WriteTransferScript() {};

   ClassDef(KVDataTransferXRD, 1) //Transfer data from remote repository using xrootd
};

#endif
