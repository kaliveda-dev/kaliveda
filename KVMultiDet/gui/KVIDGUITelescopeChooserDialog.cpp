/*
$Id: KVIDGUITelescopeChooserDialog.cpp,v 1.2 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Nov 17 14:41:53 2008
//Author: franklan

#include "KVIDGUITelescopeChooserDialog.h"
#include "TTimer.h"

using namespace std;

ClassImp(KVIDGUITelescopeChooserDialog)

KVIDGUITelescopeChooserDialog::KVIDGUITelescopeChooserDialog(KVMultiDetArray* MDA, TList* selection, Bool_t* cancel_pressed,
      const TGWindow* p, const TGWindow* main, Bool_t disable_type_select,
      UInt_t w, UInt_t h)
{
   // Create and display dialog for choosing ID telescopes
   // cancel_pressed = kTRUE if cancel button is pressed

   fSelection = selection;
   fMultiDet = MDA;

   if (!MDA) return;

   fCancelPress = cancel_pressed;
   *fCancelPress = kTRUE;

   int dx = 358;
   int xx = 400;

   // main frame
   fMainFrame1428 = new TGTransientFrame(p, main, w, h);
   fMainFrame1428->SetLayoutBroken(kTRUE);
   fMainFrame1428->Connect("CloseWindow()", "KVIDGUITelescopeChooserDialog", this,
                           "DoClose()");
   fMainFrame1428->DontCallClose();      // to avoid double deletions.
   // use hierarchical cleaning
   fMainFrame1428->SetCleanup(kDeepCleanup);

   // composite frame
   TGCompositeFrame* fMainFrame979 = new TGCompositeFrame(fMainFrame1428, xx, 273, kVerticalFrame);
   fMainFrame979->SetLayoutBroken(kTRUE);

   // combo box with list of ID telescope types
   fComboBox994 = new TGComboBox(fMainFrame979, -1, kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);

   //get list of ID telescope types from multidetarray
   unique_ptr<KVUniqueNameList> id_types(MDA->GetIDTelescopeTypes());
   // fill combo with types in list id_types
   TIter next_type(id_types.get());
   TObjString* type_string;
   int entry_id = 0;
   while ((type_string = (TObjString*)next_type())) {
      fComboBox994->AddEntry(type_string->GetString().Data(), entry_id++);
   }

   fComboBox994->Resize(dx, 22);
   fComboBox994->Select(-1);
   fMainFrame979->AddFrame(fComboBox994, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fComboBox994->MoveResize(24, 40, dx, 22);
   if (disable_type_select) fComboBox994->SetEnabled(kFALSE);

   TGLabel* fLabel1012 = new TGLabel(fMainFrame979, "Select ID telescope(s):");
   fLabel1012->SetTextJustify(36);
   fLabel1012->SetMargins(0, 0, 0, 0);
   fLabel1012->SetWrapLength(-1);
   fMainFrame979->AddFrame(fLabel1012, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1012->MoveResize(24, 72, dx, 18);

   TGLabel* fLabel1013 = new TGLabel(fMainFrame979, "Select ID type:");
   fLabel1013->SetTextJustify(36);
   fLabel1013->SetMargins(0, 0, 0, 0);
   fLabel1013->SetWrapLength(-1);
   fMainFrame979->AddFrame(fLabel1013, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1013->MoveResize(24, 8, dx, 18);

   // list box
   fListBox980 = new TGListBox(fMainFrame979);
   fListBox980->Resize(dx, 132);
   // fill with dummy entries (for padding)
   for (int i = 0; i < 10; i++) fListBox980->AddEntry("    ", i);
   fMainFrame979->AddFrame(fListBox980, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fListBox980->MapSubwindows();
   fListBox980->Layout();
   fListBox980->MoveResize(24, 104, dx, 132);
   fListBox980->SetMultipleSelections();

   TGTextButton* fTextButton1052 = new TGTextButton(fMainFrame979, "Cancel");
   fTextButton1052->SetTextJustify(36);
   fTextButton1052->SetMargins(0, 0, 0, 0);
   fTextButton1052->SetWrapLength(-1);
   fTextButton1052->Resize(72, 24);
   fMainFrame979->AddFrame(fTextButton1052, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fTextButton1052->MoveResize(140, 245, 72, 24);

   TGTextButton* fTextButton1057 = new TGTextButton(fMainFrame979, "OK");
   fTextButton1057->SetTextJustify(36);
   fTextButton1057->SetMargins(0, 0, 0, 0);
   fTextButton1057->SetWrapLength(-1);
   fTextButton1057->Resize(72, 24);
   fMainFrame979->AddFrame(fTextButton1057, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fTextButton1057->MoveResize(220, 245, 72, 24);

   fMainFrame1428->AddFrame(fMainFrame979, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   fMainFrame979->MoveResize(0, 0, xx, 273);

   fMainFrame1428->SetMWMHints(kMWMDecorAll,
                               kMWMFuncAll,
                               kMWMInputModeless);
   fMainFrame1428->MapSubwindows();

   fMainFrame1428->Resize(fMainFrame1428->GetDefaultSize());
   fMainFrame1428->CenterOnParent();
   fMainFrame1428->MapWindow();
   fMainFrame1428->Resize(xx, 291);

   /****             connect signals & slots                   ***/
   // choice of ID type
   fComboBox994->Connect("Selected(const char*)", "KVIDGUITelescopeChooserDialog", this,
                         "FillTelescopeListWithType(const char*)");
   // cancel button -> close window, leave list empty
   fTextButton1052->Connect("Clicked()", "KVIDGUITelescopeChooserDialog", this, "DoClose()");
   // OK button -> fill list with selected telescopes, then close window
   fTextButton1057->Connect("Clicked()", "KVIDGUITelescopeChooserDialog", this, "GetSelection()");

   // if list already contains selected telescopes, we change state of dialog box
   // to reflect previous selection
   if (fSelection->GetEntries()) HighlightSelectedTelescopes();

   //nothing happens until window closes
   gClient->WaitFor(fMainFrame1428);
}

KVIDGUITelescopeChooserDialog::~KVIDGUITelescopeChooserDialog()
{
   // Destructor
   // Delete all widgets

   if (fMainFrame1428) {
      delete fMainFrame1428;
      fMainFrame1428 = 0;
   }
}

void KVIDGUITelescopeChooserDialog::DoClose()
{
   TTimer::SingleShot(150, "KVIDGUITelescopeChooserDialog", this,
                      "CloseWindow()");
}

void KVIDGUITelescopeChooserDialog::CloseWindow()
{
   delete this;
}

void KVIDGUITelescopeChooserDialog::FillTelescopeListWithType(const char* type)
{
   // fill TGListBox with names of all ID telescopes of given type

   fListBox980->RemoveAll();
   fListBox980->Layout();
   unique_ptr<KVSeqCollection> telescopes(fMultiDet->GetIDTelescopesWithType(type));
   TIter next_tel(telescopes.get());
   KVIDTelescope* idt;
   int i = 0;
   while ((idt = (KVIDTelescope*)next_tel())) {
      fListBox980->AddEntry(idt->GetName(), i++);
   }
   fListBox980->SortByName();
   fListBox980->MapSubwindows();
   fListBox980->Layout();
}

void KVIDGUITelescopeChooserDialog::GetSelection()
{
   // fill fSelection list with telescopes selected by user
   // set cancel_pressed to kFALSE

   fSelection->Clear();
   TList lbentries;
   fListBox980->GetSelectedEntries(&lbentries);
   TIter next_lbe(&lbentries);
   TGLBEntry* lbe;
   while ((lbe = (TGLBEntry*)next_lbe())) {
      fSelection->Add(fMultiDet->GetIDTelescope(lbe->GetTitle()));
   }
   *fCancelPress = kFALSE;
   DoClose();
}
void KVIDGUITelescopeChooserDialog::HighlightSelectedTelescopes()
{
   // Modify widgets to reflect the ID telescopes selected in the list fSelection

   // Get 'type' of first telescope
   KVIDTelescope* id = (KVIDTelescope*)fSelection->At(0);
   if (!id) return;
   // look for 'type' in list of types
   TGLBEntry* lbe = fComboBox994->FindEntry(id->GetLabel());
   if (!lbe) {
      cout << "<KVIDGUITelescopeChooserDialog::HighlightSelectedTelescopes()> : Type "
           << id->GetLabel() << " not found in combo box" << endl;
      return;
   }
   // make 'type' be selected entry
   fComboBox994->Select(lbe->EntryId());
   FillTelescopeListWithType(id->GetLabel());
   // now 'select' telescopes
   TIter next_tel(fSelection);
   int first_tel = -1;
   while ((id = (KVIDTelescope*)next_tel())) {
      lbe = fListBox980->FindEntry(id->GetName());
      if (lbe) {
         if (first_tel < 0) first_tel = lbe->EntryId();
         fListBox980->Select(lbe->EntryId());
      }
   }
   // scroll down to first selected telescope
   fListBox980->SetTopEntry(first_tel);
}
