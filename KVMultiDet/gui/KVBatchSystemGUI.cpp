//Created by KVClassFactory on Wed Apr  3 15:57:14 2013
//Author: John Frankland,,,

#include "KVBatchSystemGUI.h"
#include "KVInputDialog.h"
#include <KVBatchJob.h>
#include <KVDatime.h>
#include <TEnv.h>
#include <KVBatchSystemManager.h>
#include <KVGEBatchJob.h>

ClassImp(KVBatchSystemGUI)


Bool_t KVBatchSystemGUI::fOpen = kFALSE;

KVBatchSystemGUI::KVBatchSystemGUI()
{
   // Default constructor
   fOpen = kTRUE;
   jobs = 0;

   MainFrame = new TGMainFrame(gClient->GetRoot(), 10, 10, kMainFrame | kVerticalFrame);
   MainFrame->SetName("BatchSystem GUI");
   TGHorizontalFrame* hf = new TGHorizontalFrame(MainFrame, 10, 10, kHorizontalFrame);
   BrefreshDir = new TGPictureButton(hf, gClient->GetPicture("refresh2.xpm"));
   hf->AddFrame(BrefreshDir, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   BrefreshDir->Resize(40, 40);
   BrefreshDir->SetToolTipText("Update");
   BrefreshDir->Connect("Clicked()", "KVBatchSystemGUI", this, "Refresh()");
   BalterJobs = new TGPictureButton(hf, gClient->GetPicture("bld_edit_s.png"));
   hf->AddFrame(BalterJobs, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   BalterJobs->Resize(40, 40);
   BalterJobs->SetToolTipText("Alter job(s) resources");
   BalterJobs->Connect("Clicked()", "KVBatchSystemGUI", this, "AlterJobs()");
   BremDir = new TGPictureButton(hf, gClient->GetPicture("mb_stop_s.xpm"));
   hf->AddFrame(BremDir, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   BremDir->Resize(40, 40);
   BremDir->SetToolTipText("Kill job(s)");
   BremDir->Connect("Clicked()", "KVBatchSystemGUI", this, "KillJobs()");
   MainFrame->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
   fLVJobs = new KVListView(KVBatchJob::Class(), MainFrame, 900, 400);
   fLVJobs->SetDataColumns(11);
   fLVJobs->SetDataColumn(0, "JobID");
   fLVJobs->SetDataColumn(1, "Name");
   fLVJobs->SetDataColumn(2, "Status");
   fLVJobs->SetDataColumn(3, "Submitted");
   fLVJobs->GetDataColumn(3)->SetIsDateTime(KVDatime::kSQL);
   fLVJobs->SetDataColumn(4, "Complete [%]", "GetPercentageComplete");
   fLVJobs->SetDataColumn(5, "CPUusage");
   fLVJobs->SetDataColumn(6, "CPUmax");
   fLVJobs->SetDataColumn(7, "MemUsed");
   fLVJobs->SetDataColumn(8, "MemMax");
   fLVJobs->SetDataColumn(9, "DiskUsed");
   fLVJobs->SetDataColumn(10, "DiskMax");
   fLVJobs->ActivateSortButtons();

   if (!gBatchSystemManager) new KVBatchSystemManager;
   if (!gBatchSystem) gBatchSystemManager->GetDefaultBatchSystem()->cd();

   selected_jobs = 0;

   Refresh();

   fTimer = 0;
   // automatic update every N second
   if (gEnv->GetValue("KVBatchSystemGUI.AutoUpdate", kFALSE)) {
      fTimer = new TTimer;
      fTimer->Connect("Timeout()", "KVBatchSystemGUI", this, "Refresh()");
      fTimer->Start(1000 * (gEnv->GetValue("KVBatchSystemGUI.RefreshInterval", 30)));
   }

   MainFrame->AddFrame(fLVJobs, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 10, 10));
   MainFrame->SetMWMHints(kMWMDecorAll,
                          kMWMFuncAll,
                          kMWMInputModeless);
   MainFrame->MapSubwindows();

   MainFrame->Resize(MainFrame->GetDefaultSize());
   MainFrame->MapWindow();

   MainFrame->Connect("CloseWindow()", "KVBatchSystemGUI", this, "DoClose()");
   MainFrame->DontCallClose();      // to avoid double deletions.
}

KVBatchSystemGUI::~KVBatchSystemGUI()
{
   // Destructor
   fOpen = kFALSE;
   SafeDelete(selected_jobs);
   if (fTimer)delete fTimer;
   delete MainFrame;
   SafeDelete(jobs);
}

void KVBatchSystemGUI::Refresh()
{
   KVDatime now;
   Info("Refresh", "Updating... [%s]", now.AsSQLString());
   if (!gBatchSystemManager) new KVBatchSystemManager;
   if (!gBatchSystem) gBatchSystemManager->GetDefaultBatchSystem()->cd();
   KVList* newjobs = gBatchSystem->GetListOfJobs();
   fLVJobs->Display(newjobs);
   SafeDelete(jobs);
   jobs = newjobs;
}

void KVBatchSystemGUI::KillJobs()
{
   SafeDelete(selected_jobs);
   selected_jobs = fLVJobs->GetSelectedObjects();
   if (!selected_jobs->GetEntries()) return;
   TIter next(selected_jobs);
   KVBatchJob* job;
   while ((job = (KVBatchJob*)next())) job->DeleteJob();
   Refresh();
}

void KVBatchSystemGUI::AlterJobs()
{
   SafeDelete(selected_jobs);
   selected_jobs = fLVJobs->GetSelectedObjects();
   if (!selected_jobs->GetEntries()) return;

   //gBatchSystem->AlterJobs(MainFrame, selected_jobs);
   KVGEBatchJob* j1 = (KVGEBatchJob*)selected_jobs->First();
   TString resources = j1->GetResources();
   Bool_t ok = kFALSE;
   new KVInputDialog(MainFrame, "Modify the currently-set resources for theses jobs ?", &resources, &ok,
                     "Change the resources you want to modify; leave the other ones as they are");
   if (ok) {
      TIter next(selected_jobs);
      KVGEBatchJob* job;
      while ((job = (KVGEBatchJob*)next())) job->AlterResources(resources);
   }

   Refresh();
}

void KVBatchSystemGUI::DoClose()
{
   TTimer::SingleShot(150, "KVBatchSystemGUI", this, "CloseWindow()");
}

void KVBatchSystemGUI::CloseWindow()
{
   delete this;
}

