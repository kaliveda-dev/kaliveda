#include "KVIDGridManagerGUI.h"
#include "Riostream.h"
#include "KVIDGridManager.h"
#include "KVIDGrid.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>
#include <KVTestIDGridDialog.h>
#include "KVConfig.h"
#include <KVIDGUITelescopeChooserDialog.h>
#include <KVDropDownDialog.h>
#include <KVIdentificationResult.h>
#include <KVReconstructedNucleus.h>
#include "TEnv.h"
#include "KVInputDialog.h"
#include "KVIDCutLine.h"
#include "KVIDCutContour.h"
#include "KVVirtualIDFitter.h"
#include "TTree.h"

using namespace std;

ClassImp(KVIDGridManagerGUI)

KVIDGridManagerGUI::KVIDGridManagerGUI(): TGMainFrame(gClient->GetRoot(), 500, 300)
{
   fFirstGrid = 0;
   fLastGrid = -1;
   fSelectedGrid = 0;
   fSelectedEntries = 0;
   fLastSelectedGrid = 0;

   fIDGridEditor = new KVIDGridEditor;

   //to have access to online KaliVeda documentation via context menus
   //and dialog box "Online Help" buttons
   gEnv->SetValue("Browser.StartUrl", Form("http://indra.in2p3.fr/KaliVedaDoc/%s/", KVBase::GetKVVersion()));

   //any change of ID grid manager causes UpdateListOfGrids to be called
   gIDGridManager->Connect("Modified()", "KVIDGridManagerGUI", this,
                           "UpdateListOfGrids()");

   //create new manager GUI

   /**************** GRIDS popup menu *****************/
   fMenuFile = new TGPopupMenu(gClient->GetRoot());
   /* cascading "Save grids" menu... */
   TGPopupMenu* sgm = new TGPopupMenu(gClient->GetRoot());
   sgm->AddEntry("Selected", M_GRIDS_SAVE_SEL);
   sgm->AddEntry("Tab", M_GRIDS_SAVE_TAB);
   sgm->AddEntry("All", M_GRIDS_SAVE_ALL);
   fMenuFile->AddPopup("Save...", sgm);

   fMenuFile->AddSeparator();
   /* cascading "Delete grids" menu... */
   sgm = new TGPopupMenu(gClient->GetRoot());
   sgm->AddEntry("Selected", M_GRIDS_DEL_SEL);
   sgm->AddEntry("Tab", M_GRIDS_DEL_TAB);
   sgm->AddEntry("All", M_GRIDS_DEL_ALL);
   fMenuFile->AddPopup("Delete...", sgm);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("&Quit", M_QUIT);

   fMenuFile->Connect("Activated(Int_t)", "KVIDGridManagerGUI", this,
                      "HandleGridsMenu(Int_t)");
   /**************** HELP popup menu *****************/
//   fMenuHelp = new TGPopupMenu(gClient->GetRoot());
//   fMenuHelp->AddEntry("About...", M_HELP_ABOUT);
   /******************MENUBAR*********************/

   fMenuEdit = new TGPopupMenu(gClient->GetRoot());
   sgm = new TGPopupMenu(gClient->GetRoot());
   sgm->AddEntry("Runlist", M_GRIDS_SET_RUNLIST);
   sgm->AddEntry("X Variable", M_GRIDS_SET_VARX);
   sgm->AddEntry("Y Variable", M_GRIDS_SET_VARY);
   //sgm->AddEntry("Mass Formula", M_GRIDS_SET_MASSFORM);
   sgm->AddEntry("Z ID Only", M_GRIDS_SET_ZID);
   fMenuEdit->AddPopup("Set...", sgm);
   fMenuEdit->AddEntry("Clear", M_GRIDS_CLEAR);
   fMenuEdit->Connect("Activated(Int_t)", "KVIDGridManagerGUI", this,
                      "HandleGridsMenu(Int_t)");

   fMenuBarItemLayout =
      new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
//   fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);
   fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
   fMenuBar->AddPopup("&Edit", fMenuEdit, fMenuBarItemLayout);
//   fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

   //add to main window
   AddFrame(fMenuBar,
            new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0,
                              0, 1, 1));
// adding a horizontal line as a separator
   TGHorizontal3DLine* lh = new TGHorizontal3DLine(this);
   AddFrame(lh, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

///////////////////////////*   TOOLBAR *////////////////////////////////
// toolbar icon files
   const char* xpms[] = {
      "ofolder_t.xpm", // open
      "filesaveas.xpm",  // save
      "profile_t.xpm",
      "bld_copy.xpm",
      "sm_delete.xpm",
      "ed_find.png",
      "draw_t.xpm",//
      "root_t.xpm",//
      "refresh1.xpm",
      0
   };
// toolbar tool tip text
   const char* tips[] = {
      "Open file containing grids",
      "Save all grids in current file",
      "New grid",
      "Copy grid",
      "Delete selected grid(s)",
      "Set ID telescopes for grid",
      "Start editor",//"Quit"
      "Open root file",
      "Refresh display",
      0
   };
   int spacing[] = {
      5,
      0,
      20,
      0,
      0,
      0,
      20,
      0,
      20,
      0
   };
   TGButton** buttons[] = {
      &fTBOpen,
      &fTBSave,
      &fTBNewG,
      &fTBCopyG,
      &fTBDelG,
      &fTBSetIDG,
      &fTBStartEditor,
      &fTBOpenRoot,
      &fTBRefresh,
      0
   };
   const char* method[] = {
      "OpenFile()",
      "SaveCurrent()",
      "NewGrid()",
      "CopyGrid()",
      "DeleteSelectedGrids()",
      "SetIDTelescopes()",
      "StartEditor()", //"Quit()"
      "OpenRootFile()",
      "UpdateListOfGrids()",
      0
   };
   fNbButtons = 0;
// structure containing toolbar button information
   ToolBarData_t t[50];
// creation of a toolbar object as a child of main frame
   fToolBar = new TGToolBar(this, 520, 80);
   int i = 0;
   while (xpms[i]) {
      t[i].fPixmap = xpms[i];
      t[i].fTipText = tips[i];
      t[i].fStayDown = kFALSE;
      t[i].fId = i + 1;
      t[i].fButton = NULL;
      *buttons[i] = fToolBar->AddButton(this, &t[i], spacing[i]);
      (*buttons[i])->Connect("Clicked()", "KVIDGridManagerGUI", this, method[i]);
      fTBbuttons[i] = *buttons[i];
      fNbButtons++;
      i++;
   }
   // 'new id line'  & 'new cut' button stays down until line is drawn
//   fTBNewIDL->AllowStayDown(kTRUE);
//   fTBNewCut->AllowStayDown(kTRUE);

// adding the tool bar to the main frame
   AddFrame(fToolBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
///////////////////////////*  END TOOLBAR *////////////////////////////////

// adding a horizontal line as a separator
   lh = new TGHorizontal3DLine(this);
   AddFrame(lh, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

//////////////////////////*STATUS BAR*////////////////////////////////:
   fStatusBar = new TGStatusBar(this);
   AddFrame(fStatusBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
//////////////////////////*STATUS BAR*////////////////////////////////:

   fHframe = new TGHorizontalFrame(this, 10, 10);

   // Tabs for lists of grids. Each tab holds list of grids for a given
   // type of ID telescope.
   fGridListTabs = new TGTab(fHframe, 550, 400);
   fGridListTabs->Connect("Selected(Int_t)", "KVIDGridManagerGUI", this, "TabSelect(Int_t)");
   fIDGridList = 0;
   //initialise tabs with lists of grids
   CreateAndFillTabs();

   fHframe->AddFrame(fGridListTabs,
                     new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10));

   TGVerticalFrame* line_frame = new TGVerticalFrame(fHframe, 350, 400);

   TGLabel* lab1 = new TGLabel(line_frame, "CURRENT GRID IDENTIFIERS");
   line_frame->AddFrame(lab1, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 2, 2, 10, 2));

   // list view for lines in current grid
   fIDLineList = new KVListView(KVIDentifier::Class(), line_frame, 350, 400);
   fIDLineList->SetDataColumns(5);
   fIDLineList->SetDataColumn(0, "Name", "", kTextLeft);
   fIDLineList->SetDataColumn(1, "Z", "", kTextCenterX);
   fIDLineList->SetDataColumn(2, "A", "", kTextCenterX);
   fIDLineList->SetDataColumn(3, "OnlyZId", "OnlyZId", kTextCenterX);
   fIDLineList->SetDataColumn(4, "MassFormula", "", kTextCenterX);
   fIDLineList->GetDataColumn(3)->SetIsBoolean();
   fIDLineList->ActivateSortButtons();
   fIDLineList->AllowBrowse(kFALSE);
   //fIDLineList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
   //                    "SelectionChanged()");
   line_frame->AddFrame(fIDLineList, new TGLayoutHints(kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 10));

   lab1 = new TGLabel(line_frame, "CUT LINES");
   line_frame->AddFrame(lab1, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 2, 2, 2, 2));

   fCUTLineList = new KVListView(KVIDCutLine::Class(), line_frame, 350, 150);
   fCUTLineList->SetDataColumns(3);
   fCUTLineList->SetDataColumn(0, "Name", "", kTextLeft);
   fCUTLineList->SetDataColumn(1, "# Points", "GetN", kTextCenterX);
   fCUTLineList->SetDataColumn(2, "Direction", "GetAcceptedDirection", kTextCenterX);
   fCUTLineList->ActivateSortButtons();
   fCUTLineList->AllowBrowse(kFALSE);
   //fIDLineList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
   //                    "SelectionChanged()");
   line_frame->AddFrame(fCUTLineList, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   lab1 = new TGLabel(line_frame, "CUT CONTOURS");
   line_frame->AddFrame(lab1, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 2, 2, 2, 2));

   fCUTContourList = new KVListView(KVIDCutContour::Class(), line_frame, 350, 150);
   fCUTContourList->SetDataColumns(3);
   fCUTContourList->SetDataColumn(0, "Name", "", kTextLeft);
   fCUTContourList->SetDataColumn(1, "# Points", "GetN", kTextCenterX);
   fCUTContourList->SetDataColumn(2, "Exclusive", "IsExclusive", kTextCenterX);
   fCUTContourList->GetDataColumn(2)->SetIsBoolean();
   fCUTContourList->ActivateSortButtons();
   fCUTContourList->AllowBrowse(kFALSE);
   //fIDLineList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
   //                    "SelectionChanged()");
   line_frame->AddFrame(fCUTContourList, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 10));

   fHframe->AddFrame(line_frame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandY, 20, 20, 20, 20));

   AddFrame(fHframe,
            new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0,
                              0));

   SetWindowName("ID Grid Manager");

   //layout & draw window
   MapSubwindows();
   Resize(GetDefaultSize());
   MapWindow();
   SetWMSize(1200, 600);

   // first tab is visible, but TabSelect(0) is not called automatically
   TabSelect(0);

}

KVIDGridManagerGUI::~KVIDGridManagerGUI()
{
   //close window
   UnmapWindow();
   gIDGridManager->Disconnect("Modified()", this, "UpdateListOfGrids()");
   if (fSelectedEntries) delete fSelectedEntries;
   fSelectedEntries = 0;
//   if(fIDGridEditor) fIDGridEditor->Close();
}

void KVIDGridManagerGUI::CloseWindow()
{
   //close viewer
   DeleteWindow();
}

void KVIDGridManagerGUI::StartEditor()
{
   if (!fIDGridEditor) fIDGridEditor = new KVIDGridEditor;
   if (fIDGridEditor->IsClosed()) fIDGridEditor->StartViewer();
   if (fSelectedGrid) {
      // avant d'editer la grille, on en fait une copie pour
      // pouvoir revenir en arriere
      fSelectedGrid->UpdateLastSavedVersion();
      fIDGridEditor->SetGrid(fSelectedGrid);
   }
}

void KVIDGridManagerGUI::StartEditor(TObject* o)
{
   Info("StartEditor(TObject*)", "obj=%p", o);
   StartEditor();
}

void KVIDGridManagerGUI::OpenRootFile()
{
   static TString dir("$HISTOROOT");
   TString currentdir(gSystem->ExpandPathName("."));

   if (gSystem->ExpandPathName(dir)) dir = ".";

   const char* filetypes[] = {"Root files", "*.root", "All files", "*", 0, 0};
   TGFileInfo fi;
   fi.fFileTypes = filetypes;
   fi.fIniDir = StrDup(dir);
   new TGFileDialog(fClient->GetDefaultRoot(), this, kFDOpen, &fi);
   if (fi.fFilename) {
      if (!(TFile::Open(fi.fFilename))) {
         new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "ID Grid Editor", Form("Could not open file %s", fi.fFilename), 0, kMBOk);
      }
   }
   dir = fi.fIniDir;
   gSystem->cd(currentdir.Data());
}

void KVIDGridManagerGUI::HandleGridsMenu(Int_t id)
{
   //Receive signals emitted by items selected in Grids menu
   switch (id) {

      case M_QUIT:

         Quit();
         break;

      case M_GRIDS_NEW:

         cout << "Add new grid" << endl;
         NewGrid();
         break;
      case M_GRIDS_READ:
         OpenFile();
         break;

      case M_GRIDS_SAVE_SEL:
         // save current selection of grids in file
         SaveGridsAs(fSelectedEntries);
         break;

      case M_GRIDS_SAVE_TAB:
         // save all grids in current tab in file
         SaveGridsAs(GetAllGridsInTab());
         break;

      case M_GRIDS_SAVE_ALL:

         //save all grids in file - ask user to confirm or change filename/path
         SaveGridsAs();
         break;

      case M_GRIDS_DEL_SEL:
         DeleteSelectedGrids();
         break;

      case M_GRIDS_DEL_TAB:
         //warning message and confirmation
         {
            Int_t ret_val;
            new TGMsgBox(fClient->GetDefaultRoot(), this, "ID Grid Manager",
                         "This will delete all grids in the current tab. Are you sure ?",
                         kMBIconExclamation, kMBOk | kMBCancel, &ret_val);
            if (ret_val & kMBOk) {
               DeleteAllGridsInTab();
            }
         }
         break;
      case M_GRIDS_DEL_ALL:

         //warning message and confirmation
         {
            Int_t ret_val;
            new TGMsgBox(fClient->GetDefaultRoot(), this, "ID Grid Manager",
                         "This will delete all grids. Are you sure ?",
                         kMBIconExclamation, kMBOk | kMBCancel, &ret_val);
            if (ret_val & kMBOk) {
               gIDGridManager->Clear();
               //UpdateListOfGrids();
               ///reset filename to avoid hitting Save and overwriting previous file
               //with an empty file !!
               fFileName = "";
            }
         }
         break;

      case M_GRIDS_SET_RUNLIST:
         // set runlist for all selected grids
         {
            if (!fSelectedGrid) break; // must have selected at least one grid
            TString runs = fSelectedGrid->GetRunList(); // fill dialog box with current runlist of selected grid
            Bool_t ok_pressed = kFALSE;
            new KVInputDialog(this, "Enter list of runs for grid(s):", &runs, &ok_pressed,
                              "Example: 1-10, 13, 22-657");
            if (!ok_pressed) break; // user pressed 'cancel' or otherwise closed the dialog
            TIter next(fSelectedEntries);
            KVIDGraph* entry;
            while ((entry = (KVIDGraph*) next())) {
               entry->SetRunList(runs.Data());
            }
         }
         break;
      case M_GRIDS_SET_VARX:
         // set varx for all selected grids
         {
            if (!fSelectedGrid) break; // must have selected at least one grid
            TString runs = fSelectedGrid->GetVarX(); // fill dialog box with current runlist of selected grid
            Bool_t ok_pressed = kFALSE;
            new KVInputDialog(this, "Enter X variable for grid(s):", &runs, &ok_pressed,
                              "");
            if (!ok_pressed) break; // user pressed 'cancel' or otherwise closed the dialog
            TIter next(fSelectedEntries);
            KVIDGraph* entry;
            while ((entry = (KVIDGraph*) next())) {
               entry->SetVarX(runs.Data());
            }
         }
         break;
      case M_GRIDS_SET_VARY:
         // set varx for all selected grids
         {
            if (!fSelectedGrid) break; // must have selected at least one grid
            TString runs = fSelectedGrid->GetVarY(); // fill dialog box with current runlist of selected grid
            Bool_t ok_pressed = kFALSE;
            new KVInputDialog(this, "Enter Y variable for grid(s):", &runs, &ok_pressed,
                              "");
            if (!ok_pressed) break; // user pressed 'cancel' or otherwise closed the dialog
            TIter next(fSelectedEntries);
            KVIDGraph* entry;
            while ((entry = (KVIDGraph*) next())) {
               entry->SetVarY(runs.Data());
            }
         }
         break;
      case M_GRIDS_SET_ZID:
         // set varx for all selected grids
         {
            if (!fSelectedGrid) break; // must have selected at least one grid
            Bool_t runs = !fSelectedGrid->HasMassIDCapability(); // fill dialog box with current runlist of selected grid
            TIter next(fSelectedEntries);
            KVIDGraph* entry;
            while ((entry = (KVIDGraph*) next())) {
               entry->SetOnlyZId(!runs);
            }
         }
         break;
      case M_GRIDS_CLEAR:
         // set varx for all selected grids
         {
            if (!fSelectedGrid) break; // must have selected at least one grid
            TIter next(fSelectedEntries);
            KVIDGraph* entry;
            while ((entry = (KVIDGraph*) next())) {
               entry->Clear();
            }
         }
         break;

      default:

         break;
   }
}

void KVIDGridManagerGUI::DeleteSelectedGrids()
{
   //warning message and confirmation
   Int_t ret_val;
   new TGMsgBox(fClient->GetDefaultRoot(), this, "ID Grid Manager",
                "This will delete the selected grids. Are you sure ?",
                kMBIconExclamation, kMBOk | kMBCancel, &ret_val);
   if (ret_val & kMBOk) {
      DeleteGrids();
   }
}

void KVIDGridManagerGUI::OpenFile()
{
   static TString dir(".");
   const char* filetypes[] = {
      "ID Grid files", "*.dat",
      "All files", "*",
      0, 0
   };
   TGFileInfo fi;
   fi.fFileTypes = filetypes;
   fi.fIniDir = StrDup(dir);
   //printf("fIniDir = %s\n", fi.fIniDir);
   new TGFileDialog(fClient->GetDefaultRoot(), this, kFDOpen, &fi);
   if (fi.fFilename) {
      int ngri = gIDGridManager->GetGrids()->GetEntries();
      bool ok = gMultiDetArray ? gMultiDetArray->ReadGridsFromAsciiFile(fi.fFilename)
                : gIDGridManager->ReadAsciiFile(fi.fFilename);
      if (ok) {
         UpdateListOfGrids();
         //read file ok no problem.
         int ngriread = gIDGridManager->GetGrids()->GetEntries() - ngri;
         SetStatus(Form("Read %d grids from file %s", ngriread, fi.fFilename));
         //set filename for Save
         fFileName = gSystem->BaseName(fi.fFilename);
      }
      else {
         new TGMsgBox(fClient->GetDefaultRoot(), this, "ID Grid Manager",
                      Form("Could not read file %s", fi.fFilename),
                      0, kMBOk);
      }
   }
   dir = fi.fIniDir;
}

void KVIDGridManagerGUI::UpdateListOfGrids()
{
   //remove all existing entries, then fill list from gIDGridManager
   //we update the current pad, in case the displayed grid no longer exists
   //grid buttons are disabled, as any selected grid is deselected
   //  cout << "DEBUG: KVIDGridManagerGUI::UpdateListOfGrids(): starting..." << endl;
   UpdateTabs();
   //  cout << "DEBUG: KVIDGridManagerGUI::UpdateListOfGrids(): tabs has been updated !" << endl;
   //update all canvases
//   TSeqCollection* Clist = gROOT->GetListOfCanvases();
//   if (Clist && Clist->GetEntries()) {
//      Clist->R__FOR_EACH(TCanvas, Modified)();
//      Clist->R__FOR_EACH(TCanvas, Update)();
//   }
   //  cout << "DEBUG: KVIDGridManagerGUI::UpdateListOfGrids(): canvas has been updated !" << endl;
}

void KVIDGridManagerGUI::SelectionChanged()
{
   //called any time the selection of grids changes
   //each time, we update:
   //fSelectedEntries : the list of all selected entries
   //GetNSelected() : the number of selected entries
   //fSelectedGrid : the last selected grid (=the only grid selected if GeTNSelected==1)

   //get number of selected items
   if (fSelectedEntries)
      delete fSelectedEntries;
   fSelectedEntries = fIDGridList->GetSelectedObjects();
   fSelectedGrid = (KVIDGraph*)fIDGridList->GetLastSelectedObject();
   if (!GetNSelected())fSelectedGrid = 0x0;
   ShowListOfLines();
   //(de)activate toolbar buttons
   ActivateToolbarButtons();
   if (!GetNSelected()) SetStatus();
   else if (GetNSelected() == 1) SetStatus(Form("Selected grid %s (%s)", fSelectedGrid->GetName(), fSelectedGrid->ClassName()));
   else SetStatus(Form("Selected %d grids, last selected grid %s (%s)", GetNSelected(), fSelectedGrid->GetName(), fSelectedGrid->ClassName()));
}

Int_t KVIDGridManagerGUI::GetNSelected()
{
   //returns current number of selected items in grid list
   return (fSelectedEntries ? fSelectedEntries->GetSize() : 0);
}

void KVIDGridManagerGUI::DeleteGrids()
{
   //delete the current selected grid, or all currently selected grids if there are more than one

   if (!GetNSelected()) return;

   TIter next(fSelectedEntries, kIterBackward);
   KVIDGraph* entry;
   while ((entry = (KVIDGraph*) next())) {
      //  cout << "DEBUG: KVIDGridManagerGUI::DeleteGrids(): deleting grid '" << entry->GetName() << "' !" << endl;
      if (fLastSelectedGrid == entry) fLastSelectedGrid = 0;
      gIDGridManager->DeleteGrid(entry, kFALSE);   //no update
      //  cout << "DEBUG: KVIDGridManagerGUI::DeleteGrids(): grid has been deleted !" << endl;
   }
   if (fSelectedEntries) delete fSelectedEntries;
   //  cout << "DEBUG: KVIDGridManagerGUI::DeleteGrids(): list of selected grids has been deleted !" << endl;
   fSelectedEntries = 0;
   fSelectedGrid = 0;
   UpdateListOfGrids();
   //  cout << "DEBUG: KVIDGridManagerGUI::DeleteGrids(): list of grids has been updated !" << endl;
}

void KVIDGridManagerGUI::DeleteAllGridsInTab()
{
   //delete the all grids in currently selected tab

   TIter next(GetAllGridsInTab(), kIterBackward);
   KVIDGraph* entry;
   while ((entry = (KVIDGraph*) next())) {
      gIDGridManager->DeleteGrid(entry, kFALSE);   //no update
   }
   if (fSelectedEntries) delete fSelectedEntries;
   fSelectedEntries = 0;
   fSelectedGrid = 0;
   UpdateListOfGrids();
}

void KVIDGridManagerGUI::SetIDTelescopes()
{
   // Set ID telescopes for selected grid
   Bool_t cancel;
   TList telescopes;
   telescopes.AddAll(fSelectedGrid->GetIDTelescopes());
   new KVIDGUITelescopeChooserDialog(gMultiDetArray, &telescopes, &cancel,
                                     fClient->GetDefaultRoot(), this, kTRUE);
   if (cancel || !telescopes.GetEntries()) return;
   fSelectedGrid->ClearListOfTelescopes();
   fSelectedGrid->AddIDTelescopes(&telescopes);
}

void KVIDGridManagerGUI::ClearGrid()
{
   //deletes all lines in currently selected grid(s)

   if (GetNSelected() == 1)
      fSelectedGrid->Clear();
   else if (GetNSelected() > 1) {
      //multiselection
      TIter next(fSelectedEntries);
      KVIDGraph* grid;
      while ((grid = (KVIDGraph*) next())) {
         grid->Clear();
      }
   }
   //update all canvases
   TSeqCollection* Clist = gROOT->GetListOfCanvases();
   if (Clist->GetSize() > 0) {
      Clist->R__FOR_EACH(TCanvas, Modified)();
      Clist->R__FOR_EACH(TCanvas, Update)();
   }
}

void KVIDGridManagerGUI::CopyGrid()
{
   if (!fSelectedGrid) return;
   gIDGridManager->Disconnect("Modified()", this, "UpdateListOfGrids()");
   KVIDGraph* new_gr = KVIDGraph::MakeIDGraph(fSelectedGrid->ClassName());
   fSelectedGrid->Copy(*new_gr);
   new_gr->AddIDTelescopes(fSelectedGrid->GetIDTelescopes());
   new_gr->SetRunList("");
   gIDGridManager->GetGrids()->Sort();
   UpdateTabs();
   gIDGridManager->Connect("Modified()", "KVIDGridManagerGUI", this, "UpdateListOfGrids()");
   fIDGridList->Sort(0);
   fIDGridList->Sort(0);
   return;
}

void KVIDGridManagerGUI::NewGrid()
{
   // Create a new identification grid.
   // First we ask the user to select the identification telescope(s)
   // for which this grid will be used.
   // For a given type of ID telescope, several types of grid may be
   // applicable. If so, we ask the user to choose one.

   Info("NewGrid", "There are %d grids in the manager", gIDGridManager->GetGrids()->GetSize());
   TString default_class = "KVIDZAGrid";
   TList* telescopes = new TList;
   if (gMultiDetArray) {
      Bool_t cancel;
      new KVIDGUITelescopeChooserDialog(gMultiDetArray, telescopes, &cancel,
                                        fClient->GetDefaultRoot(), this);
      if (cancel || !telescopes->At(0)) {
         Info("NewGrid", "No ID telescopes chosen. Grid creation cancelled.");
         return;
      }
      // get default ID grid class of first ID telescope
      default_class = ((KVIDTelescope*)telescopes->At(0))->GetDefaultIDGridClass();
   }
   // get list of possible choices of grid class = list of all plugin classes
   // defined for KVIDGraph
   TString choice = KVBase::GetListOfPlugins("KVIDGraph");
   // open dialog to choose ID grid class
   Bool_t ok_pressed = kFALSE;
   TString id_grid_class;
   new KVDropDownDialog(this, "Choose class for new grid:",
                        choice.Data(), default_class.Data(), &id_grid_class, &ok_pressed);
   if (!ok_pressed) {
      Info("NewGrid", "No ID grid class chosen. Grid creation cancelled.");
      return;
   }
   gIDGridManager->Disconnect("Modified()", this, "UpdateListOfGrids()");
   KVIDGraph* new_gr = KVIDGraph::MakeIDGraph(id_grid_class.Data());
   if (telescopes->GetEntries()) new_gr->AddIDTelescopes(telescopes);
   UpdateTabs();
   gIDGridManager->Connect("Modified()", "KVIDGridManagerGUI", this, "UpdateListOfGrids()");
   delete telescopes;
}

void KVIDGridManagerGUI::MergeGrids()
{
   //merge 2 grids
   //KVIDGraph *g1 = (KVIDGraph *) fSelectedEntries->At(1);
   //KVIDGraph *g2 = (KVIDGraph *) fSelectedEntries->First();
   //new KVMergeGridsDialog(g1, g2, fClient->GetDefaultRoot(), this, 10, 10);
}

void KVIDGridManagerGUI::SaveGridsAs(const TCollection* selection)
{
   // Opens dialog to choose filename in which to save grids.
   // If selection=0 (default), all grids are saved
   // If selection!=0 only grids in list are saved

   static TString dir(".");
   const char* filetypes[] = {
      "ID Grid files", "*.dat",
      "All files", "*",
      0, 0
   };
   TGFileInfo fi;
   fi.fFileTypes = filetypes;
   fi.fIniDir = StrDup(dir);
   if (fFileName != "") fi.fFilename = StrDup(fFileName);
   new TGFileDialog(fClient->GetDefaultRoot(), this, kFDSave, &fi);
   if (fi.fFilename) {
      //if no ".xxx" ending given, we add ".dat"
      TString filenam(fi.fFilename);
      if (!filenam.Contains('.'))
         filenam += ".dat";
      Int_t n_saved = gIDGridManager->WriteAsciiFile(filenam.Data(), selection);
      if (n_saved) {
         //wrote file no problem
         SetStatus(Form("Saved %d grids in file %s", n_saved, filenam.Data()));
         //set file name for Save
         fFileName = gSystem->BaseName(filenam);
      }
      else {
         new TGMsgBox(fClient->GetDefaultRoot(), this, "ID Grid Manager",
                      Form("Could not write file %s", filenam.Data()), 0,
                      kMBOk);
      }
   }
   dir = fi.fIniDir;
}

void KVIDGridManagerGUI::CreateAndFillTabs()
{
   // create a tab for each type of ID telescope
   // put a list box for ID grid names on each tab

   KVString labels;
   if (gIDGridManager->GetGrids()->GetSize()) gIDGridManager->GetListOfIDTelescopeLabels(labels);
   if (labels == "") {
      // no known idtelescopes referenced by grids (maybe we don't have a KVMultiDetArray?)
      // make 1 tab "Grids" and put them all in
      KVString lab = "Grids";
      TGCompositeFrame* cf = fGridListTabs->AddTab(lab.Data());
      cf->ChangeOptions(kVerticalFrame);
      fIDGridList = new KVListView(KVIDGraph::Class(), cf, 600, 400);
      fIDGridList->SetDataColumns(10);
      fIDGridList->SetDataColumn(0, "Name", "", kTextLeft);
      fIDGridList->SetDataColumn(2, "VarX", "", kTextLeft);
      fIDGridList->SetDataColumn(1, "VarY", "", kTextLeft);
      fIDGridList->SetDataColumn(3, "ID Telescopes", "GetNamesOfIDTelescopes", kTextLeft);
      fIDGridList->SetDataColumn(4, "RunList", "", kTextLeft);
      fIDGridList->SetDataColumn(5, "Identify Z and A?", "HasMassIDCapability", kTextCenterX);
      fIDGridList->GetDataColumn(5)->SetIsBoolean();
      fIDGridList->SetDataColumn(6, "# Ident.", "GetNumberOfIdentifiers", kTextRight);
      fIDGridList->SetDataColumn(7, "# Cuts", "GetNumberOfCuts", kTextRight);
      fIDGridList->SetDataColumn(8, "X scaling", "GetXScaleFactor", kTextRight);
      fIDGridList->SetDataColumn(9, "Y scaling", "GetYScaleFactor", kTextRight);
      fIDGridList->ActivateSortButtons();
      fIDGridList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
                           "SelectionChanged()");
      fIDGridList->SetDoubleClickAction("KVIDGridManagerGUI", this, "StartEditor(TObject*)");
      cf->AddFrame(fIDGridList, new TGLayoutHints(kLHintsLeft | kLHintsTop |
                   kLHintsExpandX | kLHintsExpandY, 30,
                   10, 10, 10));
      KVList* grids = gIDGridManager->GetGrids();
      fIDGridList->Display(grids);
      return;
   }
   //loop over labels
   labels.Begin(",");
   while (! labels.End()) {
      KVString lab = labels.Next();
      TGCompositeFrame* cf = fGridListTabs->AddTab(lab.Data());
      cf->ChangeOptions(kVerticalFrame);
      fIDGridList = new KVListView(KVIDGraph::Class(), cf, 600, 400);
      fIDGridList->SetDataColumns(10);
      fIDGridList->SetDataColumn(0, "Name", "", kTextLeft);
      fIDGridList->SetDataColumn(2, "VarX", "", kTextLeft);
      fIDGridList->SetDataColumn(1, "VarY", "", kTextLeft);
      fIDGridList->SetDataColumn(3, "ID Telescopes", "GetNamesOfIDTelescopes", kTextLeft);
      fIDGridList->SetDataColumn(4, "RunList", "", kTextLeft);
      fIDGridList->SetDataColumn(5, "Identify Z and A?", "HasMassIDCapability", kTextCenterX);
      fIDGridList->GetDataColumn(5)->SetIsBoolean();
      fIDGridList->SetDataColumn(6, "# Ident.", "GetNumberOfIdentifiers", kTextRight);
      fIDGridList->SetDataColumn(7, "# Cuts", "GetNumberOfCuts", kTextRight);
      fIDGridList->SetDataColumn(8, "X scaling", "GetXScaleFactor", kTextRight);
      fIDGridList->SetDataColumn(9, "Y scaling", "GetYScaleFactor", kTextRight);
      fIDGridList->ActivateSortButtons();
      fIDGridList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
                           "SelectionChanged()");
      fIDGridList->SetDoubleClickAction("KVIDGridManagerGUI", this, "StartEditor(TObject*)");
      cf->AddFrame(fIDGridList, new TGLayoutHints(kLHintsLeft | kLHintsTop |
                   kLHintsExpandX | kLHintsExpandY, 30,
                   10, 10, 10));
      KVList* grids = gIDGridManager->GetGridsForIDTelescope(lab);
      fIDGridList->Display(grids);
      delete grids;
   }

}

void KVIDGridManagerGUI::TabSelect(Int_t)
{
   //called when a new tab is selected

   TGCompositeFrame* cf = fGridListTabs->GetCurrentContainer();
   if (!cf) return;//there are no tabs
   TGFrameElement* el = (TGFrameElement*)cf->GetList()->At(0);
   fIDGridList = (KVListView*)el->fFrame;
   fIDGridList->SelectionChanged();
}

void KVIDGridManagerGUI::UpdateTabs()
{
   // create a tab for each type of ID telescope
   // put a list box for ID grid names on each tab
   //  cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : starting..." << endl;

   KVString labels("");
   if (gIDGridManager->GetGrids()->GetSize()) gIDGridManager->GetListOfIDTelescopeLabels(labels);
   else {
      // there are no grids in the grid manager
      RemoveEmptyTabs();
      KVString lab = "Grids";
      TGCompositeFrame* cf = fGridListTabs->AddTab(lab.Data());
      cf->ChangeOptions(kVerticalFrame);
      fIDGridList = new KVListView(KVIDGraph::Class(), cf, 600, 400);
      fIDGridList->SetDataColumns(10);
      fIDGridList->SetDataColumn(0, "Name", "", kTextLeft);
      fIDGridList->SetDataColumn(2, "VarX", "", kTextLeft);
      fIDGridList->SetDataColumn(1, "VarY", "", kTextLeft);
      fIDGridList->SetDataColumn(3, "ID Telescopes", "GetNamesOfIDTelescopes", kTextLeft);
      fIDGridList->SetDataColumn(4, "RunList", "", kTextLeft);
      fIDGridList->SetDataColumn(5, "Identify Z and A?", "HasMassIDCapability", kTextCenterX);
      fIDGridList->GetDataColumn(5)->SetIsBoolean();
      fIDGridList->SetDataColumn(6, "# Ident.", "GetNumberOfIdentifiers", kTextRight);
      fIDGridList->SetDataColumn(7, "# Cuts", "GetNumberOfCuts", kTextRight);
      fIDGridList->SetDataColumn(8, "X scaling", "GetXScaleFactor", kTextRight);
      fIDGridList->SetDataColumn(9, "Y scaling", "GetYScaleFactor", kTextRight);
      fIDGridList->ActivateSortButtons();
      fIDGridList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
                           "SelectionChanged()");
      cf->AddFrame(fIDGridList, new TGLayoutHints(kLHintsLeft | kLHintsTop |
                   kLHintsExpandX | kLHintsExpandY, 30,
                   10, 10, 10));
      fGridListTabs->MapSubwindows();
      fGridListTabs->Layout();
      Int_t ntabs = fGridListTabs->GetCurrent();
      TabSelect(ntabs);
      return;
   }
   if (labels == "") {
      // no known idtelescopes referenced by grids (maybe we don't have a KVMultiDetArray?)
      // update "Grids" tab
      TGCompositeFrame* cf = fGridListTabs->GetTabContainer("Grids");
      if (!cf) {
         cout << "cf = 0x0 : label=Grids tab name=" <<
              fGridListTabs->GetTabTab("Grids")->GetText()->GetString() << endl;
      }
      else {
         TGFrameElement* el = (TGFrameElement*)cf->GetList()->At(0);
         fIDGridList = (KVListView*)el->fFrame;
         KVList* grids = gIDGridManager->GetGrids();
         fIDGridList->Display(grids);
      }
      //make sure we are on the right tab
      Int_t ntabs = fGridListTabs->GetCurrent();
      TabSelect(ntabs);
      return;
   }
   //add any missing labels, update existing ones
   labels.Begin(",");
   while (! labels.End()) {
      KVString lab = labels.Next();
      //  cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : updating tab '" << lab.Data() << "'..." << endl;
      if (!fGridListTabs->GetTabContainer(lab.Data())) { // new tab
         //  cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : new tab '" << lab.Data() << "'..." << endl;
         TGCompositeFrame* cf = fGridListTabs->AddTab(lab.Data());
         cf->ChangeOptions(kVerticalFrame);
         fIDGridList = new KVListView(KVIDGraph::Class(), cf, 600, 400);
         fIDGridList->SetDataColumns(10);
         fIDGridList->SetDataColumn(0, "Name", "", kTextLeft);
         fIDGridList->SetDataColumn(2, "VarX", "", kTextLeft);
         fIDGridList->SetDataColumn(1, "VarY", "", kTextLeft);
         fIDGridList->SetDataColumn(3, "ID Telescopes", "GetNamesOfIDTelescopes", kTextLeft);
         fIDGridList->SetDataColumn(4, "RunList", "", kTextLeft);
         fIDGridList->SetDataColumn(5, "Identify Z and A?", "HasMassIDCapability", kTextCenterX);
         fIDGridList->GetDataColumn(5)->SetIsBoolean();
         fIDGridList->SetDataColumn(6, "# Ident.", "GetNumberOfIdentifiers", kTextRight);
         fIDGridList->SetDataColumn(7, "# Cuts", "GetNumberOfCuts", kTextRight);
         fIDGridList->SetDataColumn(8, "X scaling", "GetXScaleFactor", kTextRight);
         fIDGridList->SetDataColumn(9, "Y scaling", "GetYScaleFactor", kTextRight);
         fIDGridList->ActivateSortButtons();
         fIDGridList->Connect("SelectionChanged()", "KVIDGridManagerGUI", this,
                              "SelectionChanged()");
         cf->AddFrame(fIDGridList, new TGLayoutHints(kLHintsLeft | kLHintsTop |
                      kLHintsExpandX | kLHintsExpandY, 30,
                      10, 10, 10));
         KVList* grids = gIDGridManager->GetGridsForIDTelescope(lab);
         fIDGridList->Display(grids);
         delete grids;
         fGridListTabs->MapSubwindows();
         fGridListTabs->Layout();
         fGridListTabs->SetTab(fGridListTabs->GetNumberOfTabs() - 1, kTRUE);
      }
      else {   //existing tab
         //cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : existing tab '" << lab.Data() << "'..." << endl;
         TGCompositeFrame* cf = fGridListTabs->GetTabContainer(lab.Data());
         if (!cf) {
            cout << "cf = 0x0 : label=" << lab.Data() << " tab name=" <<
                 fGridListTabs->GetTabTab(lab.Data())->GetText()->GetString() << endl;
         }
         else {
            //cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : recup tab '" << cf->GetName() << "'..." << endl;
            TGFrameElement* el = (TGFrameElement*)cf->GetList()->At(0);
            //cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : recup element '" << el->GetName() << "'..." << endl;
            fIDGridList = (KVListView*)el->fFrame;
            //cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : recup view list '" << fIDGridList->GetName() << "'..." << endl;
            KVList* grids = gIDGridManager->GetGridsForIDTelescope(lab);
            //cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : recup list de telescope'" << grids->GetName() << "'..." << endl;
            //grids->ls();
            fIDGridList->Display(grids);
            //cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : display list of grids in the viewerlist..." << endl;

            if (grids) delete grids;
            //  cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : deleting the list..." << endl;
         }
      }
   }
   //now check that none of the remaining tabs are empty & should be removed
   RemoveEmptyTabs();
   //  cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : empty tabs removed !" << endl;
   //make sure we are on the right tab
   Int_t ntabs = fGridListTabs->GetCurrent();
   TabSelect(ntabs);
   //  cout << "DEBUG: KVIDGridManagerGUI::UpdateTabs() : current tab selected !" << endl;
}

void KVIDGridManagerGUI::RemoveEmptyTabs()
{
   // Recursively remove any empty tabs

   Int_t ntabs = fGridListTabs->GetNumberOfTabs();
   Bool_t recursive = kFALSE;
   for (Int_t itab = 0; itab < ntabs; itab++) {

      //get name of tab
      KVString lab = fGridListTabs->GetTabTab(itab)->GetString();
      //get grids for this tab (if any)
      KVList* grids = gIDGridManager->GetGridsForIDTelescope(lab);
      Int_t ngrids = grids->GetEntries();
      delete grids;
      if (!ngrids) {
         //empty tab! remove it!
         //delete the KVListView
         TGCompositeFrame* cf = fGridListTabs->GetTabContainer(itab);
         TGFrameElement* el = (TGFrameElement*)cf->GetList()->At(0);
         KVListView* lv = (KVListView*)el->fFrame;
         delete lv;
         //remove tab
         fGridListTabs->RemoveTab(itab, kFALSE);
         recursive = kTRUE; // call recursively
         break;//stop loop - tab numbers have changed
      }
   }
   if (recursive) RemoveEmptyTabs();
   fGridListTabs->MapSubwindows();
   fGridListTabs->Layout();
}

void KVIDGridManagerGUI::UpdateListOfLines()
{
   // called when a previously selected grid is modified
   // updates lists of lines

   if (!fLastSelectedGrid) return;

   KVList* ids = fLastSelectedGrid->GetIdentifiers();
   // sort lines in order of increasing Z
   ids->Sort();
   fIDLineList->Display(ids);
   KVSeqCollection* cutlines = fSelectedGrid->GetCuts()->GetSubListWithClass("KVIDCutLine");
   fCUTLineList->Display(cutlines);
   delete cutlines;
   cutlines = fSelectedGrid->GetCuts()->GetSubListWithClass("KVIDCutContour");
   fCUTContourList->Display(cutlines);
   delete cutlines;
}

void KVIDGridManagerGUI::ShowListOfLines()
{
   // Called when a grid is selected in list of grids
   // We fill list of all lines in grid
   //If only one grid is selected, we display its lines in the line list
   //If more than one grid is selected, we clear the line list

   if (GetNSelected() == 1 && fSelectedGrid) {
      KVList* ids = fSelectedGrid->GetIdentifiers();
      // sort lines in order of increasing Z
      ids->Sort();
      fIDLineList->Display(ids);
      KVSeqCollection* cutlines = fSelectedGrid->GetCuts()->GetSubListWithClass("KVIDCutLine");
      fCUTLineList->Display(cutlines);
      delete cutlines;
      cutlines = fSelectedGrid->GetCuts()->GetSubListWithClass("KVIDCutContour");
      fCUTContourList->Display(cutlines);
      delete cutlines;
      if (fLastSelectedGrid) {
         fLastSelectedGrid->Disconnect("Modified()", this, "UpdateListOfLines()");
      }
      fSelectedGrid->Connect("Modified()", "KVIDGridManagerGUI", this, "UpdateListOfLines()");
      fLastSelectedGrid = fSelectedGrid;
   }
   else {
      fIDLineList->RemoveAll();
      fCUTLineList->RemoveAll();
      fCUTContourList->RemoveAll();
      if (fLastSelectedGrid) {
         fLastSelectedGrid->Disconnect("Modified()", this, "UpdateListOfLines()");
         fLastSelectedGrid = 0;
      }
   }
}

void KVIDGridManagerGUI::ActivateToolbarButtons()
{
   // disable all buttons
   for (int i = 0; i < fNbButtons; i++) fTBbuttons[i]->SetEnabled(kFALSE);
   // enable 'open' & 'quit' & 'new grid"
   fTBOpen->SetEnabled();
   fTBRefresh->SetEnabled();
//   fTBStartEditor->SetEnabled();
   fTBNewG->SetEnabled();
   fTBOpenRoot->SetEnabled();
   // enable 'save' if there are grids
   if (gIDGridManager->GetGrids()->GetEntries()) fTBSave->SetEnabled();

   if (!GetNSelected()) return; // no grids selected

   //enable delete selected grid(s)
   fTBDelG->SetEnabled();

   if (GetNSelected() == 1) {
      // only one grid selected
      fTBStartEditor->SetEnabled();
      fTBCopyG->SetEnabled();
      if (gMultiDetArray) fTBSetIDG->SetEnabled();
   }
}

void KVIDGridManagerGUI::NewIDLine()
{
   // GUI method to draw a new identifier and add it to graph.
   // A dialog box with drop-down list pops up for the user to choose the class of the
   // new identifier, unless only one choice is possible, in which case it is used automatically.
   // For each KVIDGraph-derived class, the list of possible identifier classes and the
   // default class are define in .kvrootrc by the variables:
   //
   // [class_name].IDClass:  [id class 1]
   // +[class_name].IDClass:  [id class 2]
   // + ...
   // [class_name].DefaultIDClass:  [id class]

   if (!fSelectedGrid || GetNSelected() != 1) return;
   TString resname;
   resname.Form("%s.IDClass", fSelectedGrid->ClassName());
   TString cut_choices = gEnv->GetValue(resname.Data(), "");
   resname.Form("%s.DefaultIDClass", fSelectedGrid->ClassName());
   TString cut_default = gEnv->GetValue(resname.Data(), "");
   TString cut_class;
   Bool_t okpressed;
   if (cut_choices.Contains(" ")) {
      new KVDropDownDialog(this,
                           "Choose class of new identifier :",
                           cut_choices.Data(),
                           cut_default.Data(),
                           &cut_class,
                           &okpressed);
      if (!okpressed) return;
   }
   else
      cut_class = cut_choices;
   SetStatus(Form("Draw ID line (%s) in current pad", cut_class.Data()));
   fSelectedGrid->DrawAndAdd("ID", cut_class.Data());
   fTBNewIDL->SetDown(kFALSE, kFALSE);
   SetStatus();
}

void KVIDGridManagerGUI::NewCut()
{
   // GUI method to draw a new cut and add it to graph.
   // A dialog box with drop-down list pops up for the user to choose the class of the
   // new cut, unless only one choice is possible, in which case it is used automatically.
   // For each KVIDGraph-derived class, the list of possible cut classes and the
   // default class are define in .kvrootrc by the variables:
   //
   // [class_name].CutClass:  [cut class 1]
   // +[class_name].CutClass:  [cut class 2]
   // + ...
   // [class_name].DefaultCutClass:  [cut class]

   if (!fSelectedGrid || GetNSelected() != 1) return;
   TString resname;
   resname.Form("%s.CutClass", fSelectedGrid->ClassName());
   TString cut_choices = gEnv->GetValue(resname.Data(), "");
   resname.Form("%s.DefaultCutClass", fSelectedGrid->ClassName());
   TString cut_default = gEnv->GetValue(resname.Data(), "");
   TString cut_class;
   TString cut_types = cut_choices;
   cut_types.ReplaceAll("KVIDCut", "");
   Bool_t okpressed;
   if (cut_choices.Contains(" ")) {
      new KVDropDownDialog(this,
                           "Choose class of new cut :",
                           cut_types.Data(),
                           cut_default.Data(),
                           &cut_class,
                           &okpressed);
      if (!okpressed) return;
   }
   else
      cut_class = cut_types;
   SetStatus(Form("Draw cut %s in current pad", cut_class.Data()));
   cut_class.Prepend("KVIDCut");
   fSelectedGrid->DrawAndAdd("CUT", cut_class.Data());
   fTBNewCut->SetDown(kFALSE, kFALSE);
   SetStatus();
}

//___________________________________________________________________________________

void KVIDGridManagerGUI::TestGrid()
{
   // test the identification with selected grid
   // we search in current pad for the data histogram

   SetStatus("");
   if (GetNSelected() != 1) return;
   if (!fSelectedGrid) return;
   // look for data histogram in current pad
   TH2* histo = 0;
   if (gPad) {
      TIter next(gPad->GetListOfPrimitives());
      TObject* o;
      while ((o = next())) {
         if (o->InheritsFrom("TH2")) {
            histo = (TH2*)o;
            break;
         }
      }
   }
   if (!histo) {
      SetStatus("No TH2 found in current pad. Select pad containing 2D histo with data to identify.");
      return;
   }
   else {
      SetStatus(Form("Test identification of data in current pad %s.", histo->GetName()));
   }
   new KVTestIDGridDialog(fClient->GetDefaultRoot(), this, 10, 10, fSelectedGrid, histo);
   SetStatus("");
}

void KVIDGridManagerGUI::TestTreeGrid()
{
   // test the identification with selected grid
   // we search in current pad for the data histogram
   SetStatus("");
   if (GetNSelected() != 1) return;
   if (!fSelectedGrid) return;
   // look for data histogram in current pad
   TH2* histo = 0;
   if (gPad) {
      TIter next(gPad->GetListOfPrimitives());
      TObject* o;
      while ((o = next())) {
         if (o->InheritsFrom("TH2")) {
            histo = (TH2*)o;
            break;
         }
      }
   }
   if (!histo) {
      SetStatus("No TH2 found in current pad. Select pad containing 2D histo with data to identify.");
      return;
   }
   else {
      SetStatus(Form("Test identification of data in current pad %s.", histo->GetName()));
   }
   TFile* tmpfiles = TestIdentificationWithTree(fSelectedGrid, histo->GetName());
   if (!tmpfiles) {
      SetStatus("There was a problem with the test ???");
      return;
   }
   new TCanvas;
   TH2* id = (TH2*)tmpfiles->Get("idcode_map");
   id->SetStats(kFALSE);
   id->Draw("zcol");
   TTree* t = (TTree*)tmpfiles->Get("tree_idresults");
   t->StartViewer();
   SetStatus("");
}

//___________________________________________________________________________________

TFile* KVIDGridManagerGUI::TestIdentificationWithTree(KVIDGraph* gr, const Char_t* name_of_data_histo)
{
   //This method allows to test the identification capabilities of the grid using data in a TH2F.
   //We assume that 'data' contains an identification map, whose 'x' and 'y' coordinates correspond
   //to this grid. Then we loop over every bin of the histogram, perform the identification (if
   //IsIdentifiable() returns kTRUE) and fill the two histograms with the resulting identification
   //and its dependence on the 'residual energy' i.e. the 'x'-coordinate of the 'data' histogram,
   //each identification weighted by the contents of the original data bin.
   //
   //The 'identification" we represent is the result of the KVReconstructedNucleus::GetPID() method.
   //For particles identified in Z only, this is the "real Z".
   //For particles with A & Z identification, this is Z + 0.1*(A - 2*Z)

   //Initialize the grid: calculate line widths etc.
   gr->Initialize();

   TH2F* data = (TH2F*)gROOT->FindObject(name_of_data_histo);
   if (!data) {
      printf(" KVIDGraph::TestIdentificationWithTree l histo %s n existe pas\n", name_of_data_histo);
      return 0;
   }


   KVIdentificationResult* idr = new KVIdentificationResult;
   KVReconstructedNucleus nuc;

   // store current memory directory
   TDirectory* CWD = gDirectory;

   TTree* tid = 0;
   if ((tid = (TTree*)gROOT->FindObject("tree_idresults"))) {
      printf(" KVIDGraph::TestIdentificationWithTree effacemenent de l arbre existant\n");
      delete tid;
   }
   // create temporary file for tree
   TString fn("IDtestTree.root");
   KVBase::GetTempFileName(fn);
   TFile* tmpfile = new TFile(fn.Data(), "recreate");
   TH2F* idmap = (TH2F*)data->Clone("idcode_map");
   idmap->Reset();
   tid = new TTree("tree_idresults", "pid");
   Float_t br_xxx, br_yyy, br_stat, br_pid;
   Int_t br_idcode, br_isid;

   tid->Branch("X", &br_xxx, "br_xxx/F");
   tid->Branch("Y", &br_yyy, "br_yyy/F");
   tid->Branch("Stat", &br_stat, "br_stat/F");

   tid->Branch("PID", &br_pid, "br_pid/F");
   tid->Branch("IDcode", &br_idcode, "br_idcode/I");
   tid->Branch("IsIdentified", &br_isid, "br_isid/I");

   Int_t tot_events = (Int_t) data->GetSum();
   Int_t events_read = 0;
   Float_t percent = 0., cumul = 10.;

   //loop over data in histo
   for (int i = 1; i <= data->GetNbinsX(); i++) {
      for (int j = 1; j <= data->GetNbinsY(); j++) {

         Stat_t poids = data->GetBinContent(i, j);
         if (poids == 0)
            continue;
         br_stat = Float_t(poids);

         Axis_t x0 = data->GetXaxis()->GetBinCenter(i);
         Axis_t y0 = data->GetYaxis()->GetBinCenter(j);
         Axis_t wx = data->GetXaxis()->GetBinWidth(i);
         Axis_t wy = data->GetYaxis()->GetBinWidth(j);

         br_xxx = Float_t(x0);
         br_yyy = Float_t(y0);
         //If bin content ('poids') is <=20, we perform the identification 'poids' times, each time with
         //randomly-drawn x and y coordinates inside this bin
         //If 'poids'>20, we perform the identification 20 times and we fill the histograms with
         //a weight poids/20
         Double_t x, y;
         Int_t kmax = (Int_t) TMath::Min(20., poids);
         //Double_t weight = (kmax == 20 ? poids / 20. : 1.);

         for (int k = 0; k < kmax; k++) {

            x = gRandom->Uniform(x0 - .5 * wx, x0 + .5 * wx);
            y = gRandom->Uniform(y0 - .5 * wy, y0 + .5 * wy);
            if (gr->IsIdentifiable(x, y)) {
               br_isid = 1;
               gr->Identify(x, y, idr);
               nuc.SetIdentification(idr);
               br_pid = nuc.GetPID();
               br_idcode = gr->GetQualityCode();
               idmap->SetBinContent(i, j, br_idcode);
            }
            else {
               br_isid = 0;
               br_pid = -1;
               br_idcode = -1;
               idmap->SetBinContent(i, j, br_idcode);
            }
            tid->Fill();
         }
         events_read += (Int_t) poids;
         percent = (1. * events_read / tot_events) * 100.;
         gr->Increment((Float_t) events_read);      //sends signal to GUI progress bar
         if (percent >= cumul) {
            cout << (Int_t) percent << "\% processed" << endl;
            cumul += 10;
         }
         //gSystem->ProcessEvents();
      }
   }

   delete idr;
   CWD->cd();
   return tmpfile;
}

void KVIDGridManagerGUI::FitGrid()
{
   SetStatus("");
   if (GetNSelected() != 1) return;
   if (!fSelectedGrid) return;
   SetStatus(Form("Fitting grid %s", fSelectedGrid->GetName()));
   KVVirtualIDFitter* fitter = KVVirtualIDFitter::GetDefaultFitter();
   fitter->SetGrid(fSelectedGrid);
   fitter->SetPad(fSelectedGrid->GetPad());
   TMethod* m = fitter->IsA()->GetMethodAny("FitPanel");
   TContextMenu* cm = new TContextMenu("FitPanel", "Context menu for KVVirtualIDFitter::FitPanel");
   cm->Action(fitter, m);
   delete cm;
}
