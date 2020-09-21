//Created by KVClassFactory on Wed Dec 12 10:11:10 2012
//Author: dgruyer

#include "KVZAFinderDialog.h"
#include "KVIDGridEditor.h"

Int_t   KVZAFinderDialog::fZmin  = -1;
Int_t   KVZAFinderDialog::fZmax  = -1;
TString KVZAFinderDialog::fAList = "1,4,7,9,11,12,14,16,19,21,23,25,27,29,31,34,37,40,41";

Int_t   KVZAFinderDialog::fBinsByZ      = 60;
Int_t   KVZAFinderDialog::fStepsByLine  = -1;

ClassImp(KVZAFinderDialog)

//________________________________________________________________
KVZAFinderDialog::KVZAFinderDialog(KVIDGraph* g, TH2* data_histo)
{
   fGrid  = g;
   fHisto = data_histo;
   if ((fZmin < 0) && fGrid) fZmin = ((KVIDentifier*)fGrid->GetIdentifiers()->First())->GetPID();
   if ((fZmax < 0) && fGrid) fZmax = ((KVIDentifier*)fGrid->GetIdentifiers()->Last())->GetPID();

   //Dialog box for testing identification grid
   fMain = new TGTransientFrame(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), 10, 10);
   fMain->Connect("CloseWindow()", "KVSpIdGUI", this, "DoClose()");
   fMain->DontCallClose();
   fMain->SetCleanup(kDeepCleanup);

   /******* frame for initialization **************/
   fInitFrame = new TGGroupFrame(fMain, "Initialization");

   fZpFrame = new TGHorizontalFrame(fInitFrame);
   fZminLabel = new TGLabel(fZpFrame, "Zmin ");
   fZminEntry = new TGNumberEntry(fZpFrame, fZmin, 6, 0,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);
   fZpFrame->AddFrame(fZminLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fZpFrame->AddFrame(fZminEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   fZmaxLabel = new TGLabel(fZpFrame, "  Zmax ");
   fZmaxEntry = new TGNumberEntry(fZpFrame, fZmax, 6, 0,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);
   fZpFrame->AddFrame(fZmaxLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fZpFrame->AddFrame(fZmaxEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   fZbinLabel = new TGLabel(fZpFrame, "  Bins ");
   fZbinEntry = new TGNumberEntry(fZpFrame, fBinsByZ, 6, 0,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);

   fZpFrame->AddFrame(fZbinLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fZpFrame->AddFrame(fZbinEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fInitFrame->AddFrame(fZpFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

   fApFrame = new TGHorizontalFrame(fInitFrame);
   fALabel = new TGLabel(fApFrame, "A  ");
   fAEntry = new TGTextEntry(fAList.Data(), fApFrame);
   fApFrame->AddFrame(fALabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fApFrame->AddFrame(fAEntry, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));
   fInitFrame->AddFrame(fApFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 5));

//       /********** linearization bar **********/
//  fLinearFrame = new TGGroupFrame(fMain, "Linearization");
//  fLinearBar = new TGHProgressBar(fLinearFrame, TGProgressBar::kFancy, 300);
//  fLinearBar->SetFillType(TGProgressBar::kBlockFill);
//  fLinearBar->ShowPosition();
//  fLinearBar->SetBarColor("green");
//  fLinearFrame->AddFrame(fLinearBar,new TGLayoutHints(kLHintsExpandX | kLHintsTop,5,5,5,5));

   /********** progress bar **********/
   fProgressFrame = new TGGroupFrame(fMain, "Progress");
   fProgressBar = new TGHProgressBar(fProgressFrame, TGProgressBar::kFancy, 300);
   fProgressBar->SetFillType(TGProgressBar::kBlockFill);
   fProgressBar->ShowPosition();
   fProgressBar->SetBarColor("green");
   fProgressFrame->AddFrame(fProgressBar, new TGLayoutHints(kLHintsExpandX | kLHintsTop, 5, 5, 5, 5));

   /************buttons***************/
   fButtonsFrame = new TGHorizontalFrame(fMain, 100, 50);
   fTestButton = new TGTextButton(fButtonsFrame, "&Process");
   fTestButton->Connect("Clicked()", "KVZAFinderDialog", this,  "ProcessIdentification()");
   fCloseButton = new TGTextButton(fButtonsFrame, "&Close");
   fCloseButton->Connect("Clicked()", "KVZAFinderDialog", this, "DoClose()");

   fButtonsFrame->AddFrame(fTestButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));
   fButtonsFrame->AddFrame(fCloseButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));


   fMain->AddFrame(fInitFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2, 2, 10, 2));
//  fMain->AddFrame(fLinearFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
   fMain->AddFrame(fProgressFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
   fMain->AddFrame(fButtonsFrame, new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2, 2, 2, 2));

   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Masses Identification");
   fMain->MapWindow();


}

//________________________________________________________________
KVZAFinderDialog::KVZAFinderDialog(const KVZAFinderDialog&)
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

}

//________________________________________________________________
KVZAFinderDialog::~KVZAFinderDialog()
{
   // Destructor
}

//________________________________________________________________
void KVZAFinderDialog::DoClose()
{
   TTimer::SingleShot(150, "KVZAFinderDialog", this, "CloseWindow()");
}

//________________________________________________________________
void KVZAFinderDialog::CloseWindow()
{
   delete fMain;
   delete this;
}

//________________________________________________________________
void KVZAFinderDialog::ProcessIdentification()
{
   fZmin  = fZminEntry->GetIntNumber();
   fZmax  = fZmaxEntry->GetIntNumber();
   fAList = fAEntry->GetText();

   Int_t zmin = ((KVIDentifier*)fGrid->GetIdentifiers()->First())->GetPID();
   if (fZmin < zmin) fZmin = zmin;
   Int_t zmax = ((KVIDentifier*)fGrid->GetIdentifiers()->Last())->GetPID();
   if (fZmax > zmax) fZmax = zmax;

   KVZALineFinder toto((KVIDZAGrid*)fGrid, fHisto);

//   fLinearBar->SetRange(0, ((TH2F*)fHisto)->GetSum());
//   fLinearBar->SetRange(0, fHisto->GetNbinsX()*fHisto->GetNbinsY());
//   fLinearBar->Reset();
//  toto.Connect("IncrementLinear(Float_t)","TGHProgressBar",
//                          fLinearBar, "SetPosition(Float_t)");
   fProgressBar->SetRange(0, zmax - zmin);
   fProgressBar->Reset();
   toto.Connect("Increment(Float_t)", "TGHProgressBar",
                fProgressBar, "SetPosition(Float_t)");

   toto.SetAList(fAList.Data());
   toto.SetNbinsByZ(fBinsByZ);

   fTestButton->SetEnabled(kFALSE);
   fCloseButton->SetEnabled(kFALSE);

   toto.ProcessIdentification(fZmin, fZmax);
   fTestButton->SetEnabled(kTRUE);
   fCloseButton->SetEnabled(kTRUE);

   toto.Disconnect("IncrementLinear(Float_t)",
                   fLinearBar, "SetPosition(Float_t)");
   toto.Disconnect("Increment(Float_t)",
                   fProgressBar, "SetPosition(Float_t)");


   if (!gIDGridEditor) {
      new KVIDGridEditor;
      gIDGridEditor->StartViewer();
   }

   gIDGridEditor->SetHisto(toto.GetHisto());
   gIDGridEditor->SetGrid(toto.GetGrid(), kFALSE);

   DoClose();
}
