/*
$Id: KVNewGridRRMDialog.cpp,v 1.5 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.5 $
$Date: 2009/03/03 13:36:00 $
$Author: franklan $
*/

#include "KVNewGridRRMDialog.h"

using namespace std;

ClassImp(KVNewGridRRMDialog)
//////////////////////////////////////////
// KVNewGridRRMDialog
//////////////////////////////////////////
KVNewGridRRMDialog::KVNewGridRRMDialog(): KVNewGridDialog()
{
   //Default ctor. Do nothing.
}

KVNewGridRRMDialog::KVNewGridRRMDialog(const TGWindow* p,
                                       const TGWindow* main, UInt_t w,
                                       UInt_t h, Option_t* dialog_type,
                                       KVIDGraph* g)
   : KVNewGridDialog()
{
   //Dialog box for grid derived from KVINDRARRMValidator (with lists of runs, rings and modules).
   //We use text entry widgets to handle these lists, instead of the "min" and "max" number entry
   //fields used in the default version
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

void KVNewGridRRMDialog::set_selected_grid(KVIDGraph* g)
{
   //Make sure selected grid is of right type i.e. inherits from KVINDRARRMValidator, otherwise return with error message
   fSelectedGrid = g;
   if (!g->InheritsFrom("KVINDRARRMValidator")) {
      cout << "Error in KVNewGridRRMDialog ctor:" << endl;
      cout <<
           "This dialogue box can only be used with grids derived from KVINDRARRMValidator"
           << endl;
      fSelectedGrid = 0;        // will force exit from base class ctor, dialogue box will not be built
      return;
   }
   //get pointer to RRMValidator part of grid (for access to GetRunList(), GetRingList(), GetModuleList() methods)
   fRRMGrid = dynamic_cast < KVINDRARRMValidator* >(g);
}

void KVNewGridRRMDialog::layout_parameter_control_widgets()
{
   //for "edit" and "copy" dialog boxes, we add controls for setting scale factors, detector numbers, etc.

   //set state of interface from selected grid
   _set_choose_rings(!fRRMGrid->GetRingList().IsEmpty());
   _set_choose_mods(!fRRMGrid->GetModuleList().IsEmpty());
   _set_choose_runs(!fRRMGrid->GetRunList().IsEmpty());
   if (fSelectedGrid->GetXScaleFactor() != 1.0
         || fSelectedGrid->GetYScaleFactor() != 1.0) {
      _set_choose_scales(kTRUE);
      _set_scale_x(fSelectedGrid->GetXScaleFactor());
      _set_scale_y(fSelectedGrid->GetYScaleFactor());
   } else
      _set_choose_scales(kFALSE);

   fMatrixFrame = new TGGroupFrame(fMain, "Grid parameters");
   //**************Select run numbers************
   fHFRuns = new TGHorizontalFrame(fMatrixFrame);
   fSetRuns = new TGCheckButton(fHFRuns, "Set runs :", CHK_BUT_RUNS);
   //set checked according to last known state
   fSetRuns->SetDown(_choose_runs());
   fSetRuns->Connect("Toggled(Bool_t)", "KVNewGridRRMDialog", this,
                     "SelectRuns(Bool_t)");
   fRunList = new TGTextEntry(fHFRuns, fRRMGrid->GetRunList().AsString());
   fRunList->
   SetToolTipText
   ("Enter a list of runs for which grid is valid, i.e. \"1000-1009, 1011, 984-999\"",
    200);
   fRunList->SetWidth(300);
   fHFRuns->AddFrame(fSetRuns,
                     new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fHFRuns->AddFrame(fRunList,
                     new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fMatrixFrame->AddFrame(fHFRuns,
                          new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5,
                                5, 2));
   //**************Select ring numbers************
   fHFRings = new TGHorizontalFrame(fMatrixFrame);
   fSetRings = new TGCheckButton(fHFRings, "Set rings :", CHK_BUT_RINGS);
   //set checked according to last known state
   fSetRings->SetDown(_choose_rings());
   fSetRings->Connect("Toggled(Bool_t)", "KVNewGridRRMDialog", this,
                      "SelectRings(Bool_t)");
   fRingList =
      new TGTextEntry(fHFRings, fRRMGrid->GetRingList().AsString());
   fRingList->
   SetToolTipText
   ("Enter a list of rings for which grid is valid, i.e. \"2-3, 5, 8-9\"",
    200);
   fRingList->SetWidth(300);
   fHFRings->AddFrame(fSetRings,
                      new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fHFRings->AddFrame(fRingList,
                      new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fMatrixFrame->AddFrame(fHFRings,
                          new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5,
                                5, 2));

   //**************Select module numbers************
   fHFModules = new TGHorizontalFrame(fMatrixFrame);
   fSetMods = new TGCheckButton(fHFModules, "Set mods :", CHK_BUT_MODS);
   //set checked according to last known state
   fSetMods->SetDown(_choose_mods());
   fSetMods->Connect("Toggled(Bool_t)", "KVNewGridRRMDialog", this,
                     "SelectMods(Bool_t)");
   fModList =
      new TGTextEntry(fHFModules, fRRMGrid->GetModuleList().AsString());
   fModList->
   SetToolTipText
   ("Enter a list of modules for which grid is valid, i.e. \"1-5, 11, 21-23\"",
    200);
   fModList->SetWidth(300);
   fHFModules->AddFrame(fSetMods,
                        new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fHFModules->AddFrame(fModList,
                        new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fMatrixFrame->AddFrame(fHFModules,
                          new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5,
                                5, 2));

   //**************Scaling factors************
   fHFScales = new TGHorizontalFrame(fMatrixFrame);
   fSetScales = new TGCheckButton(fHFScales, "Scaling :", CHK_BUT_SCALES);
   //set checked according to last known state
   fSetScales->SetDown(_choose_scales());
   fSetScales->Connect("Toggled(Bool_t)", "KVNewGridRRMDialog", this,
                       "SelectScales(Bool_t)");
   fScaleMinLabel = new TGLabel(fHFScales, "X factor");
   fScaleMaxLabel = new TGLabel(fHFScales, "Y factor");
   fScaleMin =
      new TGNumberEntry(fHFScales, _scale_x(), 5, NE_SCALE_X,
                        TGNumberFormat::kNESReal,
                        TGNumberFormat::kNEAPositive);
   fScaleMax =
      new TGNumberEntry(fHFScales, _scale_y(), 5, NE_SCALE_Y,
                        TGNumberFormat::kNESReal,
                        TGNumberFormat::kNEAPositive);

   fHFScales->AddFrame(fSetScales,
                       new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fHFScales->AddFrame(fScaleMinLabel,
                       new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fHFScales->AddFrame(fScaleMin,
                       new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fHFScales->AddFrame(fScaleMaxLabel,
                       new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fHFScales->AddFrame(fScaleMax,
                       new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fMatrixFrame->AddFrame(fHFScales,
                          new TGLayoutHints(kLHintsTop | kLHintsCenterX, 1,
                                1, 1, 1));

   fMain->AddFrame(fMatrixFrame,
                   new TGLayoutHints(kLHintsCenterX | kLHintsTop, 10, 5,
                                     10, 5));
}


void KVNewGridRRMDialog::SelectRuns(Bool_t yes)
{
   //connected to 'select runs for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the text entry field for valid runs is activated
   //if yes==kFALSE, the text entry field for valid runs is deactivated
   _set_choose_runs(yes);
   fRunList->SetState(yes);
}

void KVNewGridRRMDialog::ReadRuns()
{
   //Read current values in run list
   fRunListString = fRunList->GetText();
}

void KVNewGridRRMDialog::SelectRings(Bool_t yes)
{
   //connected to 'select rings for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the text entry field for valid rings is activated
   //if yes==kFALSE, the text entry field for valid rings is deactivated
   _set_choose_rings(yes);
   fRingList->SetState(yes);
}

void KVNewGridRRMDialog::ReadRings()
{
   //Read current values in ring list
   fRingListString = fRingList->GetText();
}

void KVNewGridRRMDialog::SelectMods(Bool_t yes)
{
   //connected to 'select modules for grid' check button 'Toggled(Bool_t)' signal
   //if yes==kTRUE, the text entry field for valid modules is activated
   //if yes==kFALSE, the text entry field for valid modules is deactivated
   _set_choose_mods(yes);
   fModList->SetState(yes);
}

void KVNewGridRRMDialog::ReadMods()
{
   //Read current values of first_run and last_run from number fields
   fModListString = fModList->GetText();
}

void KVNewGridRRMDialog::ReadAndSetSelectedGridProperties(KVIDGraph* grid)
{
   KVINDRARRMValidator* RRMGrid =
      dynamic_cast < KVINDRARRMValidator* >(grid);
   if (_choose_rings()) {
      RRMGrid->GetRingList().SetList(fRingListString);
   } else {
      RRMGrid->GetRingList().Clear();
   }
   if (_choose_mods()) {
      RRMGrid->GetModuleList().SetList(fModListString);
   } else {
      RRMGrid->GetModuleList().Clear();
   }
   if (_choose_runs()) {
      RRMGrid->GetRunList().SetList(fRunListString);
   } else {
      RRMGrid->GetRunList().Clear();
   }
   if (_choose_scales()) {
      if (_scale_x() != 1)
         grid->SetXScaleFactor(_scale_x());
      else
         grid->SetXScaleFactor();
      if (_scale_y() != 1)
         grid->SetYScaleFactor(_scale_y());
      else
         grid->SetYScaleFactor();
   } else {
      grid->SetXScaleFactor();
      grid->SetYScaleFactor();
   }
}
