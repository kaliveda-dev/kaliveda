/*
$Id: KVNewGridRRMDialog.h,v 1.4 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.4 $
$Date: 2009/03/03 13:36:00 $
$Author: franklan $
*/

#ifndef __KVNEWGRRMDIA_H
#define __KVNEWGRRMDIA_H

#include "KVNewGridDialog.h"
#include "KVINDRARRMValidator.h"
#include "TGTextEntry.h"

class KVNewGridRRMDialog: public KVNewGridDialog {

   RQ_OBJECT("KVNewGridRRMDialog")

protected:
   KVINDRARRMValidator* fRRMGrid;
   TGTextEntry* fRunList;
   TGTextEntry* fRingList;
   TGTextEntry* fModList;
   TGHorizontalFrame* fHFRuns;
   TGHorizontalFrame* fHFRings;
   TGHorizontalFrame* fHFModules;
   TGHorizontalFrame* fHFScales;
   TString fRunListString;
   TString fRingListString;
   TString fModListString;

   virtual void set_selected_grid(KVIDGraph* g);
   virtual void layout_parameter_control_widgets();

public:

   KVNewGridRRMDialog();
   KVNewGridRRMDialog(const TGWindow* p, const TGWindow* main,
                      UInt_t w = 1, UInt_t h = 1, Option_t* type =
                         "New", KVIDGraph* g = 0);
   virtual ~ KVNewGridRRMDialog()
   {
   };

   virtual void SelectRuns(Bool_t);
   virtual void ReadRuns();
   virtual void SelectRings(Bool_t);
   virtual void ReadRings();
   virtual void SelectMods(Bool_t);
   virtual void ReadMods();
   virtual void ReadAndSetSelectedGridProperties(KVIDGraph*);

   ClassDef(KVNewGridRRMDialog, 0)      //Dialog box for creation of new grid in KVIDGridManagerGUI, for grids which inherit from KVINDRARRMValidator
};
#endif
