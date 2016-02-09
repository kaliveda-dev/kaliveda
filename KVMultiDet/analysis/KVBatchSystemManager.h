/*
$Id: KVBatchSystemManager.h,v 1.2 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.2 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Mon May  7 17:29:05 2007
//Author: franklan

#ifndef __KVBATCHSYSTEMMANAGER_H
#define __KVBATCHSYSTEMMANAGER_H

#include "TObject.h"
#include "KVList.h"
#include "KVBatchSystem.h"

class KVBatchSystemManager {
   KVList fBatchSystems;//list of available batch systems
   void Init();
   KVBatchSystem* fDefault;//default batch system

public:

   KVBatchSystemManager();
   virtual ~KVBatchSystemManager();

   KVBatchSystem* GetBatchSystem(const Char_t* name);
   KVBatchSystem* GetBatchSystem(Int_t index);
   KVBatchSystem* GetDefaultBatchSystem() const
   {
      return fDefault;
   };
   void Print(Option_t* opt = "") const;

   ClassDef(KVBatchSystemManager, 1) //Manages methods of processing non-interactive data analysis tasks
};

//................  global variable
R__EXTERN KVBatchSystemManager* gBatchSystemManager;

#endif
