//Created by KVClassFactory on Thu May 16 11:38:27 2013
//Author: gruyer,,,

#include "KVTextDialog.h"
#include "TGTextEntry.h"
#include "TGFrame.h"
#include "KVString.h"
#include "TGComboBox.h"
#include "TGWindow.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TTimer.h"


ClassImp(KVTextDialog)

KVTextDialog::KVTextDialog(const TGWindow* main, const Char_t* question, const Char_t* default_value, TString* chosen, Bool_t* ok, const char* unit)
{
   fMain = new TGTransientFrame(gClient->GetDefaultRoot(), main, 200, 100);
   fMain->CenterOnParent();
   fMain->Connect("CloseWindow()", "KVDropDownDialog", this, "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   fAnswer = chosen;
   fOK = ok;
   *fOK = kFALSE;               //initialise flag

   // Add a label
   TGLabel* l = new TGLabel(fMain, question);
   fMain->AddFrame(l, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 0));
   TGHorizontalFrame* hf1 = new TGHorizontalFrame(fMain);

   //Add text Entry
   fTextEntry = new TGTextEntry(hf1, default_value);
   fTextEntry->Resize(70, 22);
   fTextEntry->Connect("ReturnPressed()", "KVTextDialog", this, "ReadAnswer()");
   hf1->AddFrame(fTextEntry, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

   l = new TGLabel(hf1, unit);
   hf1->AddFrame(l, new TGLayoutHints(kLHintsTop | kLHintsRight, 5, 5, 5, 0));

   fMain->AddFrame(hf1, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));

   fMain->SetWindowName("KVTextDialog");
   fMain->SetIconName("KVTextDialog");

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
   fOKBut->Connect("Clicked()", "KVTextDialog", this, "ReadAnswer()");
   fOKBut->Connect("Clicked()", "KVTextDialog", this, "DoClose()");
   fTextEntry->Connect("ReturnPressed()", "TGTextButton", fOKBut, "Clicked()");

   fCancelBut = new TGTextButton(hf, "&Cancel", 3);
   hf->AddFrame(fCancelBut, l1);
   height = fCancelBut->GetDefaultHeight();
   width = TMath::Max(width, fCancelBut->GetDefaultWidth());
   ++nb;
   fCancelBut->Connect("Clicked()", "KVTextDialog", this, "DoClose()");

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

//________________________________________________________________
KVTextDialog::~KVTextDialog()
{
   // Destructor
   if (fMain) {
      delete fMain;
      fMain = 0;
   }
}

//________________________________________________________________
void KVTextDialog::DoClose()
{
   TTimer::SingleShot(150, "KVTextDialog", this, "CloseWindow()");

}

//______________________________________________________________________________

void KVTextDialog::CloseWindow()
{
   delete this;
}

//______________________________________________________________________________
void KVTextDialog::ReadAnswer()
{
   //Read text from drop down list and store in "answer" TString.
   //Set "ok" Bool_t flag to kTRUE
   *fAnswer = fTextEntry->GetText();
   *fOK = kTRUE;
}

