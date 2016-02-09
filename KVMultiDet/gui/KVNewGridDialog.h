/*
$Id: KVNewGridDialog.h,v 1.7 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.7 $
$Date: 2009/03/03 13:36:00 $
$Author: franklan $
*/

#ifndef __KVNEWGDIA_H
#define __KVNEWGDIA_H

#include "TGFrame.h"
#include "TGMenu.h"
#include "TGListBox.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "KVIDGraph.h"
#include "RQ_OBJECT.h"
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGProgressBar.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// KVNewGridDialog
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class KVNewGridDialog {

   RQ_OBJECT("KVNewGridDialog")

protected:
   TGTransientFrame* fMain;
   TGComboBox* fClassList;
   TGHorizontalFrame* fHframe;
   TGHorizontalFrame* fHframe2;

   TGCompositeFrame* fMatrixFrame;

   TGCheckButton* fSetRuns;
   TGNumberEntry* fRunMin;
   TGNumberEntry* fRunMax;
   TGLabel* fRunMinLabel;
   TGLabel* fRunMaxLabel;
   TGCheckButton* fSetRings;
   TGNumberEntry* fRingMin;
   TGNumberEntry* fRingMax;
   TGLabel* fRingMinLabel;
   TGLabel* fRingMaxLabel;
   TGCheckButton* fSetMods;
   TGNumberEntry* fModMin;
   TGNumberEntry* fModMax;
   TGLabel* fModMinLabel;
   TGLabel* fModMaxLabel;
   TGCheckButton* fSetScales;
   TGNumberEntry* fScaleMin;
   TGNumberEntry* fScaleMax;
   TGLabel* fScaleMinLabel;
   TGLabel* fScaleMaxLabel;

   TGTextButton* fCreate;
   TGTextButton* fCancel;
   TGTextButton* fApply;
   TGLabel* fClassLabel;
   static TString fGridClass;
   static Int_t fClassIndex;

   static Int_t first_run;
   static Int_t last_run;
   static Bool_t choose_runs;

   static Int_t ring_min;
   static Int_t ring_max;
   static Bool_t choose_rings;

   static Int_t mod_min;
   static Int_t mod_max;
   static Bool_t choose_mods;

   static Double_t scale_x;
   static Double_t scale_y;
   static Bool_t choose_scales;

   void _set_choose_runs(Bool_t);
   Bool_t _choose_runs();
   void _set_choose_rings(Bool_t);
   Bool_t _choose_rings();
   void _set_choose_mods(Bool_t);
   Bool_t _choose_mods();
   void _set_choose_scales(Bool_t);
   Bool_t _choose_scales();
   void _set_scale_x(Double_t);
   void _set_scale_y(Double_t);
   Double_t _scale_x();
   Double_t _scale_y();

   enum {
      CHK_BUT_RUNS,
      CHK_BUT_RINGS,
      CHK_BUT_MODS,
      CHK_BUT_SCALES
   };
   enum {
      NE_RUN_MIN,
      NE_RUN_MAX,
      NE_RING_MIN,
      NE_RING_MAX,
      NE_MOD_MIN,
      NE_MOD_MAX,
      NE_SCALE_X,
      NE_SCALE_Y
   };

   KVIDGraph* fSelectedGrid;

   /**********grid lines lists and buttons******************/
   TGHorizontalFrame* fHFlines;
   TGGroupFrame* fLineButGrp;
   TGTextButton* fNewLine;      //add new line
   TGTextButton* fUpdate;       //add new line
   TGTextButton* fRemove;       //remove selected line
   TGHorizontalFrame* fButsHorFrame;
   TGRadioButton* fRadOK;
   TGRadioButton* fRadID;
   TGComboBox* fLineClass;
   TString fLineClassName;
   Int_t fLineType;
   TGVerticalFrame* fVFLBoxes;
   TGGroupFrame* fGFOK;
   TGListBox* fOKLines;
   TGGroupFrame* fGFID;
   TGListBox* fIDLines;
   KVIDentifier* fSelectedLine;
   Int_t fLastSelectedIDLine;
   Int_t fLastSelectedOKLine;
   Color_t fLastSelectedLineColor;
   static Color_t fColorOfSelectedLines;
   Int_t fFirstIDline, fLastIDline, fFirstOKline, fLastOKline;

   enum {
      kIDRadBut,
      kOKRadBut
   };

   virtual void set_selected_grid(KVIDGraph* g);
   virtual void init_interface(const TGWindow* p, const TGWindow* main,
                               UInt_t w, UInt_t h, Option_t* dialog_type);
   virtual void new_grid_class_list();
   virtual void layout_parameter_control_widgets();
   virtual void layout_ok_apply_cancel_buttons(Option_t* dialog_type);
   virtual void layout_grid_editor();
   virtual void map_interface(Option_t* dialog_type);

public:

   KVNewGridDialog()
   {
      fMain = 0;
   };
   KVNewGridDialog(const TGWindow* p, const TGWindow* main, UInt_t w =
                      1, UInt_t h = 1, Option_t* type = "New", KVIDGraph* g =
                      0);
   virtual ~ KVNewGridDialog();

   void DoClose();
   void CloseWindow();
   void CreateGrid();
   void ModifyGrid();
   void CopyGrid();
   void SetGridClass(const char* name);
   void SetClassIndex(Int_t index)
   {
      fClassIndex = index;
   };
   virtual void SelectRuns(Bool_t);
   virtual void ReadRuns();
   virtual void SelectRings(Bool_t);
   virtual void ReadRings();
   virtual void SelectMods(Bool_t);
   virtual void ReadMods();
   void SelectScales(Bool_t);
   void ReadScales();
   void NewLine();
   void FillLineLists();
   void ClearLineLists();
   void UpdateLineLists();
   void SelectOKLine(Int_t);
   void SelectIDLine(Int_t);
   void SetLineClass(const char* name);
   void DeselectOKLine();
   void DeselectIDLine();
   void HandleButtons(Int_t id = -1);
   virtual void ReadAndSetSelectedGridProperties(KVIDGraph*);
   void SetAllLinesNonEditable();
   void RemoveSelectedLine();

   ClassDef(KVNewGridDialog, 0) //Dialog box for creation of new grid in KVIDGridManagerGUI
};
#endif
