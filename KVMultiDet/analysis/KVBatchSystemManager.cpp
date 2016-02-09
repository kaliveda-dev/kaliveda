/*
$Id: KVBatchSystemManager.cpp,v 1.4 2007/11/20 16:45:22 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/20 16:45:22 $
*/

//Created by KVClassFactory on Mon May  7 17:29:05 2007
//Author: franklan

#include "KVBatchSystemManager.h"
#include "KVString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TEnv.h"

using namespace std;

ClassImp(KVBatchSystemManager)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVBatchSystemManager</h2>
<h4>Handles list of all available batch systems for processing non-interactive data analysis tasks</h4>
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
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVBatchSystemManager* gBatchSystemManager = 0;

KVBatchSystemManager::KVBatchSystemManager()
{
   //Default constructor

   if (gBatchSystemManager) return;

   Init();
   gBatchSystemManager = this;
}

KVBatchSystemManager::~KVBatchSystemManager()
{
   //Destructor
   gBatchSystemManager = 0;
   gBatchSystem = 0;
}

void KVBatchSystemManager::Init()
{
   //Set up list of available batch systems

   gBatchSystem = 0;

   KVString list = gEnv->GetValue("BatchSystem", "");

   fBatchSystems.Clear();

   TObjArray* systems = list.Tokenize(" ");
   TIter next(systems);
   TObjString* batch_sys;
   while ((batch_sys = (TObjString*)next())) {

      KVBatchSystem* bs = KVBatchSystem::GetBatchSystem(batch_sys->GetString().Data());
      fBatchSystems.Add(bs);
   }

   delete systems;

   //set default
   fDefault = (KVBatchSystem*)fBatchSystems.FindObjectByName(gEnv->GetValue("Default.BatchSystem", ""));
}

KVBatchSystem* KVBatchSystemManager::GetBatchSystem(const Char_t* name)
{
   //Get batch system by name
   return (KVBatchSystem*)fBatchSystems.FindObjectByName(name);
}

KVBatchSystem* KVBatchSystemManager::GetBatchSystem(Int_t index)
{
   //Get batch system by number in the list printed by Print()
   return (KVBatchSystem*)fBatchSystems.At(index - 1);
}

void KVBatchSystemManager::Print(Option_t* opt) const
{
   //Print list of available batch systems
   // Default is to show numbered list with title of each batch system
   //if opt="all", print detailed information on each batch system
   if (!strcmp(opt, "all")) {
      for (register int i = 1; i <= fBatchSystems.GetSize(); i++) {
         fBatchSystems.At(i - 1)->Print("all");
      }
   } else {
      for (register int i = 1; i <= fBatchSystems.GetSize(); i++) {
         cout << "\t" << i << ". " << ((KVBatchSystem*)fBatchSystems.At(i - 1))->GetTitle() << endl;
      }
   }
}
