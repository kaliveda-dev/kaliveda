#include "KVMergeGridsDialog.h"
#include "Riostream.h"
#include "KVIDGridManager.h"
#include "KVIDGrid.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>

Int_t KVMergeGridsDialog::G1IDmin = 1;
Int_t KVMergeGridsDialog::G1IDmax = 99;
Int_t KVMergeGridsDialog::G2IDmin = 1;
Int_t KVMergeGridsDialog::G2IDmax = 99;

using namespace std;

ClassImp(KVMergeGridsDialog)
////////////////////////////////////////////////////////////////
// KVMergeGridsDialog
////////////////////////////////////////////////////////////////
KVMergeGridsDialog::KVMergeGridsDialog(KVIDGrid* g1, KVIDGrid* g2,
                                       const TGWindow* p,
                                       const TGWindow* main, UInt_t w,
                                       UInt_t h)
{
   //Dialog box for merging grids

   fMain = new TGTransientFrame(p, main, w, h);
   fMain->Connect("CloseWindow()", "KVMergeGridsDialog", this,
                  "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.

   fGrid1 = g1;
   fGrid2 = g2;

   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   //group frame
   fGFrame = new TGGroupFrame(fMain, "Select lines to merge");

   fG1NameLabel = new TGLabel(fGFrame, fGrid1->GetName());
   fGFrame->AddFrame(fG1NameLabel,
                     new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));

   /******* frame for G1 idmin idmax **************/
   fG1IDFrame = new TGHorizontalFrame(fGFrame);
   fG1IDminLabel = new TGLabel(fG1IDFrame, "ID min:");
   fG1IDFrame->AddFrame(fG1IDminLabel,
                        new TGLayoutHints(kLHintsNormal, 2, 5, 2, 2));
   fG1IDminEntry =
      new TGNumberEntry(fG1IDFrame, G1IDmin, 3, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fG1IDFrame->AddFrame(fG1IDminEntry,
                        new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   fG1IDmaxLabel = new TGLabel(fG1IDFrame, "ID max:");
   fG1IDFrame->AddFrame(fG1IDmaxLabel,
                        new TGLayoutHints(kLHintsNormal, 10, 5, 2, 2));
   fG1IDmaxEntry =
      new TGNumberEntry(fG1IDFrame, G1IDmax, 3, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fG1IDFrame->AddFrame(fG1IDmaxEntry,
                        new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   fGFrame->AddFrame(fG1IDFrame,
                     new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));

   fG2NameLabel = new TGLabel(fGFrame, fGrid2->GetName());
   fGFrame->AddFrame(fG2NameLabel,
                     new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));

   /******* frame for G2 idmin idmax **************/
   fG2IDFrame = new TGHorizontalFrame(fGFrame);
   fG2IDminLabel = new TGLabel(fG2IDFrame, "ID min:");
   fG2IDFrame->AddFrame(fG2IDminLabel,
                        new TGLayoutHints(kLHintsNormal, 2, 5, 2, 2));
   fG2IDminEntry =
      new TGNumberEntry(fG2IDFrame, G2IDmin, 3, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fG2IDFrame->AddFrame(fG2IDminEntry,
                        new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   fG2IDmaxLabel = new TGLabel(fG2IDFrame, "ID max:");
   fG2IDFrame->AddFrame(fG2IDmaxLabel,
                        new TGLayoutHints(kLHintsNormal, 10, 5, 2, 2));
   fG2IDmaxEntry =
      new TGNumberEntry(fG2IDFrame, G2IDmax, 3, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fG2IDFrame->AddFrame(fG2IDmaxEntry,
                        new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));

   fGFrame->AddFrame(fG2IDFrame,
                     new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   fMain->AddFrame(fGFrame, new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));

   /************buttons***************/
   fButtonsFrame = new TGHorizontalFrame(fMain, 100, 50);
   fOKButton = new TGTextButton(fButtonsFrame, "&Merge");
   fOKButton->Connect("Clicked()", "KVMergeGridsDialog", this,
                      "MergeGrids()");
   fCancelButton = new TGTextButton(fButtonsFrame, "&Cancel");
   fCancelButton->Connect("Clicked()", "KVMergeGridsDialog", this,
                          "DoClose()");

   fButtonsFrame->AddFrame(fOKButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));
   fButtonsFrame->AddFrame(fCancelButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));

   fMain->AddFrame(fButtonsFrame,
                   new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));

   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Merge ID grids");
   fMain->MapWindow();

}

void KVMergeGridsDialog::DoClose()
{
   TTimer::SingleShot(150, "KVMergeGridsDialog", this, "CloseWindow()");
}

void KVMergeGridsDialog::CloseWindow()
{
   delete this;
}

KVMergeGridsDialog::~KVMergeGridsDialog()
{
   //Delete all widgets

   delete fMain;
}

void KVMergeGridsDialog::MergeGrids()
{
   //merge grids together. add new grid to grid manager.
   G1IDmin = fG1IDminEntry->GetIntNumber();
   G1IDmax = fG1IDmaxEntry->GetIntNumber();
   G2IDmin = fG2IDminEntry->GetIntNumber();
   G2IDmax = fG2IDmaxEntry->GetIntNumber();
   cout << "Merging grids : id1_min=" << G1IDmin
        << " id1_max=" << G1IDmax
        << " id2_min=" << G2IDmin << " id2_max=" << G2IDmax << endl;
   KVIDGrid* m =
      (KVIDGrid*)KVIDGrid::AddGraphs(fGrid1, G1IDmin, G1IDmax, fGrid2, G2IDmin,
                                     G2IDmax);
   TString newname;
   newname.Form("Merge of %s and %s", fGrid1->GetName(), fGrid2->GetName());
   m->SetName(newname);
   DoClose();
}
