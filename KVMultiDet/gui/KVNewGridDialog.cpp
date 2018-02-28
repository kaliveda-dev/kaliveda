#include "KVNewGridDialog.h"
#include "Riostream.h"
#include "KVIDGridManager.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>
#include "KVConfig.h"

#ifdef __WITHOUT_TGBUTTON_SETENABLED
#define SetEnabled(flag) \
   SetState( flag ? kButtonUp : kButtonDisabled )
#endif

Int_t KVNewGridDialog::first_run = 1;
Int_t KVNewGridDialog::last_run = 9999;
Bool_t KVNewGridDialog::choose_runs = kFALSE;
Int_t KVNewGridDialog::ring_min = 1;
Int_t KVNewGridDialog::ring_max = 99;
Bool_t KVNewGridDialog::choose_rings = kFALSE;
Int_t KVNewGridDialog::mod_min = 1;
Int_t KVNewGridDialog::mod_max = 99;
Bool_t KVNewGridDialog::choose_mods = kFALSE;
Double_t KVNewGridDialog::scale_x = 1.00;
Double_t KVNewGridDialog::scale_y = 1.00;
Bool_t KVNewGridDialog::choose_scales = kFALSE;
TString KVNewGridDialog::fGridClass("KVIDZAGrid");
Int_t KVNewGridDialog::fClassIndex = 1;
Color_t KVNewGridDialog::fColorOfSelectedLines = kCyan;

void KVNewGridDialog::_set_choose_runs(Bool_t j)
{
   choose_runs = j;
}

Bool_t KVNewGridDialog::_choose_runs()
{
   return choose_runs;
}

void KVNewGridDialog::_set_choose_rings(Bool_t j)
{
   choose_rings = j;
}

Bool_t KVNewGridDialog::_choose_rings()
{
   return choose_rings;
}

void KVNewGridDialog::_set_choose_mods(Bool_t j)
{
   choose_mods = j;
}

Bool_t KVNewGridDialog::_choose_mods()
{
   return choose_mods;
}

void KVNewGridDialog::_set_choose_scales(Bool_t j)
{
   choose_scales = j;
}

Bool_t KVNewGridDialog::_choose_scales()
{
   return choose_scales;
}

Double_t KVNewGridDialog::_scale_x()
{
   return scale_x;
}

Double_t KVNewGridDialog::_scale_y()
{
   return scale_y;
}

void KVNewGridDialog::_set_scale_x(Double_t i)
{
   scale_x = i;
}

void KVNewGridDialog::_set_scale_y(Double_t i)
{
   scale_y = i;
}

using namespace std;

ClassImp(KVNewGridDialog)
//////////////////////////////////////////
// KVNewGridDialog
//Dialog box for creating a new grid/modifying a grid/copying a grid
//////////////////////////////////////////
void KVNewGridDialog::init_interface(const TGWindow* p,
                                     const TGWindow* main, UInt_t w,
                                     UInt_t h, Option_t*)
{
   //initlalisations, creations of main dialog window

   fFirstIDline = 0;
   fLastIDline = -1;
   fFirstOKline = 0;
   fLastOKline = -1;
   fMain = new TGTransientFrame(p, main, w, h);
   fMain->Connect("CloseWindow()", "KVNewGridDialog", this, "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.

   fSelectedLine = 0;
   fLastSelectedIDLine = fLastSelectedOKLine = -1;
   fLastSelectedLineColor = kBlack;
   fLineClassName = "KVIDZALine";
   fLineType = kIDRadBut;
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   fClassList = 0;
}

void KVNewGridDialog::new_grid_class_list()
{
   //Add a drop-down menu with different id grid classes. Used for "New grid" dialog.

   fHframe = new TGHorizontalFrame(fMain, 100, 20);
   fClassLabel = new TGLabel(fHframe, "ID graph type :");

   fHframe->AddFrame(fClassLabel,
                     new TGLayoutHints(kLHintsCenterY, 5, 5, 2, 2));

   fClassList = new TGComboBox(fHframe);
   TString classes[] =
   { "KVIDZGrid", "KVIDZAGrid", "KVIDGChIoSi", "KVIDGCsI" };
   for (int i = 0; i < 4; i++) {
      fClassList->AddEntry(classes[i].Data(), i + 1);
   }
   fClassList->Resize(120, 20);
   fClassList->Select(fClassIndex);

   fClassList->Connect("Selected(const char*)", "KVNewGridDialog", this,
                       "SetGridClass(const char*)");
   fClassList->Connect("Selected(Int_t)", "KVNewGridDialog", this,
                       "SetClassIndex(Int_t)");
   fHframe->AddFrame(fClassList,
                     new TGLayoutHints(kLHintsCenterY, 5, 5, 2, 2));

   fMain->AddFrame(fHframe,
                   new TGLayoutHints(kLHintsCenterX, 2, 2, 10, 5));
}

void KVNewGridDialog::layout_parameter_control_widgets()
{
   //for "edit" and "copy" dialog boxes, we add controls for setting scale factors, detector numbers, etc.

   //set state of interface from selected grid
   if (fSelectedGrid->GetParameters()->GetIntValue("Ring min") > 0) {
      choose_rings = kTRUE;
      ring_min = fSelectedGrid->GetParameters()->GetIntValue("Ring min");
      ring_max = fSelectedGrid->GetParameters()->GetIntValue("Ring max");
   } else
      choose_rings = kFALSE;
   if (fSelectedGrid->GetParameters()->GetIntValue("Mod min") > 0) {
      choose_mods = kTRUE;
      mod_min = fSelectedGrid->GetParameters()->GetIntValue("Mod min");
      mod_max = fSelectedGrid->GetParameters()->GetIntValue("Mod max");
   } else
      choose_mods = kFALSE;
   if (fSelectedGrid->GetParameters()->GetIntValue("First run") > 0) {
      choose_runs = kTRUE;
      first_run = fSelectedGrid->GetParameters()->GetIntValue("First run");
      last_run = fSelectedGrid->GetParameters()->GetIntValue("Last run");
   } else
      choose_runs = kFALSE;
   if (fSelectedGrid->GetXScaleFactor() != 1.0
         || fSelectedGrid->GetYScaleFactor() != 1.0) {
      choose_scales = kTRUE;
      scale_x = fSelectedGrid->GetXScaleFactor();
      scale_y = fSelectedGrid->GetYScaleFactor();
   } else
      choose_scales = kFALSE;
   /**************Select run numbers************/
   fMatrixFrame = new TGCompositeFrame(fMain);
   fMatrixFrame->
   SetLayoutManager(new TGMatrixLayout(fMatrixFrame, 4, 5, 10));

   fSetRuns = new TGCheckButton(fMatrixFrame, "Set runs :", CHK_BUT_RUNS);
   //set checked according to last known state
   fSetRuns->SetDown(choose_runs);
   fSetRuns->Connect("Toggled(Bool_t)", "KVNewGridDialog", this,
                     "SelectRuns(Bool_t)");
   fRunMinLabel = new TGLabel(fMatrixFrame, "first run");
   fRunMaxLabel = new TGLabel(fMatrixFrame, "last run");
   fRunMin =
      new TGNumberEntry(fMatrixFrame, first_run, 5, NE_RUN_MIN,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fRunMax =
      new TGNumberEntry(fMatrixFrame, last_run, 5, NE_RUN_MAX,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);

   fMatrixFrame->AddFrame(fSetRuns);
   fMatrixFrame->AddFrame(fRunMinLabel);
   fMatrixFrame->AddFrame(fRunMin);
   fMatrixFrame->AddFrame(fRunMaxLabel);
   fMatrixFrame->AddFrame(fRunMax);

   /**************Select ring numbers************/
   fSetRings =
      new TGCheckButton(fMatrixFrame, "Set rings :", CHK_BUT_RINGS);
   //set checked according to last known state
   fSetRings->SetDown(choose_rings);
   fSetRings->Connect("Toggled(Bool_t)", "KVNewGridDialog", this,
                      "SelectRings(Bool_t)");
   fRingMinLabel = new TGLabel(fMatrixFrame, "ring min");
   fRingMaxLabel = new TGLabel(fMatrixFrame, "ring max");
   fRingMin =
      new TGNumberEntry(fMatrixFrame, ring_min, 5, NE_RING_MIN,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fRingMax =
      new TGNumberEntry(fMatrixFrame, ring_max, 5, NE_RING_MAX,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);

   fMatrixFrame->AddFrame(fSetRings);
   fMatrixFrame->AddFrame(fRingMinLabel);
   fMatrixFrame->AddFrame(fRingMin);
   fMatrixFrame->AddFrame(fRingMaxLabel);
   fMatrixFrame->AddFrame(fRingMax);

   /**************Select module numbers************/
   fSetMods = new TGCheckButton(fMatrixFrame, "Set mods :", CHK_BUT_MODS);
   //set checked according to last known state
   fSetMods->SetDown(choose_mods);
   fSetMods->Connect("Toggled(Bool_t)", "KVNewGridDialog", this,
                     "SelectMods(Bool_t)");
   fModMinLabel = new TGLabel(fMatrixFrame, "mod min");
   fModMaxLabel = new TGLabel(fMatrixFrame, "mod max");
   fModMin =
      new TGNumberEntry(fMatrixFrame, mod_min, 5, NE_MOD_MIN,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fModMax =
      new TGNumberEntry(fMatrixFrame, mod_max, 5, NE_MOD_MAX,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);

   fMatrixFrame->AddFrame(fSetMods);
   fMatrixFrame->AddFrame(fModMinLabel);
   fMatrixFrame->AddFrame(fModMin);
   fMatrixFrame->AddFrame(fModMaxLabel);
   fMatrixFrame->AddFrame(fModMax);

   /**************Scaling factors************/
   fSetScales =
      new TGCheckButton(fMatrixFrame, "Scaling :", CHK_BUT_SCALES);
   //set checked according to last known state
   fSetScales->SetDown(choose_scales);
   fSetScales->Connect("Toggled(Bool_t)", "KVNewGridDialog", this,
                       "SelectScales(Bool_t)");
   fScaleMinLabel = new TGLabel(fMatrixFrame, "X factor");
   fScaleMaxLabel = new TGLabel(fMatrixFrame, "Y factor");
   fScaleMin =
      new TGNumberEntry(fMatrixFrame, scale_x, 5, NE_SCALE_X,
                        TGNumberFormat::kNESReal,
                        TGNumberFormat::kNEAPositive);
   fScaleMax =
      new TGNumberEntry(fMatrixFrame, scale_y, 5, NE_SCALE_Y,
                        TGNumberFormat::kNESReal,
                        TGNumberFormat::kNEAPositive);

   fMatrixFrame->AddFrame(fSetScales);
   fMatrixFrame->AddFrame(fScaleMinLabel);
   fMatrixFrame->AddFrame(fScaleMin);
   fMatrixFrame->AddFrame(fScaleMaxLabel);
   fMatrixFrame->AddFrame(fScaleMax);

   fMain->AddFrame(fMatrixFrame,
                   new TGLayoutHints(kLHintsExpandX | kLHintsTop, 10, 5,
                                     10, 5));
}

void KVNewGridDialog::layout_ok_apply_cancel_buttons(Option_t*
      dialog_type)
{
   //add "ok", "create", "copy", "cancel" etc. buttons depending on dialog_type

   fApply = 0;
   if (!strcmp(dialog_type, "New")) {
      fHframe2 = new TGHorizontalFrame(fMain, 150, 50);
      fCreate = new TGTextButton(fHframe2, "C&reate");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this,
                       "CreateGrid()");
   } else if (!strcmp(dialog_type, "Edit")) {
      fHframe2 = new TGHorizontalFrame(fMain, 300, 50);
      fCreate = new TGTextButton(fHframe2, "&OK");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this,
                       "ReadRings()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this, "ReadRuns()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this, "ReadMods()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this,
                       "ReadScales()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this,
                       "ModifyGrid()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this, "DoClose()");
      fApply = new TGTextButton(fHframe2, "&Apply");
      fApply->Connect("Clicked()", "KVNewGridDialog", this, "ReadRings()");
      fApply->Connect("Clicked()", "KVNewGridDialog", this, "ReadRuns()");
      fApply->Connect("Clicked()", "KVNewGridDialog", this, "ReadMods()");
      fApply->Connect("Clicked()", "KVNewGridDialog", this,
                      "ReadScales()");
      fApply->Connect("Clicked()", "KVNewGridDialog", this,
                      "ModifyGrid()");
   } else if (!strcmp(dialog_type, "Copy")) {
      fHframe2 = new TGHorizontalFrame(fMain, 150, 50);
      fCreate = new TGTextButton(fHframe2, "C&opy");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this,
                       "ReadRings()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this, "ReadRuns()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this, "ReadMods()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this,
                       "ReadScales()");
      fCreate->Connect("Clicked()", "KVNewGridDialog", this, "CopyGrid()");
   }
   fCancel = new TGTextButton(fHframe2, "&Cancel");
   fCancel->Connect("Clicked()", "KVNewGridDialog", this, "DoClose()");

   fHframe2->AddFrame(fCreate,
                      new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2,
                                        5, 5));
   if (fApply)
      fHframe2->AddFrame(fApply,
                         new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2,
                                           2, 5, 5));
   fHframe2->AddFrame(fCancel,
                      new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2,
                                        5, 5));

   fMain->AddFrame(fHframe2,
                   new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 15,
                                     10));
}

void KVNewGridDialog::layout_grid_editor()
{
   //set up widget used for editing grids in "Edit" dialogue mode
   //this is a horizontal frame  containing:
   //on the left a vertical button group with buttons for adding, deleting lines
   //on the right, two list boxes in a vertical layout, one for OK lines, one for ID lines

   fHFlines = new TGHorizontalFrame(fMain, 10, 10);

   //new line group
   fLineButGrp = new TGGroupFrame(fHFlines, "New line");
   fRadID =
      new TGRadioButton(fLineButGrp, new TGHotString("&ID"), kIDRadBut);
   fRadOK =
      new TGRadioButton(fLineButGrp, new TGHotString("&OK"), kOKRadBut);
   fRadID->Connect("Pressed()", "KVNewGridDialog", this,
                   "HandleButtons(Int_t)");
   fRadID->SetState(kButtonDown);
   fRadOK->Connect("Pressed()", "KVNewGridDialog", this,
                   "HandleButtons(Int_t)");
   fLineButGrp->AddFrame(fRadID,
                         new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 5,
                                           2, 2));
   fLineButGrp->AddFrame(fRadOK,
                         new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 5,
                                           2, 2));
   fLineClass = new TGComboBox(fLineButGrp);
   fLineClass->AddEntry("KVIDZALine", 1);

   fLineClass->Resize(120, 20);
   fLineClass->Connect("Selected(const char*)", "KVNewGridDialog", this,
                       "SetLineClass(const char*)");
   fLineClass->Select(1);
   fLineButGrp->AddFrame(fLineClass,
                         new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2,
                                           2, 2));
   fButsHorFrame = new TGHorizontalFrame(fLineButGrp);
   fNewLine = new TGTextButton(fButsHorFrame, "Add");
   fNewLine->SetToolTipText("Draw new line and add to grid");
   fNewLine->Connect("Clicked()", "KVNewGridDialog", this, "NewLine()");
   fButsHorFrame->AddFrame(fNewLine,
                           new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   fUpdate = new TGTextButton(fButsHorFrame, "Update");
   fUpdate->SetToolTipText("Refresh lists of lines");
   fUpdate->Connect("Clicked()", "KVNewGridDialog", this,
                    "UpdateLineLists()");
   fButsHorFrame->AddFrame(fUpdate,
                           new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   //button for removing selected line - initially disabled (no line selected)
   fRemove = new TGTextButton(fButsHorFrame, "Remove");
   fRemove->SetToolTipText("Remove selected line from grid");
   fRemove->Connect("Clicked()", "KVNewGridDialog", this,
                    "RemoveSelectedLine()");
   fRemove->SetEnabled(kFALSE);
   fButsHorFrame->AddFrame(fRemove,
                           new TGLayoutHints(kLHintsNormal, 2, 2, 2, 2));
   fLineButGrp->AddFrame(fButsHorFrame,
                         new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2,
                                           2, 2, 2));

   fHFlines->AddFrame(fLineButGrp,
                      new TGLayoutHints(kLHintsLeft | kLHintsTop, 10, 5, 2,
                                        2));

   //list boxes vertical container
   fVFLBoxes = new TGVerticalFrame(fHFlines, 10, 10);
   //OK lines
   fGFOK = new TGGroupFrame(fVFLBoxes, "OK lines");
   //list box for OK line names
   fOKLines = new TGListBox(fGFOK, 0);
   fOKLines->Resize(200, 60);
   fOKLines->Connect("Selected(Int_t)", "KVNewGridDialog", this,
                     "SelectOKLine(Int_t)");

   fGFOK->AddFrame(fOKLines,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                     kLHintsExpandX, 1, 1, 1, 1));
   fVFLBoxes->AddFrame(fGFOK,
                       new TGLayoutHints(kLHintsTop | kLHintsExpandX, 1, 1,
                                         1, 1));

   //ID lines
   fGFID = new TGGroupFrame(fVFLBoxes, "ID lines");
   //list box for ID line names
   fIDLines = new TGListBox(fGFID, 0);
   fIDLines->Resize(200, 150);
   fIDLines->Connect("Selected(Int_t)", "KVNewGridDialog", this,
                     "SelectIDLine(Int_t)");

   fGFID->AddFrame(fIDLines,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                     kLHintsExpandX, 1, 1, 1, 1));
   fVFLBoxes->AddFrame(fGFID,
                       new TGLayoutHints(kLHintsTop | kLHintsExpandX, 1, 1,
                                         1, 1));

   fHFlines->AddFrame(fVFLBoxes,
                      new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                        kLHintsExpandX, 30, 5, 5, 5));
   fMain->AddFrame(fHFlines,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                     kLHintsExpandX, 0, 0, 0, 0));
}

void KVNewGridDialog::map_interface(Option_t* dialog_type)
{
   //final set up and display of dialogue box
   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   if (!strcmp(dialog_type, "New")) {
      fMain->SetWindowName("New ID grid");
   } else if (!strcmp(dialog_type, "Edit")) {
      fMain->SetWindowName("Edit ID grid");
   } else if (!strcmp(dialog_type, "Copy")) {
      fMain->SetWindowName("Edit properties of copied ID grid");
   }
   fMain->MapWindow();

   if (!strcmp(dialog_type, "Edit") || !strcmp(dialog_type, "Copy")) {
      SelectRuns(choose_runs);
      SelectRings(choose_rings);
      SelectMods(choose_mods);
      SelectScales(choose_scales);
   }
   //fill lists of lines if in "Edit" mode
   if (!strcmp(dialog_type, "Edit")) {
      FillLineLists();
   }
}

void KVNewGridDialog::set_selected_grid(KVIDGraph* g)
{
   //store pointer to selected grid
   fSelectedGrid = g;
}

KVNewGridDialog::KVNewGridDialog(const TGWindow* p, const TGWindow* main,
                                 UInt_t w, UInt_t h,
                                 Option_t* dialog_type, KVIDGraph* g)
{
   //Create dialogue box as child window of 'main'
   //dialog_type can be "New", "Edit" or "Copy"
   //Appearance and behaviour of dialog is different in each case

   if (!strcmp(dialog_type, "Edit") || !strcmp(dialog_type, "Copy")) {
      //in "Edit" and "Copy" modes a pointer to the selected grid is passed to the ctor.
      set_selected_grid(g);
      //if fSelectedGrid is still = 0 after this call, there is something wrong with the pointer 'g' given to the ctor,
      //and we should exit without setting up the interface
      if (!fSelectedGrid)
         return;
   }
   //initialise dialog
   init_interface(p, main, w, h, dialog_type);

   //for "New grid"  dialog, add class list drop-down menu
   if (!strcmp(dialog_type, "New"))
      new_grid_class_list();

   //for "edit" and "copy" dialog boxes, we add controls for setting scale factors, detector numbers, etc.
   if (!strcmp(dialog_type, "Edit") || !strcmp(dialog_type, "Copy"))
      layout_parameter_control_widgets();

   //add buttons for "OK", "Cancel" etc.
   layout_ok_apply_cancel_buttons(dialog_type);

   //set up grid editor if in "Edit" mode
   if (!strcmp(dialog_type, "Edit"))
      layout_grid_editor();

   //finish set up and draw dialogue box
   map_interface(dialog_type);

}

KVNewGridDialog::~KVNewGridDialog()
{
   //Delete all widgets

   if (fMain) {
      delete fMain;
      fMain = 0;
   }
}

void KVNewGridDialog::HandleButtons(Int_t id)
{
   //deal with radio buttons

   if (id == -1) {
      TGButton* btn = (TGButton*) gTQSender;
      id = btn->WidgetId();
   }
   fLineType = id;
   switch (id) {

      case kIDRadBut:
         //user selects line type "ID"
         //class(es) depend on grid class
         fRadOK->SetState(kButtonUp);
         fLineClass->RemoveAll();
         if (fGridClass == "KVIDGCsI") {
            fLineClass->AddEntry("KVIDCsIRLLine", 1);
            fLineClassName = "KVIDCsIRLLine";
         } else {
            fLineClass->AddEntry("KVIDZALine", 1);
            fLineClassName = "KVIDZALine";
         }
         fLineClass->Resize(120, 20);
         fLineClass->Select(1);
         break;

      case kOKRadBut:
         //user selects line type "OK"
         //class is KVIDCutLine
         fRadID->SetState(kButtonUp);
         fLineClass->RemoveAll();
         fLineClass->AddEntry("KVIDCutLine", 1);
         fLineClassName = "KVIDCutLine";
         fLineClass->Resize(120, 20);
         fLineClass->Select(1);
         break;
   }
}

void KVNewGridDialog::DoClose()
{
   if (fSelectedLine) {
      //reset colour and editability of last selected line
      fSelectedLine->SetLineColor(fLastSelectedLineColor);
      fSelectedLine->SetEditable(kFALSE);
      if (gPad) {
         gPad->Modified();
         gPad->Update();
      }
   }
   TTimer::SingleShot(150, "KVNewGridDialog", this, "CloseWindow()");
}

void KVNewGridDialog::CloseWindow()
{
   if (fSelectedLine) {
      //reset colour and editability of last selected line
      fSelectedLine->SetLineColor(fLastSelectedLineColor);
      fSelectedLine->SetEditable(kFALSE);
      if (gPad) {
         gPad->Modified();
         gPad->Update();
      }
   }
   delete this;
}

void KVNewGridDialog::CreateGrid()
{
   //if grid class is KVIDZGrid, we create a KVIDZAGrid and use SetOnlyZId(kTRUE)
   TClass* clas = 0;
   if (fGridClass == "KVIDZGrid") clas = TClass::GetClass("KVIDZAGrid");
   else clas = TClass::GetClass(fGridClass.Data());
   KVIDGraph* grid = (KVIDGraph*) clas->New();
   if (fGridClass == "KVIDZGrid") grid->SetOnlyZId(kTRUE);
   DoClose();
}

void KVNewGridDialog::ModifyGrid()
{
   ReadAndSetSelectedGridProperties(fSelectedGrid);
   gIDGridManager->Modified();
   //DoClose();
}

void KVNewGridDialog::CopyGrid()
{
   if (!fSelectedGrid) {
      Error("CopyGrid", "No selected grid to copy!");
      return;
   }
   KVIDGraph* grid = (KVIDGraph*) fSelectedGrid->IsA()->New();
   fSelectedGrid->Copy((TObject&)(*grid));
   ReadAndSetSelectedGridProperties(grid);
   DoClose();
}

void KVNewGridDialog::ReadAndSetSelectedGridProperties(KVIDGraph* grid)
{
   //Depending on state of check buttons, read and set (or reset, i.e. remove)
   //parameters describing grid characteristics
   if (choose_rings) {
      grid->GetParameters()->SetValue("Ring min", ring_min);
      grid->GetParameters()->SetValue("Ring max", ring_max);
   } else {
      grid->GetParameters()->RemoveParameter("Ring min");
      grid->GetParameters()->RemoveParameter("Ring max");
   }
   if (choose_mods) {
      grid->GetParameters()->SetValue("Mod min", mod_min);
      grid->GetParameters()->SetValue("Mod max", mod_max);
   } else {
      grid->GetParameters()->RemoveParameter("Mod min");
      grid->GetParameters()->RemoveParameter("Mod max");
   }
   if (choose_runs) {
      grid->GetParameters()->SetValue("First run", first_run);
      grid->GetParameters()->SetValue("Last run", last_run);
   } else {
      grid->GetParameters()->RemoveParameter("First run");
      grid->GetParameters()->RemoveParameter("Last run");
   }
   if (choose_scales) {
      if (scale_x != 1)
         grid->SetXScaleFactor(scale_x);
      else
         grid->SetXScaleFactor();
      if (scale_y != 1)
         grid->SetYScaleFactor(scale_y);
      else
         grid->SetYScaleFactor();
   } else {
      grid->SetXScaleFactor();
      grid->SetYScaleFactor();
   }
}

void KVNewGridDialog::SetGridClass(const char* name)
{
   fGridClass = name;
}

void KVNewGridDialog::SetLineClass(const char* name)
{
   fLineClassName = name;
}

void KVNewGridDialog::SelectRuns(Bool_t yes)
{
   //connected to 'select runs for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the number entry fields & labels for first & last run are activated
   //if yes==kFALSE, the number entry fields & labels for first & last run are deactivated
   choose_runs = yes;
   if (yes) {
      fRunMin->SetState(kTRUE);
      fRunMinLabel->Enable();
      fRunMax->SetState(kTRUE);
      fRunMaxLabel->Enable();
   } else {
      fRunMin->SetState(kFALSE);
      fRunMinLabel->Disable();
      fRunMax->SetState(kFALSE);
      fRunMaxLabel->Disable();
   }
}

void KVNewGridDialog::ReadRuns()
{
   //Read current values of first_run and last_run from number fields
   first_run = (Int_t) fRunMin->GetIntNumber();
   last_run = (Int_t) fRunMax->GetIntNumber();
}

void KVNewGridDialog::SelectRings(Bool_t yes)
{
   //connected to 'select rings for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the number entry fields & labels for min & max ring are activated
   //if yes==kFALSE, the number entry fields & labels for min & max are deactivated
   choose_rings = yes;
   if (yes) {
      fRingMin->SetState(kTRUE);
      fRingMinLabel->Enable();
      fRingMax->SetState(kTRUE);
      fRingMaxLabel->Enable();
   } else {
      fRingMin->SetState(kFALSE);
      fRingMinLabel->Disable();
      fRingMax->SetState(kFALSE);
      fRingMaxLabel->Disable();
   }
}

void KVNewGridDialog::ReadRings()
{
   //Read current values of first_run and last_run from number fields
   ring_min = (Int_t) fRingMin->GetIntNumber();
   ring_max = (Int_t) fRingMax->GetIntNumber();
}

void KVNewGridDialog::SelectMods(Bool_t yes)
{
   //connected to 'select modules for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the number entry fields & labels for min & max mod are activated
   //if yes==kFALSE, the number entry fields & labels for min & max mod are deactivated
   choose_mods = yes;
   if (yes) {
      fModMin->SetState(kTRUE);
      fModMinLabel->Enable();
      fModMax->SetState(kTRUE);
      fModMaxLabel->Enable();
   } else {
      fModMin->SetState(kFALSE);
      fModMinLabel->Disable();
      fModMax->SetState(kFALSE);
      fModMaxLabel->Disable();
   }
}

void KVNewGridDialog::ReadMods()
{
   //Read current values of first_run and last_run from number fields
   mod_min = (Int_t) fModMin->GetIntNumber();
   mod_max = (Int_t) fModMax->GetIntNumber();
}

void KVNewGridDialog::SelectScales(Bool_t yes)
{
   //connected to 'select scaling for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the number entry fields & labels are activated
   //if yes==kFALSE, the number entry fields & labels are deactivated
   choose_scales = yes;
   if (yes) {
      fScaleMin->SetState(kTRUE);
      fScaleMinLabel->Enable();
      fScaleMax->SetState(kTRUE);
      fScaleMaxLabel->Enable();
   } else {
      fScaleMin->SetState(kFALSE);
      fScaleMinLabel->Disable();
      fScaleMax->SetState(kFALSE);
      fScaleMaxLabel->Disable();
   }
}

void KVNewGridDialog::ReadScales()
{
   //Read current values of scaling factors from number fields
   scale_x = fScaleMin->GetNumber();
   scale_y = fScaleMax->GetNumber();
}

void KVNewGridDialog::NewLine()
{
   //called when "New" button is pressed
   cout << "fLineClassName=" << fLineClassName.Data() << endl;
   if (fLineType == kIDRadBut)
      fSelectedGrid->DrawAndAdd("ID", fLineClassName.Data());
   else
      fSelectedGrid->DrawAndAdd("OK", fLineClassName.Data());

}

void KVNewGridDialog::FillLineLists()
{
   //fill list boxes with lines from selected grid

   KVIDentifier* line = 0;
   Int_t index = 0;
   //ok lines
   KVList* oklines = fSelectedGrid->GetCuts();
   if (oklines->GetSize() > 0) {

      TIter nline(oklines);
      while ((line = (KVIDentifier*) nline())) {

         fOKLines->AddEntry(line->GetName(), index++);

      }
      fFirstOKline = 0;
      fLastOKline = index - 1;
      fOKLines->MapSubwindows();
      fOKLines->Layout();
   }
   index = 0;
   KVList* idlines = fSelectedGrid->GetIdentifiers();
   if (idlines->GetSize() > 0) {

      TIter nline(idlines);
      while ((line = (KVIDentifier*) nline())) {

         fIDLines->AddEntry(line->GetName(), index++);

      }
      fFirstIDline = 0;
      fLastIDline = index - 1;
      fIDLines->MapSubwindows();
      fIDLines->Layout();
   }
}

void KVNewGridDialog::SetAllLinesNonEditable()
{
   //makes all lines from selected grid not be editable i.e. SetEditable(kFALSE)
   //is called for all lines. this is called when the Edit dialog is closed, to avoid
   //any lines remaining editable by accident.

   KVIDentifier* line = 0;
   //ok lines
   KVList* oklines = fSelectedGrid->GetCuts();
   if (oklines->GetSize() > 0) {

      TIter nline(oklines);
      while ((line = (KVIDentifier*) nline())) {

         line->SetEditable(kFALSE);

      }
   }
   KVList* idlines = fSelectedGrid->GetIdentifiers();
   if (idlines->GetSize() > 0) {

      TIter nline(idlines);
      while ((line = (KVIDentifier*) nline())) {

         line->SetEditable(kFALSE);

      }
   }
}

void KVNewGridDialog::ClearLineLists()
{
   //remove all entries from the two line lists

   if (fFirstOKline <= fLastOKline) {
      fOKLines->RemoveEntries(fFirstOKline, fLastOKline);
      fOKLines->Layout();
      fFirstOKline = 0;
      fLastOKline = -1;
   }
   if (fFirstIDline <= fLastIDline) {
      fIDLines->RemoveEntries(fFirstIDline, fLastIDline);
      fIDLines->Layout();
      fFirstIDline = 0;
      fLastIDline = -1;
   }
}


void KVNewGridDialog::UpdateLineLists()
{
   //clear then refill lists of ID and OK lines
   ClearLineLists();
   FillLineLists();
}


void KVNewGridDialog::SelectIDLine(Int_t id)
{
   //Called when user clicks an ID line in the ID line list box
   //We need to deselect any selected OK line in the OK line list box
   //The new selected line becomes editable; any previously selected line becomes non-editable.
   //The "Remove Line" button becomes active
   DeselectOKLine();
   if (fSelectedLine) {
      //reset colour and editability of last selected line
      fSelectedLine->SetLineColor(fLastSelectedLineColor);
      fSelectedLine->SetEditable(kFALSE);
   }
   fLastSelectedIDLine = id;
   fSelectedLine = (KVIDentifier*)fSelectedGrid->GetIdentifiers()->At(id);
   if (!fSelectedLine) {
      cout << "No ID line found with index " << id << endl;
      return;
   }
   fSelectedLine->SetEditable(kTRUE);   //now we can edit line with mouse
   fLastSelectedLineColor = fSelectedLine->GetLineColor();
   fSelectedLine->SetLineColor(fColorOfSelectedLines);
   if (gPad) {
      gPad->Modified();
      gPad->Update();
   }
   fRemove->SetEnabled(kTRUE);
}

void KVNewGridDialog::SelectOKLine(Int_t id)
{
   //Called when user clicks an OK line in the OK line list box
   //We need to deselect any selected ID line in the ID line list box
   //The "Remove Line" button becomes active
   DeselectIDLine();
   if (fSelectedLine) {
      //reset colour and editability of last selected line
      fSelectedLine->SetLineColor(fLastSelectedLineColor);
      fSelectedLine->SetEditable(kFALSE);
   }
   fLastSelectedOKLine = id;
   fSelectedLine = (KVIDentifier*)fSelectedGrid->GetIdentifiers()->At(id);
   if (!fSelectedLine) {
      cout << "No OK line found with index " << id << endl;
      return;
   }
   fSelectedLine->SetEditable(kTRUE);   //now we can edit line with mouse
   fLastSelectedLineColor = fSelectedLine->GetLineColor();
   fSelectedLine->SetLineColor(fColorOfSelectedLines);
   if (gPad) {
      gPad->Modified();
      gPad->Update();
   }
   fRemove->SetEnabled(kTRUE);
}

void KVNewGridDialog::DeselectOKLine()
{
   //Deselect "fLastSelectedOKLine" in the OK line list box
   if (fLastSelectedOKLine > -1) {
      fOKLines->Select(fLastSelectedOKLine, kFALSE);
      fLastSelectedOKLine = -1;
   }
}

void KVNewGridDialog::DeselectIDLine()
{
   //Deselect "fLastSelectedIDLine" in the ID line list box
   if (fLastSelectedIDLine > -1) {
      fIDLines->Select(fLastSelectedIDLine, kFALSE);
      fLastSelectedIDLine = -1;
   }
}

void KVNewGridDialog::RemoveSelectedLine()
{
   //Remove from edited grid the selected line (and destroy the line)

   //remove line from pad if it has been drawn
   if (gPad) {
      if (gPad->GetListOfPrimitives()->FindObject(fSelectedLine)) {
         gPad->GetListOfPrimitives()->Remove(fSelectedLine);
         gPad->Modified();
         gPad->Update();
      }
   }
   if (fLastSelectedIDLine > -1) {
      //remove ID line
      DeselectIDLine();
      fSelectedGrid->RemoveIdentifier(fSelectedLine);
   } else if (fLastSelectedOKLine > -1) {
      //remove OK line
      DeselectOKLine();
      fSelectedGrid->RemoveCut(fSelectedLine);
   }
   UpdateLineLists();
   fRemove->SetEnabled(kFALSE);
}
#ifdef __WITHOUT_TGBUTTON_SETENABLED
#undef SetEnabled
#endif
