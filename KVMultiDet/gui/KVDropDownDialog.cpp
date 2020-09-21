/*
$Id: KVDropDownDialog.cpp,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
$Author: franklan $
*/

#include "KVDropDownDialog.h"
#include "TGFrame.h"
#include "KVString.h"
#include "TGComboBox.h"
#include "TGWindow.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TTimer.h"

ClassImp(KVDropDownDialog)


KVDropDownDialog::KVDropDownDialog(const TGWindow* main,
                                   const Char_t* question, const Char_t* choice_list,
                                   const Char_t* default_choice, TString* chosen,  Bool_t* ok)
{
   // Create the dialog box asking a "question".
   // Possible answers are in "choice list" (whitespace-separated list).
   // Default choice (will be visible in combo box when dialog opens) is "default".
   // The Bool_t variable will be set to kTRUE if "OK" is pressed (kFALSE otherwise)
   // Optional argument 'tooltip' will be displayed when mouse held over text entry widget

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
   fMain->AddFrame(l,
                   new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5,
                                     0));
   // Add drop down list
   fDropDown = new TGComboBox(fMain);
   // fill with choices
   KVString choix(choice_list);
   choix.Begin(" ");
   int entry_id = 0;
   int default_id = -1;
   KVString this_choice;
   while (! choix.End()) {
      this_choice = choix.Next(kTRUE);
      if (this_choice == default_choice) default_id = entry_id;
      fDropDown->AddEntry(this_choice.Data(), entry_id++);
   }
   fDropDown->Resize(260, 22);
   fMain->AddFrame(fDropDown,
                   new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5,
                                     5));
   // display default choice
   fDropDown->Select(default_id);

   fMain->SetWindowName("KVDropDownDialog");
   fMain->SetIconName("KVDropDownDialog");

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
   fOKBut->Connect("Clicked()", "KVDropDownDialog", this, "ReadAnswer()");
   fOKBut->Connect("Clicked()", "KVDropDownDialog", this, "DoClose()");

   fCancelBut = new TGTextButton(hf, "&Cancel", 3);
   hf->AddFrame(fCancelBut, l1);
   height = fCancelBut->GetDefaultHeight();
   width = TMath::Max(width, fCancelBut->GetDefaultWidth());
   ++nb;
   fCancelBut->Connect("Clicked()", "KVDropDownDialog", this, "DoClose()");


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

KVDropDownDialog::~KVDropDownDialog()
{
   //Delete all widgets

   if (fMain) {
      delete fMain;
      fMain = 0;
   }
}

//______________________________________________________________________________

void KVDropDownDialog::DoClose()
{
   TTimer::SingleShot(150, "KVDropDownDialog", this, "CloseWindow()");
}

//______________________________________________________________________________

void KVDropDownDialog::CloseWindow()
{
   delete this;
}

//______________________________________________________________________________
void KVDropDownDialog::ReadAnswer()
{
   //Read text from drop down list and store in "answer" TString.
   //Set "ok" Bool_t flag to kTRUE
   *fAnswer = fDropDown->GetSelectedEntry()->GetTitle();
   *fOK = kTRUE;
}
