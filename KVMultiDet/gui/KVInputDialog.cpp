/*
$Id: KVInputDialog.cpp,v 1.6 2007/12/12 11:17:54 franklan Exp $
$Revision: 1.6 $
$Date: 2007/12/12 11:17:54 $
$Author: franklan $
*/

#include "KVInputDialog.h"
#include "TGFrame.h"
#include "TString.h"
#include "KVTextEntry.h"
#include "TGWindow.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TTimer.h"

ClassImp(KVInputDialog)
KVInputDialog::KVInputDialog(const TGWindow* main,
                             const Char_t* question, TString* answer,
                             Bool_t* ok, const Char_t* tooltip)
{
   //Create the dialog box asking a "question" and receiving the "answer"
   //If you want a default value to appear, put it in 'answer' before opening dialogue
   //The Bool_t variable will be set to kTRUE if "OK" is pressed (kFALSE otherwise)
   //Optional argument 'tooltip' will be displayed when mouse held over text entry widget

   fMain = new TGTransientFrame(gClient->GetRoot(), main, 600, 150);
   fMain->Connect("CloseWindow()", "KVInputDialog", this, "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   fAnswer = answer;
   fOK = ok;
   *fOK = kFALSE;               //initialise flag
   // Add a label and text input field.

   TGLabel* l = new TGLabel(fMain, question);
   fTextEntry = new KVTextEntry(fMain, answer->Data());
   if (strcmp(tooltip, ""))
      fTextEntry->SetToolTipText(tooltip);

   fTextEntry->Resize(l->GetDefaultWidth(), fTextEntry->GetDefaultHeight());
   fMain->AddFrame(l,
                   new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5,
                                     0));
   fMain->AddFrame(fTextEntry,
                   new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5,
                                     5));

   fMain->SetWindowName("KVInputDialog");
   fMain->SetIconName("KVInputDialog");

   //--- create the OK, Apply and Cancel buttons

   UInt_t nb = 0, width = 0, height = 0;

   TGHorizontalFrame* hf =
      new TGHorizontalFrame(fMain, 60, 20, kFixedWidth);
   TGLayoutHints* l1 =
      new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 5, 5, 0, 0);

   fOKBut = new TGTextButton(hf, "&OK", 1);
   hf->AddFrame(fOKBut, l1);
   height = fOKBut->GetDefaultHeight();
   width = TMath::Max(width, fOKBut->GetDefaultWidth());
   ++nb;
   fOKBut->Connect("Clicked()", "KVInputDialog", this, "ReadAnswer()");
   fOKBut->Connect("Clicked()", "KVInputDialog", this, "DoClose()");

   fCancelBut = new TGTextButton(hf, "&Cancel", 3);
   hf->AddFrame(fCancelBut, l1);
   height = fCancelBut->GetDefaultHeight();
   width = TMath::Max(width, fCancelBut->GetDefaultWidth());
   ++nb;
   fCancelBut->Connect("Clicked()", "KVInputDialog", this, "DoClose()");

   // place buttons at the bottom
   l1 = new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);

   fMain->AddFrame(hf, l1);

   // keep the buttons centered and with the same width
   hf->Resize((width + 20) * nb, height);

   // map all widgets and calculate size of dialog
   fMain->MapSubwindows();

   width = fMain->GetDefaultWidth();
   height = fMain->GetDefaultHeight();

   fMain->Resize(width, height);

   // position relative to the parent's window
   fMain->CenterOnParent();

   // make the message box non-resizable
   fMain->SetWMSize(width, height);
   fMain->SetWMSizeHints(width, height, width, height, 0, 0);

   fMain->SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize |
                      kMWMDecorMinimize | kMWMDecorMenu,
                      kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize |
                      kMWMFuncMinimize, kMWMInputModeless);

   fMain->CenterOnParent();
   fMain->MapWindow();
   gClient->WaitFor(fMain);
}

//______________________________________________________________________________

KVInputDialog::~KVInputDialog()
{
   //Delete all widgets

   if (fMain) {
      delete fMain;
      fMain = 0;
   }
}

//______________________________________________________________________________

void KVInputDialog::DoClose()
{
   TTimer::SingleShot(150, "KVInputDialog", this, "CloseWindow()");
}

//______________________________________________________________________________

void KVInputDialog::CloseWindow()
{
   delete this;
}

//______________________________________________________________________________
void KVInputDialog::ReadAnswer()
{
   //Read text from text entry widget and store in "answer" TString.
   //Set "ok" Bool_t flag to kTRUE
   *fAnswer = fTextEntry->GetText();
   *fOK = kTRUE;
}
