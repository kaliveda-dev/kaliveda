//Created by KVClassFactory on Wed Apr  3 15:57:14 2013
//Author: John Frankland,,,

#ifndef __KVBATCHSYSTEMGUI_H
#define __KVBATCHSYSTEMGUI_H
#include "RQ_OBJECT.h"
#include <KVBase.h>
#include <TGFrame.h>
#include <TTimer.h>
#include <TGButton.h>
#include "KVListView.h"

/**
   \class KVBatchSystemGUI
  \brief GUI for batch system jobs
  \ingroup GUI
Graphical interface for monitoring, altering, and deleting batch jobs.
To run:
~~~~
kaliveda[0]  new KVBatchSystemGUI
~~~~
It will automatically connect to the default batch system and display
all of the user's queued or running jobs along with information
on resource requests and consumption.

The buttons at the top of the GUI can be used to (manually) refresh
the informations (see Automatic refresh, below), alter the properties
of selected (queued) job(s), or delete selected (queued or running) job(s).

#### Automatic refresh
The GUI automatically refreshes every N seconds, as defined by the
following environment variables (.kvrootrc):
~~~~
   KVBatchSystemGUI.AutoUpdate:       yes
   KVBatchSystemGUI.RefreshInterval:    30
~~~~
Change these variables in your .kvrootrc if you want to modify or
disable automatic refreshing.
*/
class KVBatchSystemGUI : public KVBase {
   RQ_OBJECT("KVBatchSystemGUI")
   TGMainFrame* MainFrame;
   TGPictureButton* BrefreshDir;
   TGPictureButton* BremDir;
   TGPictureButton* BalterJobs;
   KVListView* fLVJobs;
   TList* selected_jobs;
   TTimer* fTimer;
   KVList* jobs;
   static Bool_t fOpen;//kTRUE if GUI is already open

public:
   KVBatchSystemGUI();
   virtual ~KVBatchSystemGUI();
   void DoClose();
   void CloseWindow();

   static Bool_t IsOpen()
   {
      return fOpen;
   }

   void Refresh();
   void KillJobs();
   void AlterJobs();

   ClassDef(KVBatchSystemGUI, 1) //GUI for batch system jobs
};

#endif
