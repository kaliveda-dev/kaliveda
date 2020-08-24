/*
$Id: KVRemoteDataSetManager.h,v 1.3 2007/09/20 11:30:17 franklan Exp $
$Revision: 1.3 $
$Date: 2007/09/20 11:30:17 $
*/

//Created by KVClassFactory on Thu Sep  7 15:16:29 2006
//Author: John Frankland

#ifndef __KVREMOTEDATASETMANAGER_H
#define __KVREMOTEDATASETMANAGER_H

#include <KVDataSetManager.h>
#include "Riostream.h"

/**
  \class KVRemoteDataSetManager
  \brief Manage datasets stored in a remote data repository
  \ingroup DM
 */
class KVRemoteDataSetManager: public KVDataSetManager {

protected:
   Bool_t OpenAvailableDatasetsFile();
   TString fCurl;               //full path to executable used to copy files


public:

   KVRemoteDataSetManager();
   virtual ~ KVRemoteDataSetManager();

   virtual void CheckAvailability();

   ClassDef(KVRemoteDataSetManager, 2) //Handles data sets in remote data repository
};

#endif
