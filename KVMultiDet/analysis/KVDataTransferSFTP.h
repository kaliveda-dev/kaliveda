/*
$Id: KVDataTransferSFTP.h,v 1.2 2006/10/19 14:32:43 franklan Exp $
$Revision: 1.2 $
$Date: 2006/10/19 14:32:43 $
*/

//Created by KVClassFactory on Mon Sep 18 15:28:18 2006
//Author: franklan

#ifndef __KVDATATRANSFERSFTP_H
#define __KVDATATRANSFERSFTP_H

#include <KVDataTransfer.h>

class KVDataTransferSFTP: public KVDataTransfer {
public:

   KVDataTransferSFTP();
   virtual ~ KVDataTransferSFTP();

   ClassDef(KVDataTransferSFTP, 0)      //File transfer between repositories using sftp
};

#endif
