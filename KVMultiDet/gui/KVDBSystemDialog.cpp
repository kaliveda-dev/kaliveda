/*
$Id: KVDBSystemDialog.cpp,v 1.8 2009/01/16 14:55:20 franklan Exp $
$Revision: 1.8 $
$Date: 2009/01/16 14:55:20 $
*/

//Created by KVClassFactory on Wed Apr  4 12:10:28 2007
//Author: franklan

#include "KVDBSystemDialog.h"
#ifndef ROOT_TGDockableFrame
#include "TGDockableFrame.h"
#endif
#ifndef ROOT_TGMenu
#include "TGMenu.h"
#endif
#ifndef ROOT_TGMdiDecorFrame
#include "TGMdiDecorFrame.h"
#endif
#ifndef ROOT_TG3DLine
#include "TG3DLine.h"
#endif
#ifndef ROOT_TGMdiFrame
#include "TGMdiFrame.h"
#endif
#ifndef ROOT_TGMdiMainFrame
#include "TGMdiMainFrame.h"
#endif
#ifndef ROOT_TGMdiMenu
#include "TGMdiMenu.h"
#endif
#ifndef ROOT_TGListBox
#include "TGListBox.h"
#endif
#ifndef ROOT_TGNumberEntry
#include "TGNumberEntry.h"
#endif
#ifndef ROOT_TGScrollBar
#include "TGScrollBar.h"
#endif
#ifndef ROOT_TRootBrowser
#include "TRootBrowser.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif
#ifndef ROOT_TGFileDialog
#include "TGFileDialog.h"
#endif
#ifndef ROOT_TGShutter
#include "TGShutter.h"
#endif
#ifndef ROOT_TGButtonGroup
#include "TGButtonGroup.h"
#endif
#ifndef ROOT_TGCanvas
#include "TGCanvas.h"
#endif
#ifndef ROOT_TGFSContainer
#include "TGFSContainer.h"
#endif
#ifndef ROOT_TGFSComboBox
#include "TGFSComboBox.h"
#endif
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif
#ifndef ROOT_TGMsgBox
#include "TGMsgBox.h"
#endif
#ifndef ROOT_TGTab
#include "TGTab.h"
#endif
#ifndef ROOT_TGListView
#include "TGListView.h"
#endif
#ifndef ROOT_TGSplitter
#include "TGSplitter.h"
#endif
#ifndef ROOT_TGStatusBar
#include "TGStatusBar.h"
#endif
#ifndef ROOT_TGListTree
#include "TGListTree.h"
#endif
#ifndef ROOT_TGToolTip
#include "TGToolTip.h"
#endif
#ifndef ROOT_TGToolBar
#include "TGToolBar.h"
#endif

#include "Riostream.h"
#include "KVDataSet.h"
#include "KVDBTable.h"
#include "KVDBSystem.h"
#include "KVDataBase.h"
#include "KVNucleus.h"
#include "KVMaterial.h"
#include "KVTarget.h"
#include "KVInputDialog.h"
#include "KVConfig.h"
#include "KVUnits.h"
#include "KVIonRangeTable.h"

using namespace std;

ClassImp(KVDBSystemDialog)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDBSystemDialog</h2>
<h4>Dialog box for setting system parameters associated to runs</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVDBSystemDialog::KVDBSystemDialog(const TGWindow* p, const TGWindow* main,
                                   KVDBSystem* init_sys, const KVNumberList& runs,
                                   UInt_t w, UInt_t h)
{
   //Default constructor
   //init_sys = initial system corresponding to chosen runs

   fTarget = 0;
   fLayer = 0;
   fSystem = init_sys;
   fRuns = runs;
   //'undo' manager
   fUndo = new KVDatedFileManager(gDataSet->GetDataSetEnv("INDRADB.Systems"), gDataSet->GetDataSetDir());
   fCurrentSystemsFile = gDataSet->GetDataSetEnv("INDRADB.Systems");
   CreateMainWindow(p, main, w, h);
   SetNeedSave(0);
   if (runs.IsEmpty()) fSetRuns->SetEnabled(kFALSE);
   FillSystemList();
   Connect("UpdateRunlist()", "KVINDRARunSheetGUI", const_cast<TGWindow*>(main), "UpdateListOfRuns()");
   //nothing happens until window closes
   gClient->WaitFor(fMainFrame1475);
}

KVDBSystemDialog::~KVDBSystemDialog()
{
   //Destructor
   if (fMainFrame1475) {
      delete fMainFrame1475;
      fMainFrame1475 = 0;
   }
   if (fMaterialsList)delete fMaterialsList;
   delete fUndo;
}

void KVDBSystemDialog::DoClose()
{
   TTimer::SingleShot(150, "KVDBSystemDialog", this,
                      "CloseWindow()");
}

void KVDBSystemDialog::CloseWindow()
{
   delete this;
}


//_________________________________________________________________

void KVDBSystemDialog::DeleteSystem()
{
   //Delete the currently selected system
   if (!fSystem) return;
   TString answer(fSystem->GetName());
   Bool_t ok = kFALSE;
   new KVInputDialog(fMainFrame1475, "Do you really want to delete this system ?", &answer, &ok,
                     "Click OK to delete, Cancel to leave");
   if (!ok) return;
   //unassociate all runs from system
   fSystem->RemoveAllRuns();
   //remove system from database
   gDataBase->GetTable("Systems")->RemoveRecord(fSystem);
   //delete system
   delete fSystem;
   fSystem = 0;
   //update list of systems
   FillSystemList();
   SaveSystems();
   UpdateRunlist();
}

//_________________________________________________________________

void KVDBSystemDialog::CreateNewSystem()
{
   //Use current values of all fields in dialog box to create a new KVDBSystem
   //We open a dialog to ask for the title of the new system, and for the target
   //Z and A if a target is defined.

   TString sys_name;

   //If a projectile and target are defined, we suggest a standard name
   if (IsProjectileDefined() && IsTargetDefined()) {
      sys_name.Form("%ld%s + %s %.3g MeV/A",
                    fNumberEntry1499->GetIntNumber(),  // proj A
                    fTextEntry1490->GetText(), // proj symbol
                    fTarget->GetName(), // target symbol
                    fNumberEntry1509->GetNumber() //projectile energy
                   );
   }

   Bool_t ok_pressed;
   new KVInputDialog(
      fMainFrame1475,
      "Enter name for new system",
      &sys_name,
      &ok_pressed
   );
   if (ok_pressed) {
      cout << "Creating system : " << sys_name.Data() << endl;
      fSystem = new KVDBSystem(sys_name.Data());
      if (IsProjectileDefined()) {
         Int_t Z = (Int_t)fNumberEntry1493->GetIntNumber();
         fSystem->SetZbeam(Z);
         Int_t A = (Int_t)fNumberEntry1499->GetIntNumber();
         fSystem->SetAbeam(A);
         Double_t E = fNumberEntry1509->GetNumber();
         fSystem->SetEbeam(E);
      }
      if (IsTargetDefined()) {
         fSystem->SetTarget(fTarget);
         sys_name.Form("%d", (Int_t)fTarget->GetLayerByIndex(1)->GetZ());
         new KVInputDialog(
            fMainFrame1475,
            "Enter Z of target nucleus",
            &sys_name,
            &ok_pressed
         );
         fSystem->SetZtarget(sys_name.Atoi());
         sys_name.Form("%d", (Int_t)fTarget->GetLayerByIndex(1)->GetMass());
         new KVInputDialog(
            fMainFrame1475,
            "Enter A of target nucleus",
            &sys_name,
            &ok_pressed
         );
         fSystem->SetAtarget(sys_name.Atoi());
      }
      //add system to database
      gDataBase->GetTable("Systems")->AddRecord(fSystem);
      //update list of systems
      FillSystemList();
      //select the new system
      fComboBox1476->Select(fComboBox1476->GetNumberOfEntries() - 2);
      SaveSystems();
      fCreateSystem->SetEnabled(kFALSE);
   }
}

//_________________________________________________________________

void KVDBSystemDialog::SaveSystems()
{
   //Write new Systems.dat file.
   //The list of file versions for the 'Undo' is updated
   gDataBase->Save("Systems");
   SetNeedSave(0);
   fUndo->Update();
   fCurrentSystemsFile = gDataSet->GetDataSetEnv("INDRADB.Systems");
}

//_________________________________________________________________

void KVDBSystemDialog::Undo()
{
   //Revert unsaved changes to system (if fNeedSave=kTRUE), or
   //revert to previously saved version of Systems.dat file

   //to remove unsaved changes, we simply rebuild the database from
   //the existing files
   //if there are no unsaved changes, we replace the current Systems.dat
   //with the previous version
   if (!NeedSave()) {
      //is their a previous version of the Systems.dat ?
      KVString old_vers = fUndo->GetPreviousVersion(fCurrentSystemsFile.Data());
      if (old_vers == "") return; // no older version : cannot undo
      //get full path to previous version
      KVString fullpath_backup;
      KVBase::SearchKVFile(old_vers.Data(), fullpath_backup, gDataSet->GetDataSetDir());
      //get full path to Systems.dat file
      KVString fullpath_systemsdat;
      KVBase::SearchKVFile(gDataSet->GetDataSetEnv("INDRADB.Systems"), fullpath_systemsdat, gDataSet->GetDataSetDir());
      //save current Systems.dat with timestamp if it was not already an old version
      if (fCurrentSystemsFile == gDataSet->GetDataSetEnv("INDRADB.Systems")) {
         KVBase::BackupFileWithDate(fullpath_systemsdat.Data());
      }
      //now make a copy of the previous version, with the name Systems.dat
      gSystem->CopyFile(fullpath_backup.Data(), fullpath_systemsdat.Data(), kTRUE);
      fCurrentSystemsFile = old_vers;
   }
   //now rebuild the database
   gDataSet->GetDataBase("update")->cd();
   SetNeedSave(0);
   //update list of systems
   FillSystemList();
   UpdateRunlist();
}

//_________________________________________________________________

void KVDBSystemDialog::FillSystemList()
{
   //Fill combo box with list of all systems for the current dataset

   //clear list of systems
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(fComboBox1476);
#else
   fComboBox1476->RemoveAll();
#endif
   fComboBox1476->Select(-1);

   Int_t index_selected = 0;
   KVSeqCollection* syslist = gDataBase->GetTable("Systems")->GetRecords();
   syslist->ls();
   TIter itsys(syslist);
   TObject* o;
   Int_t index = 0;
   while ((o = itsys())) {
      if ((KVDBSystem*)o == fSystem) index_selected = index;
      fComboBox1476->AddEntry(o->GetName(), index++);
   }
   //Add entry "New system" at end of list
   fComboBox1476->AddEntry("NEW SYSTEM", index++);
   fComboBox1476->Select(index_selected);
}

void KVDBSystemDialog::SelectSystem(Int_t index)
{
   //Called when a new system is selected in fComboBox1476
   if (index < fComboBox1476->GetNumberOfEntries() - 1) {
      fSystem = (KVDBSystem*)gDataBase->GetTable("Systems")->GetRecords()->At(index);
      fCreateSystem->SetEnabled(kFALSE);
      fSetRuns->SetEnabled(!fRuns.IsEmpty());
   } else {
      fCreateSystem->SetEnabled(kTRUE);
      fSetRuns->SetEnabled(kFALSE);
      fSystem = 0;
      fTarget = 0;
   }
   UpdateSystemProperties();
}

void KVDBSystemDialog::UpdateSystemProperties()
{
   //Update displayed system properties, e.g. when a new system is selected in fComboBox1476
   UpdateProjectileProperties();
   UpdateTargetProperties();
}

void KVDBSystemDialog::EnableProjectileProperties(Bool_t on)
{
   //Called when "Projectile" check box is checked (on=kTRUE) or unchecked (on=kFALSE)
   fTextEntry1490->SetEnabled(on);
   fNumberEntry1493->SetState(on);
   fNumberEntry1499->SetState(on);
   fNumberEntry1509->SetState(on);
}

void KVDBSystemDialog::UpdateProjectileProperties()
{
   //Update displayed projectile properties
   //Called by UpdateSystemProperties().

   if (!fSystem) {
      fCheckButton1376->SetState(kButtonUp);
      EnableProjectileProperties(kFALSE);
      return;
   }
   KVNucleus proj(fSystem->GetZproj(), fSystem->GetAproj());
   //if no projectile is defined, the entire "Projectile" part is "greyed out"
   if (proj.GetZ() == 0) {
      fCheckButton1376->SetState(kButtonUp);
      EnableProjectileProperties(kFALSE);
   } else {
      fCheckButton1376->SetState(kButtonDown);
      EnableProjectileProperties(kTRUE);
   }
   //update projectile symbol
   fTextEntry1490->SetText(proj.GetSymbol("EL"));
   //update projectile Z
   fNumberEntry1493->SetNumber(proj.GetZ());
   //update projectile A
   fNumberEntry1499->SetNumber(proj.GetA());
   //update projectile E/A
   fNumberEntry1509->SetNumber(fSystem->GetEproj());
}

void KVDBSystemDialog::UpdateTargetProperties()
{
   //Update displayed target properties
   //Called by UpdateSystemProperties().

   //clear list of target layers
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(fComboBox1515);
#else
   fComboBox1515->RemoveAll();
#endif
   fComboBox1515->Select(-1);
   if (fSystem) fTarget = fSystem->GetTarget();
   if (!fTarget) {
      fNumberEntry1532->SetNumber(0);
      UpdateTargetLayerProperties(-1);
      return;
   }
   //update target angle
   fNumberEntry1532->SetNumber(fTarget->GetAngleToBeam());
   Int_t index = 0;
   TIter next(fTarget->GetLayers());
   TObject* obj;
   while ((obj = next())) {
      fComboBox1515->AddEntry(obj->GetName(), index++);
   }
   fComboBox1515->Select(0);
}

void KVDBSystemDialog::UpdateTargetLayerProperties(Int_t ind)
{
   //Update displayed target layer properties
   //Called when a new layer is selected in fComboBox1515

   if (ind == -1) {
      fNumberEntry1526->SetNumber(0);
      fNumberEntry1537->SetNumber(0);
      fLayer = 0;
      fTextButton1554->SetEnabled(kFALSE);
      return;
   }
   fLayer = (KVMaterial*)fTarget->GetLayers()->At(ind);
   //update thickness - actually area density in mg/cm2
   fNumberEntry1526->SetNumber(fLayer->GetAreaDensity() / (KVUnits::mg / pow(KVUnits::cm, 2)));
   //update atomic mass
   fNumberEntry1537->SetNumber(fLayer->GetMass());
   //update thickness units
   fLabel1530->SetText("mg/cm2");
   //enable button to remove layer
   fTextButton1554->SetEnabled(kTRUE);
}

void KVDBSystemDialog::ProjectileSymbolChanged()
{
   //Called when projectile symbol is changed and the user presses "Return"
   //Changes Z and A of projectile for system

   KVNucleus r(fTextEntry1490->GetText());
   if (!r.GetZ()) {
      //give warning that symbol is not valid
      new TGMsgBox(gClient->GetRoot(), fMainFrame1475, "KVDBSystemDialog",
                   "Give a valid atomic symbol for the projectile", kMBIconExclamation);
      return;
   }
   //update projectile Z
   fNumberEntry1493->SetNumber(r.GetZ());
   //update projectile A
   fNumberEntry1499->SetNumber(r.GetA());
   if (fSystem) {
      fSystem->SetZbeam(r.GetZ());
      fSystem->SetAbeam(r.GetA());
   }
   SetNeedSave(1);
}

void KVDBSystemDialog::ProjectileZChanged(Long_t)
{
   //Called when projectile Z is changed
   //Changes Z and symbol of projectile for system

   Int_t Z = (Int_t)fNumberEntry1493->GetIntNumber();
   KVNucleus r(Z);
   fTextEntry1490->SetText(r.GetSymbol("EL"));
   Int_t A = r.GetA();
   fNumberEntry1499->SetNumber(A);
   if (fSystem) {
      fSystem->SetZbeam(r.GetZ());
      fSystem->SetAbeam(r.GetA());
   }
   SetNeedSave(1);
}

void KVDBSystemDialog::ProjectileAChanged(Long_t)
{
   //Called when projectile A is changed
   //Changes A of projectile for system

   Int_t A = (Int_t)fNumberEntry1499->GetIntNumber();
   if (fSystem) fSystem->SetAbeam(A);
   SetNeedSave(1);
}

void KVDBSystemDialog::ProjectileEChanged(Long_t)
{
   //Called when projectile E is changed
   //Changes E of projectile for system

   Double_t E = fNumberEntry1509->GetNumber();
   if (fSystem) fSystem->SetEbeam(E);
   SetNeedSave(1);
}

void KVDBSystemDialog::TargetLayerThicknessChanged(Long_t)
{
   // Called when target layer "thickness" is changed
   // Note that this is in fact the area density in mg/cm**2

   Double_t t = fNumberEntry1526->GetNumber();
   fLayer->SetAreaDensity(t * KVUnits::mg / pow(KVUnits::cm, 2));
   SetNeedSave(1);
}

void KVDBSystemDialog::TargetLayerAChanged(Long_t)
{
   //Called when target layer A is changed

   Double_t A = fNumberEntry1537->GetNumber();
   fLayer->SetMass(A);
   SetNeedSave(1);
}

void KVDBSystemDialog::TargetAngleChanged(Long_t)
{
   //Called when target angle is changed

   Double_t A = fNumberEntry1532->GetNumber();
   fTarget->SetAngleToBeam(A);
   SetNeedSave(1);
}

void KVDBSystemDialog::AddNewTargetLayer()
{
   //Called when "Add" button is clicked.
   //Add a new target layer. Creates target if none exists.
   //The currently selected material in the list will be used
   //The new layer will be added after any existing layers in the target.

   //get selected material
   TNamed* mat = (TNamed*)fMaterialsList->At(fComboBox1542->GetSelected());
   KVMaterial bidon;
   KVIonRangeTable* RT = bidon.GetRangeTable();
   //add to target of current system
   //if no target is defined, we create a new one
   if (!fTarget) {
      fTarget = new KVTarget;
      if (fSystem) {
         fSystem->SetTarget(fTarget);
         cout << "Created target for system : " << fSystem->GetName() << endl;
         fSystem->SetZtarget((UInt_t)RT->GetZ(mat->GetName()));
         fSystem->SetAtarget((UInt_t)RT->GetAtomicMass(mat->GetName()));
      }
   }
   //add layer with default area density 0.1 mg/cm2
   fTarget->AddLayer(mat->GetTitle(), 0.1);
   //update list of layers in target
   Int_t nlay = fComboBox1515->GetNumberOfEntries();
   fComboBox1515->AddEntry(mat->GetName(), nlay);
   fComboBox1515->Select(nlay + 1);
   cout << "Added layer " << mat->GetTitle() << " to target" << endl;
   SetNeedSave(1);
}

void KVDBSystemDialog::RemoveTargetLayer()
{
   //Removes currently selected layer from target.
   //In fact we create a new target which does not have the offending layer

   cout << "Removing layer : " << fLayer->GetName() << " from target !" << endl;
   if (fTarget->NumberOfLayers() == 1) {
      //last layer in target - destroy target
      delete fTarget;
      fTarget = 0;
      if (fSystem) fSystem->SetTarget(0);
      fLayer = 0;
   } else {
      KVTarget* new_target = new KVTarget;
      TIter next(fTarget->GetLayers());
      KVMaterial* mat;
      while ((mat = (KVMaterial*)next())) {
         if (mat != fLayer) {
            new_target->AddLayer(mat->GetType(), mat->GetAreaDensity());
         }
      }
      new_target->SetAngleToBeam(fTarget->GetAngleToBeam());
      delete fTarget;
      fTarget = 0;
      if (fSystem) fSystem->SetTarget(new_target);
      fLayer = 0;
   }
   UpdateTargetProperties();
   SetNeedSave(1);
}

void KVDBSystemDialog::SetRuns()
{
   //Called when "SetRuns" button is clicked.
   //The selected system is associated with the runlist passed to the constructor.
   //The Systems.dat file is updated
   //Any previous association run<->system is removed

//    KVNumberList tmp;
//    fSystem->GetRunList(tmp);
//    tmp.Add(fRuns);
//    fSystem->SetRuns(tmp);
   fRuns.Begin();
   while (!fRuns.End()) fSystem->AddRun(fRuns.Next());
   SaveSystems();
   UpdateRunlist();
}

//__________________________________________

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
void KVDBSystemDialog::RemoveAll(TGComboBox* box)
{
   //replaces functionality of TGComboBox::RemoveAll for ROOT versions < 5.11/02

   Int_t n = box->GetListBox()->GetNumberOfEntries();
   if (n) box->RemoveEntries(0, n - 1);
   if (box->GetSelectedEntry()) {
      ((TGTextLBEntry*)box->GetSelectedEntry())->SetTitle("");
      gClient->NeedRedraw(box->GetSelectedEntry());
   } else {
      box->GetTextEntry()->SetTitle("");
      gClient->NeedRedraw(box->GetTextEntry());
   }
}
void KVDBSystemDialog::RemoveAll(TGListBox* box)
{
   //replaces functionality of TGListBox::RemoveAll for ROOT versions < 5.11/02

   Int_t n = box->GetNumberOfEntries();
   if (n) box->RemoveEntries(0, n - 1);
   if (box->GetSelectedEntry()) {
      ((TGTextLBEntry*)box->GetSelectedEntry())->SetTitle("");
      gClient->NeedRedraw(box->GetSelectedEntry());
   }
}
#endif

void KVDBSystemDialog::CreateMainWindow(const TGWindow* p, const TGWindow* main,
                                        UInt_t w, UInt_t h)
{

   //Creates main window and displays it

   fMainFrame1475 = new TGTransientFrame(p, main, w, h);
   fMainFrame1475->SetLayoutBroken(kTRUE);
   fMainFrame1475->Connect("CloseWindow()", "KVDBSystemDialog", this,
                           "DoClose()");
   fMainFrame1475->DontCallClose();      // to avoid double deletions.
   // use hierarchical cleaning
   fMainFrame1475->SetCleanup(kDeepCleanup);

   ULong_t red, cyan, green, yellow, magenta;    // will reflect user color changes
   gClient->GetColorByName("#ff00ff", magenta);
   gClient->GetColorByName("#ff0000", red);
   gClient->GetColorByName("#00ff00", green);
   gClient->GetColorByName("#00ffff", cyan);
   gClient->GetColorByName("#ffff00", yellow);

   // combo box displaying list of systems for current dataset
   fComboBox1476 = new TGComboBox(fMainFrame1475, -1, kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
   fComboBox1476->Select(-1);
   fMainFrame1475->AddFrame(fComboBox1476, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fComboBox1476->MoveResize(88, 8, 296, 22);
   fComboBox1476->Connect("Selected(Int_t)",
                          "KVDBSystemDialog", this,
                          "SelectSystem(Int_t)");

   TGLabel* fLabel1487 = new TGLabel(fMainFrame1475, "System");
   fLabel1487->SetTextJustify(36);
   fMainFrame1475->AddFrame(fLabel1487, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1487->MoveResize(8, 8, 64, 18);

   // "System properties" group frame
   TGGroupFrame* fGroupFrame1488 = new TGGroupFrame(fMainFrame1475, "System properties");
   fGroupFrame1488->SetLayoutBroken(kTRUE);

   fCheckButton1376 = new TGCheckButton(fGroupFrame1488, "Projectile");
   fCheckButton1376->SetToolTipText("Define projectile for system");
   fGroupFrame1488->AddFrame(fCheckButton1376, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fCheckButton1376->MoveResize(18, 24, 72, 19);
   fCheckButton1376->Connect("Toggled(Bool_t)",
                             "KVDBSystemDialog", this,
                             "EnableProjectileProperties(Bool_t)");

   TGFont* ufont;         // will reflect user font changes
   ufont = gClient->GetFont("-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

   TGGC*   uGC;           // will reflect user GC changes
   // graphics context changes
   GCValues_t valEntry1490;
   valEntry1490.fMask = kGCForeground | kGCBackground | kGCFillStyle | kGCFont | kGCGraphicsExposures;
   gClient->GetColorByName("#000000", valEntry1490.fForeground);
   gClient->GetColorByName("#c6c3c6", valEntry1490.fBackground);
   valEntry1490.fFillStyle = kFillSolid;
   valEntry1490.fFont = ufont->GetFontHandle();
   valEntry1490.fGraphicsExposures = kFALSE;
   uGC = gClient->GetGC(&valEntry1490, kTRUE);
   fTextEntry1490 = new TGTextEntry(fGroupFrame1488, new TGTextBuffer(15), -1, uGC->GetGC(), ufont->GetFontStruct(), kSunkenFrame | kDoubleBorder | kOwnBackground);
   fTextEntry1490->SetMaxLength(3);
   fTextEntry1490->SetAlignment(kTextLeft);
   fTextEntry1490->SetText("p");
   fTextEntry1490->Resize(24, fTextEntry1490->GetDefaultHeight());
   fTextEntry1490->SetToolTipText("Chemical symbol for projectile nuclei");
   fTextEntry1490->Connect("ReturnPressed()",
                           "KVDBSystemDialog", this, "ProjectileSymbolChanged()");

   fGroupFrame1488->AddFrame(fTextEntry1490, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fTextEntry1490->MoveResize(100, 24, 24, 22);
   fNumberEntry1493 = new TGNumberEntry(fGroupFrame1488, (Double_t) 1, 3, -1, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 2, (TGNumberFormat::ELimit) 2, 1, 92);
   fGroupFrame1488->AddFrame(fNumberEntry1493, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1493->MoveResize(158, 24, 43, 22);
   fNumberEntry1493->Connect("ValueSet(Long_t)",
                             "KVDBSystemDialog", this, "ProjectileZChanged(Long_t)");

   TGLabel* fLabel1497 = new TGLabel(fGroupFrame1488, "Z=");
   fLabel1497->SetTextJustify(36);
   fGroupFrame1488->AddFrame(fLabel1497, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1497->MoveResize(130, 24, 24, 18);
   TGLabel* fLabel1498 = new TGLabel(fGroupFrame1488, "A=");
   fLabel1498->SetTextJustify(36);
   fGroupFrame1488->AddFrame(fLabel1498, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1498->MoveResize(206, 24, 24, 18);
   fNumberEntry1499 = new TGNumberEntry(fGroupFrame1488, (Double_t) 1, 3, -1, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 2, (TGNumberFormat::ELimit) 2, 1, 250);
   fGroupFrame1488->AddFrame(fNumberEntry1499, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1499->MoveResize(232, 24, 43, 22);
   fNumberEntry1499->Connect("ValueSet(Long_t)",
                             "KVDBSystemDialog", this, "ProjectileAChanged(Long_t)");

   TGLabel* fLabel1503 = new TGLabel(fGroupFrame1488, "Q=");
   fLabel1503->SetTextJustify(36);
   fGroupFrame1488->AddFrame(fLabel1503, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1503->MoveResize(280, 24, 24, 18);
   //the "Q" field is inactive for the moment
   fLabel1503->Disable();
   TGNumberEntry* fNumberEntry1504 = new TGNumberEntry(fGroupFrame1488, (Double_t) 0, 3, -1, (TGNumberFormat::EStyle) 0);
   fGroupFrame1488->AddFrame(fNumberEntry1504, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1504->MoveResize(312, 24, 43, 22);
   //the "Q" field is inactive for the moment
   fNumberEntry1504->SetState(kFALSE);
   TGLabel* fLabel1508 = new TGLabel(fGroupFrame1488, "Energy=");
   fLabel1508->SetTextJustify(36);
   fGroupFrame1488->AddFrame(fLabel1508, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1508->MoveResize(96, 56, 48, 18);
   fNumberEntry1509 = new TGNumberEntry(fGroupFrame1488, (Double_t) 0, 9, -1, (TGNumberFormat::EStyle) 5);
   fGroupFrame1488->AddFrame(fNumberEntry1509, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1509->MoveResize(152, 56, 80, 22);
   fNumberEntry1509->Connect("ValueSet(Long_t)",
                             "KVDBSystemDialog", this, "ProjectileEChanged(Long_t)");

   TGLabel* fLabel1513 = new TGLabel(fGroupFrame1488, "MeV/nucleon");
   fLabel1513->SetTextJustify(36);
   fGroupFrame1488->AddFrame(fLabel1513, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1513->MoveResize(240, 56, 80, 18);

   // "Target" group frame
   TGGroupFrame* fGroupFrame1514 = new TGGroupFrame(fGroupFrame1488, "Target");
   fGroupFrame1514->SetLayoutBroken(kTRUE);


   // combo box with layers of target
   fComboBox1515 = new TGComboBox(fGroupFrame1514, -1, kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
   fComboBox1515->Resize(104, 22);
   fComboBox1515->Select(-1);
   fComboBox1515->Connect("Selected(Int_t)",
                          "KVDBSystemDialog", this, "UpdateTargetLayerProperties(Int_t)");

   fGroupFrame1514->AddFrame(fComboBox1515, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fComboBox1515->MoveResize(80, 48, 104, 22);
   fNumberEntry1526 = new TGNumberEntry(fGroupFrame1514, (Double_t) 0, 6, -1, (TGNumberFormat::EStyle) 5, (TGNumberFormat::EAttribute) 2);
   fGroupFrame1514->AddFrame(fNumberEntry1526, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1526->MoveResize(8, 80, 59, 22);
   fNumberEntry1526->Connect("ValueSet(Long_t)",
                             "KVDBSystemDialog", this, "TargetLayerThicknessChanged(Long_t)");

   fLabel1530 = new TGLabel(fGroupFrame1514, "mg/cm2");
   fLabel1530->SetTextJustify(36);
   fGroupFrame1514->AddFrame(fLabel1530, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1530->MoveResize(72, 80, 46, 18);
   TGLabel* fLabel1531 = new TGLabel(fGroupFrame1514, "Angle=");
   fLabel1531->SetTextJustify(36);
   fGroupFrame1514->AddFrame(fLabel1531, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1531->MoveResize(104, 16, 40, 18);
   fNumberEntry1532 = new TGNumberEntry(fGroupFrame1514, (Double_t) 0, 8, -1, (TGNumberFormat::EStyle) 5);
   fGroupFrame1514->AddFrame(fNumberEntry1532, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1532->MoveResize(152, 16, 75, 22);
   fNumberEntry1532->Connect("ValueSet(Long_t)",
                             "KVDBSystemDialog", this, "TargetAngleChanged(Long_t)");

   TGLabel* fLabel1536 = new TGLabel(fGroupFrame1514, "A=");
   fLabel1536->SetTextJustify(36);
   fGroupFrame1514->AddFrame(fLabel1536, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1536->MoveResize(136, 80, 16, 18);
   fNumberEntry1537 = new TGNumberEntry(fGroupFrame1514, (Double_t) 1, 6, -1, (TGNumberFormat::EStyle) 5, (TGNumberFormat::EAttribute) 2, (TGNumberFormat::ELimit) 2, 1, 250);
   fGroupFrame1514->AddFrame(fNumberEntry1537, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fNumberEntry1537->MoveResize(160, 80, 64, 22);
   fNumberEntry1537->Connect("ValueSet(Long_t)",
                             "KVDBSystemDialog", this, "TargetLayerAChanged(Long_t)");

   // "Add new layer" group frame
   TGGroupFrame* fGroupFrame1541 = new TGGroupFrame(fGroupFrame1514, "Add new layer");
   fGroupFrame1541->SetLayoutBroken(kTRUE);


   // combo box for choosing new layer for target
   fComboBox1542 = new TGComboBox(fGroupFrame1541, -1, kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
   fComboBox1542->Resize(80, 22);
   //fill list of all available materials
   KVMaterial bidon;
   fMaterialsList = bidon.GetRangeTable()->GetListOfMaterials();
   TIter it_mat(fMaterialsList);
   TObject* obj;
   Int_t ind = 0;
   while ((obj = it_mat())) {
      fComboBox1542->AddEntry(obj->GetName(), ind++);
   }
   fComboBox1542->Select(0);
   fGroupFrame1541->AddFrame(fComboBox1542, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fComboBox1542->MoveResize(16, 24, 80, 22);
   fTextButton1553 = new TGTextButton(fGroupFrame1541, "Add");
   fTextButton1553->SetTextJustify(36);
   fTextButton1553->Resize(56, 24);
   fGroupFrame1541->AddFrame(fTextButton1553, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fTextButton1553->MoveResize(24, 56, 56, 24);
   fTextButton1553->Connect("Clicked()",
                            "KVDBSystemDialog", this, "AddNewTargetLayer()");

   fGroupFrame1541->SetLayoutManager(new TGVerticalLayout(fGroupFrame1541));
   fGroupFrame1541->Resize(112, 104);
   fGroupFrame1514->AddFrame(fGroupFrame1541, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fGroupFrame1541->MoveResize(232, 40, 112, 104);
   fTextButton1554 = new TGTextButton(fGroupFrame1514, "Remove");
   fTextButton1554->SetTextJustify(36);
   fTextButton1554->Resize(56, 24);
   fGroupFrame1514->AddFrame(fTextButton1554, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fTextButton1554->MoveResize(80, 112, 56, 24);
   fTextButton1554->SetEnabled(kFALSE);
   fTextButton1554->Connect("Clicked()",
                            "KVDBSystemDialog", this, "RemoveTargetLayer()");

   TGLabel* fLabel1555 = new TGLabel(fGroupFrame1514, "Layer:");
   fLabel1555->SetTextJustify(36);
   fGroupFrame1514->AddFrame(fLabel1555, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fLabel1555->MoveResize(24, 48, 40, 18);

   fGroupFrame1514->SetLayoutManager(new TGVerticalLayout(fGroupFrame1514));
   fGroupFrame1514->Resize(360, 168);
   fGroupFrame1488->AddFrame(fGroupFrame1514, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fGroupFrame1514->MoveResize(8, 80, 360, 168);

   //___________________________________________________________//
   //  Bottom row buttons
   //___________________________________________________________//

   fCreateSystem = new TGTextButton(fGroupFrame1488, "Create");
   fCreateSystem->SetTextJustify(36);
   fCreateSystem->Resize(48, 24);
   fCreateSystem->ChangeBackground(green);
   fCreateSystem->SetToolTipText("Create new system with current properties");
   fGroupFrame1488->AddFrame(fCreateSystem, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fCreateSystem->MoveResize(36, 280, 60, 24);
   fCreateSystem->Connect("Clicked()",
                          "KVDBSystemDialog", this, "CreateNewSystem()");
   fCreateSystem->SetEnabled(kFALSE);

   TGTextButton* fTextButton1573 = new TGTextButton(fGroupFrame1488, "Delete");
   fTextButton1573->SetTextJustify(36);
   fTextButton1573->Resize(48, 24);
   fTextButton1573->ChangeBackground(red);
   fGroupFrame1488->AddFrame(fTextButton1573, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fTextButton1573->MoveResize(100, 280, 60, 24);
   fTextButton1573->SetToolTipText("Delete the currently selected system");
   fTextButton1573->Connect("Clicked()",
                            "KVDBSystemDialog", this, "DeleteSystem()");

   fSetRuns = new TGTextButton(fGroupFrame1488, "Set Runs");
   fSetRuns->SetTextJustify(36);
   fSetRuns->Resize(48, 24);
   fSetRuns->ChangeBackground(yellow);
   fGroupFrame1488->AddFrame(fSetRuns, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fSetRuns->MoveResize(164, 280, 60, 24);
   fSetRuns->SetToolTipText("Set runs for selected system");
   fSetRuns->Connect("Clicked()",
                     "KVDBSystemDialog", this, "SetRuns()");

   fSaveButton = new TGTextButton(fGroupFrame1488, "Save");
   fSaveButton->SetTextJustify(36);
   fSaveButton->Resize(48, 24);
   //fSaveButton->ChangeBackground(yellow);
   fGroupFrame1488->AddFrame(fSaveButton, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fSaveButton->MoveResize(228, 280, 60, 24);
   fSaveButton->SetToolTipText("Save changes to systems");
   fSaveButton->Connect("Clicked()",
                        "KVDBSystemDialog", this, "SaveSystems()");

   TGTextButton* fcloseButton = new TGTextButton(fGroupFrame1488, "Close");
   fcloseButton->SetTextJustify(36);
   fcloseButton->Resize(48, 24);
//  fcloseButton->ChangeBackground(cyan);
   fGroupFrame1488->AddFrame(fcloseButton, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fcloseButton->MoveResize(292, 280, 60, 24);
   fcloseButton->SetToolTipText("Return to main window");
   fcloseButton->Connect("Clicked()",
                         "KVDBSystemDialog", this, "DoClose()");

   fGroupFrame1488->SetLayoutManager(new TGVerticalLayout(fGroupFrame1488));
   fGroupFrame1488->Resize(376, 320);
   fMainFrame1475->AddFrame(fGroupFrame1488, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   fGroupFrame1488->MoveResize(8, 32, 376, 320);

   fMainFrame1475->MapSubwindows();
   fMainFrame1475->Resize(fMainFrame1475->GetDefaultSize());
   fMainFrame1475->CenterOnParent();
   fMainFrame1475->SetWindowName("Set system properties for runs");
   fMainFrame1475->MapWindow();
   fMainFrame1475->Resize(394, 358);
}

