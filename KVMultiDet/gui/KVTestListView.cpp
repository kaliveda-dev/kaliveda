//Created by KVClassFactory on Wed Apr  3 15:57:14 2013
//Author: John Frankland,,,

#include "KVTestListView.h"
#include <KVDatime.h>
#include <TEnv.h>

ClassImp(KVTestListView)

KVTestListView::KVTestListView(TClass* obclass, Int_t ncols, TString* columns)
{
   // Default constructor

   MainFrame = new TGMainFrame(gClient->GetRoot(), 10, 10, kMainFrame | kVerticalFrame);
   MainFrame->SetName("Test GUI");
   fLV = new KVListView(obclass, MainFrame, 900, 400);
   fLV->SetDataColumns(ncols);
   for (int i = 0; i < ncols; i++) fLV->SetDataColumn(i, columns[i]);
   fLV->ActivateSortButtons();

   MainFrame->AddFrame(fLV, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 10, 10));
   MainFrame->SetMWMHints(kMWMDecorAll,
                          kMWMFuncAll,
                          kMWMInputModeless);
   MainFrame->MapSubwindows();

   MainFrame->Resize(MainFrame->GetDefaultSize());
   MainFrame->MapWindow();

   MainFrame->Connect("CloseWindow()", "KVTestListView", this, "DoClose()");
   MainFrame->DontCallClose();      // to avoid double deletions.
}

KVTestListView::~KVTestListView()
{
   // Destructor
   delete MainFrame;
}

void KVTestListView::DoClose()
{
   TTimer::SingleShot(150, "KVTestListView", this, "CloseWindow()");
}

void KVTestListView::CloseWindow()
{
   delete this;
}

