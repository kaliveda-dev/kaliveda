/*
$Id: KVDataTransferBBFTP.h,v 1.3 2007/01/04 16:38:50 franklan Exp $
$Revision: 1.3 $
$Date: 2007/01/04 16:38:50 $
*/

//Created by KVClassFactory on Mon Sep 18 15:28:27 2006
//Author: franklan

#ifndef __KVDATATRANSFERBBFTP_H
#define __KVDATATRANSFERBBFTP_H

#include <KVDataTransfer.h>

class KVDataTransferBBFTP: public KVDataTransfer {
   KVDataRepository* fServer;   //data repository with BBFTP server

public:

   KVDataTransferBBFTP();
   virtual ~ KVDataTransferBBFTP();

protected:

   virtual void init();
   virtual void ExecuteCommand();
   virtual void WriteTransferScript();

   ClassDef(KVDataTransferBBFTP, 0)    //File transfer between repositories using bbftp
};

#endif
