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

/**
\class KVBatchSystemManager
\brief Handles list of all available batch systems for processing non-interactive data analysis tasks
\ingroup Infrastructure

<p>The different "batch" systems available are described in the $KVROOT/KVFiles/.kvrootrc
or the user's $HOME/.kvrootrc, as in the following examples:</p>
<pre>
# Batch systems
BatchSystem:     Xterm
Xterm.BatchSystem.Title:    Execute task in an X-terminal window
+BatchSystem:     Linux
Linux.BatchSystem.Title:    Execute task in background with output stored in log file
#+BatchSystem:     BQS
#BQS.BatchSystem.Title:   Use CCIN2P3 BQS batch system
#BQS.BatchSystem.DefaultJobTime:   4000
#BQS.BatchSystem.DefaultJobMemory:   256MB
#BQS.BatchSystem.DefaultJobDisk:   100MB
</pre>
<p>For each named batch system, there must be a corresponding plugin which
defines the actual class to use:</p>
<pre>
#Plugins for batch systems
Plugin.KVBatchSystem:    Xterm    KVRootBatch     KVMultiDet    "KVRootBatch()"
+Plugin.KVBatchSystem:    Linux    KVLinuxBatch     KVMultiDet    "KVLinuxBatch()"
+Plugin.KVBatchSystem:    BQS    KV_CCIN2P3_BQS     KVMultiDet    "KV_CCIN2P3_BQS()"
</pre>
<p>The "default" batch system is defined as follows:</p>
<pre>
#Default batch system
Default.BatchSystem:   Xterm
</pre>
<p>It can be accessed via the global pointer to the batch system manager:</p>
<pre>
KVBatchSystem* default = gBatchSystemManager->GetDefaultBatchSystem();
</pre>
*/

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
