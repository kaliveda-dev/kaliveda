//Created by KVClassFactory on Fri Feb 17 09:32:40 2017
//Author: John Frankland,,,

#include "KVNameValueListGUI.h"

#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include "TTimer.h"
#include "TMath.h"

ClassImp(KVNameValueListGUI)

TObject* KVNameValueListGUI::AddAString(Int_t i, TGHorizontalFrame* hf)
{
   TGTextEntry* text = new TGTextEntry(hf, theList->GetStringValue(i));
   max_width = TMath::Max(max_width, text->GetDefaultWidth() + 10);
   text->Resize(text->GetDefaultWidth(), text->GetDefaultHeight());
   hf->AddFrame(text, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 2, 1, 1));
   return text;
}

TObject* KVNameValueListGUI::AddABool(Int_t i, TGHorizontalFrame* hf)
{
   TGCheckButton* but = new TGCheckButton(hf);
   but->Resize(but->GetDefaultWidth(), 10);
   hf->AddFrame(but, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 2, 1, 1));
   but->SetState(theList->GetBoolValue(i) ? kButtonDown : kButtonUp, kFALSE);
   return but;
}

TObject* KVNameValueListGUI::AddADouble(Int_t i, TGHorizontalFrame* hf)
{
   TGNumberEntry* num = new TGNumberEntry(hf, theList->GetDoubleValue(i), 10, -1,
                                          TGNumberEntry::kNESReal,
                                          TGNumberEntry::kNEAAnyNumber,
                                          TGNumberEntry::kNELNoLimits);
   max_width = TMath::Max(max_width, num->GetDefaultWidth() + 10);
   num->Resize(num->GetDefaultWidth(), num->GetDefaultHeight());
   hf->AddFrame(num, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 2, 1, 1));
   return num;
}

TObject* KVNameValueListGUI::AddAInt(Int_t i, TGHorizontalFrame* hf)
{
   TGNumberEntry* num = new TGNumberEntry(hf, theList->GetDoubleValue(i), 10, -1,
                                          TGNumberEntry::kNESInteger,
                                          TGNumberEntry::kNEAAnyNumber,
                                          TGNumberEntry::kNELNoLimits);
   max_width = TMath::Max(max_width, num->GetDefaultWidth() + 10);
   num->Resize(num->GetDefaultWidth(), num->GetDefaultHeight());
   hf->AddFrame(num, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 2, 1, 1));
   return num;
}

void KVNameValueListGUI::ReadData()
{
   for (int i = 0; i < theList->GetNpar(); ++i) {
      KVNamedParameter* par = theList->GetParameter(i);
      if (par->IsDouble()) {
         par->Set(dynamic_cast<TGNumberEntry*>(fData[i])->GetNumber());
      }
      else if (par->IsInt()) {
         par->Set((Int_t)dynamic_cast<TGNumberEntry*>(fData[i])->GetNumber());
      }
      else if (par->IsString()) {
         par->Set(dynamic_cast<TGTextEntry*>(fData[i])->GetText());
      }
      else if (par->IsBool()) {
         par->Set(dynamic_cast<TGCheckButton*>(fData[i])->IsDown());
      }
   }
}

void KVNameValueListGUI::RestoreData()
{
   // return all values to original state
   fOriginal.Copy(*theList);
   *fCancel = kTRUE;
}

KVNameValueListGUI::KVNameValueListGUI(const TGWindow* main, KVNameValueList* params, Bool_t* cancel_pressed, Bool_t wait_for_main)
   : fData(params->GetNpar())
{
   // Constructor
   fMain = new TGTransientFrame(gClient->GetDefaultRoot(), main, 1, 1, kVerticalFrame);
   fMain->CenterOnParent();
   fMain->Connect("CloseWindow()", "KVNameValueListGUI", this, "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   fCancel = cancel_pressed;
   *fCancel = kFALSE;

   // SET UP GUI
   theList = params;
   theList->Copy(fOriginal); // copy original values in case "Cancel" is pressed
   // for each name-value pair, add a horizontal frame with a label (name)
   // and a text entry box (value)
   Int_t nentries = theList->GetEntries();
   max_width = 1;
   for (Int_t i = 0; i < nentries; ++i) {
      TGHorizontalFrame* hf = new TGHorizontalFrame(fMain);
      TGLabel* lab = new TGLabel(hf, theList->GetNameAt(i));
      max_width = TMath::Max(max_width, lab->GetDefaultWidth() + 10);
      lab->Resize(lab->GetDefaultWidth(), lab->GetDefaultHeight());
      hf->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 2, 3, 1, 1));

      if (theList->GetParameter(i)->IsInt()) {
         fData[i] = AddAInt(i, hf);
      }
      else if (theList->GetParameter(i)->IsDouble()) {
         fData[i] = AddADouble(i, hf);
      }
      else if (theList->GetParameter(i)->IsBool()) {
         fData[i] = AddABool(i, hf);
      }
      else {
         fData[i] = AddAString(i, hf);
      }

      fMain->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 1, 1, 1, 1));
   }

   //--- create the OK, Apply and Cancel buttons

   UInt_t width = max_width * 2, height = 0;

   TGHorizontalFrame* hf = new TGHorizontalFrame(fMain);

   fOKBut = new TGTextButton(hf, "&OK", 1);
   hf->AddFrame(fOKBut, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2, 3, 0, 0));
   height = fOKBut->GetDefaultHeight();
   fOKBut->SetToolTipText("Save changes and close");
   fOKBut->Resize(max_width, height);
   fOKBut->Connect("Clicked()", "KVNameValueListGUI", this, "ReadData()");
   fOKBut->Connect("Clicked()", "KVNameValueListGUI", this, "DoClose()");

   fCancelBut = new TGTextButton(hf, "&Cancel", 3);
   hf->AddFrame(fCancelBut, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 3, 2, 0, 0));
   fCancelBut->Resize(max_width, height);
   fCancelBut->SetToolTipText("Discard changes and close");
   fCancelBut->Connect("Clicked()", "KVNameValueListGUI", this, "RestoreData()");
   fCancelBut->Connect("Clicked()", "KVNameValueListGUI", this, "DoClose()");

   fMain->AddFrame(hf, new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX, 0, 0, 10, 5));

   // map all widgets and calculate size of dialog
   fMain->MapSubwindows();

   height = fMain->GetDefaultHeight();
   fMain->Resize(width, height);
   fMain->SetWMSize(width, height);
   fMain->SetWMSizeHints(width, height, width, height, 0, 0);
   fMain->SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize |
                      kMWMDecorMinimize | kMWMDecorMenu,
                      kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize |
                      kMWMFuncMinimize, kMWMInputModeless);

   // position relative to the parent's window
   fMain->CenterOnParent();
   fMain->SetWindowName(theList->GetTitle());

   fMain->MapWindow();
   if (wait_for_main) gClient->WaitFor(fMain);
}

//____________________________________________________________________________//

KVNameValueListGUI::~KVNameValueListGUI()
{
   // Destructor
   if (fMain) {
      delete fMain;
      fMain = 0;
   }
}

//____________________________________________________________________________//

void KVNameValueListGUI::DoClose()
{
   TTimer::SingleShot(150, "KVNameValueListGUI", this, "CloseWindow()");
}

//______________________________________________________________________________

void KVNameValueListGUI::CloseWindow()
{
   delete this;
}
