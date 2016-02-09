#include "KVINDRARunSheetGUI.h"
#include "Riostream.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>
#include "KVINDRADB.h"
#include "KVINDRADBRun.h"
#include "KVInputDialog.h"
#include "TFile.h"
#include "KVDataSet.h"
#include "TApplication.h"
#include "TSystem.h"
#include "KVDataSetManager.h"
#include "KVDataRepositoryManager.h"
#include "KVDBSystemDialog.h"

using namespace std;

ClassImp(KVINDRARunSheetGUI)

//////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataBaseGUI</h2>
<h4>KaliVeda run/system database configuration tool</h4>
<img style="width: 910px; height: 627px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui.png"><br>
<h4>DIRECTORY</h4>
Set path to directory containing runsheets<br>
<h4>FILENAME</h4>
Set format for runsheet filenames, e.g. RUN_%04d.TXT if runsheets files
are called RUN_0001.TXT, RUN_0002.TXT, etc.<br>
<h4>Make TTree</h4>
If ticked, when runsheets are read (by clicking Read runsheets button),
a TTree is created and filled with all information contained in
runsheets (run number, start date/time, scalers, etc.)<br>
<h4>View Tree<br>
</h4>
Launch TTree browser. There is a branch/leaf for each information read
from the runsheets:<br>
<br>
<img style="width: 663px; height: 398px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui_treeviewer.gif"><br>
<br>
Here is an example plot: dead time ("temps_mort") vs. run number:<br>
<br>
<img style="width: 810px; height: 510px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui_TMplot.gif"><br>
<h4>Change dataset</h4>
Choose a different dataset to work with<br>
<h4>Read runsheets</h4>
Read all runsheets in directory shown, if they have the right filename
format. Informations stored in TTree, if Make TTree is ticked.<br>
<h4>Save</h4>
Save all informations in database files:
$KVROOT/KVFiles/[dataset]/DataBase.root,
$KVROOT/KVFiles/[dataset]/Systems.dat and
$KVROOT/KVFiles/[dataset]/Runlist.csv.<br>
<h4>Quit</h4>
Quits the application (without saving informations).<br>
<h4>Systems</h4>
Opens Systems dialog box to create, modify, delete systems or associate
them with selected run(s): see <a href="#Setting_systems_properties">below</a>.<br>
<h4>Unset systems</h4>
Unassociates selected run(s) from any systems.<br>
<h4>Add comment</h4>
Add comment for selected run(s).<br>
<h4>Logbook</h4>
Add a reference to the experiment logbook for selected run(s).<br>
<h4>Set Trigger</h4>
Set multiplicity trigger of INDRA Selector for selected run(s).<br>
<h4>Print run<br>
</h4>
Print all information on selected run (1 only) in terminal window.<br>
<br>
<h3><a name="Setting_systems_properties"></a>Setting systems properties<br>
</h3>
<img style="width: 404px; height: 385px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui_systems.gif"><br>
<br>
Choose an existing system in the drop-down list:<br>
<ul>
<li>modify its properties (press 'Return' key after entering data in
each numeric/text field; see below for Target properties)<br>
</li>
<li>delete it (press "Delete")<br>
</li>
<li>associate it to the selected runs (press "Set Runs"; only active
if runs were selected in main interface)<br>
</li>
</ul>
Press "Save" to save your changes to systems.<br>
<br>
Creating a new system:<br>
<ul>
<li>select NEW SYSTEM in the drop-down list</li>
<li>enter required properties (tick "Projectile" to enter projectile
properties, see below for Target)</li>
<li>press "Create"</li>
<li>System name dialog box appears:</li>
</ul>
<img style="width: 280px; height: 112px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui_systemname.gif"><br>
<ul>
<li>modify as required, then press "OK"</li>
<li>Target Z dialog box appears:</li>
</ul>
<img style="width: 280px; height: 112px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui_Ztarget.gif"><br>
<ul>
<li>modify as necessary, then press "OK"</li>
<li>Target A dialog box appears:</li>
</ul>
<img style="width: 280px; height: 112px;" alt=""
src="http://indra.in2p3.fr/KaliVedaDoc/images/kvdatabasegui_Atarget.gif"><br>
<ul>
<li>modify as necessary, then press "OK"</li>
</ul>
The new system is automatically saved in the database files.<br>
<br>
<span style="font-weight: bold;">Modifying Target Properties</span><br>
To change target properties, select the required layer in the "Layer"
drop-down list,<br>
then modify (press 'Return' key after entering data in each
numeric/text field).To add a<br>
new layer, select a material in the "Add new layer" materials list,
then press "Add".<br>
Then select the new layer in the "Layer" list of the target to set its
properties.<!-- */
// --> END_HTML
//////////////////////////////////////////////////////////

#ifdef __WITHOUT_TGBUTTON_SETENABLED
#define SetEnabled(flag) \
   SetState( flag ? kButtonUp : kButtonDisabled )
#endif

KVINDRARunSheetGUI::KVINDRARunSheetGUI(): TGMainFrame(gClient->GetRoot(), 500,
         300)
{

   GUIenv = new TEnv(".KVDataBaseGUIrc");

   fFirstRun = 0;
   fLastRun = -1;
   fSelectedRun = 0;
   fSelectedEntries = 0;
   fButList = new TList;

   //create new manager GUI

   //Label with name of run sheet directory
   fHframe = new TGHorizontalFrame(this, 10, 10);
   TGTextButton* tb = new TGTextButton(fHframe, "DIRECTORY");
   tb->SetToolTipText("Set path to directory containing run sheets");
   tb->Connect("Clicked()", "KVINDRARunSheetGUI", this,
               "SetRunSheetDir()");
   fHframe->AddFrame(tb,
                     new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2,
                                       2, 2));

   fDirLabel = new TGLabel(fHframe, "./");
   fHframe->AddFrame(fDirLabel,
                     new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 10,
                                       2, 2));

   tb = new TGTextButton(fHframe, "FILENAME");
   tb->SetToolTipText("Set format for run sheet filenames");
   tb->Connect("Clicked()", "KVINDRARunSheetGUI", this, "SetFileFormat()");
   fHframe->AddFrame(tb,
                     new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2,
                                       2, 2));

   fReader.SetFileFormat(GUIenv->GetValue("KVDataBaseGUI.FileFormat", "RUN_%04d.TXT"));

   if (strcmp(fReader.GetFileFormat(), "")) {
      fFormLabel = new TGLabel(fHframe, fReader.GetFileFormat());
   } else
      fFormLabel = new TGLabel(fHframe, "************");

//      fFormLabel->SetTextColor( (TColor*)gROOT->GetListOfColors()->FindObject("green") );
   fHframe->AddFrame(fFormLabel,
                     new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 50,
                                       2, 2));

   TGCheckButton* cb = new TGCheckButton(fHframe, "Make TTree");
   cb->SetDown(kFALSE);
   cb->Connect("Toggled(Bool_t)", "KVINDRARunSheetReader", &fReader,
               "SetMakeTree(Bool_t)");
   cb->SetToolTipText
   ("Check this to fill a TTree with all infos read from runsheet");
   fHframe->AddFrame(cb,
                     new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 10,
                                       2, 2));

   tb = new TGTextButton(fHframe, "View Tree");
   tb->SetToolTipText("Start Tree viewer");
   tb->Connect("Clicked()", "KVINDRARunSheetGUI", this,
               "StartTreeViewer()");
   tb->SetEnabled(kFALSE);
#ifdef __WITHOUT_TGBUTTON_SETENABLED
   cb->Connect("Toggled(Bool_t)", "KVINDRARunSheetGUI", this, "SetViewTreeEnabled(Bool_t)");
   fViewTreeButton = tb;
#else
   cb->Connect("Toggled(Bool_t)", "TGTextButton", tb, "SetEnabled(Bool_t)");
#endif
   fHframe->AddFrame(tb,
                     new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2,
                                       2, 2));

   AddFrame(fHframe,
            new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 5, 5));


   ULong_t red, cyan, green, yellow, magenta;    // will reflect user color changes
   gClient->GetColorByName("#ff00ff", magenta);
   gClient->GetColorByName("#ff0000", red);
   gClient->GetColorByName("#00ff00", green);
   gClient->GetColorByName("#00ffff", cyan);
   gClient->GetColorByName("#ffff00", yellow);

   fHframe = new TGHorizontalFrame(this, 10, 10);
   //2 vertical frames for buttons
   TGVerticalFrame* vf = new TGVerticalFrame(fHframe, 10, 10);
   fButtonsFrame = new TGVButtonGroup(vf, "");
   fChangeDataSet = new TGTextButton(fButtonsFrame, "Change dataset");
   fChangeDataSet->SetToolTipText("Select set of experimental data to work on");
   fChangeDataSet->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                           "ChangeDataSet()");
   fButList->Add(fChangeDataSet);

   fUpdate = new TGTextButton(fButtonsFrame, "Read runsheets");
   fUpdate->SetToolTipText("Read all runsheets in directory, adding informations to database");
   fUpdate->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                    "UpdateList()");
   fUpdate->ChangeBackground(green);
   fButList->Add(fUpdate);

   fSaveDB = new TGTextButton(fButtonsFrame, "Save");
   fSaveDB->
   SetToolTipText("Save all informations in database");
   fSaveDB->Connect("Clicked()", "KVINDRARunSheetGUI", this, "SaveAll()");
   fButList->Add(fSaveDB);

   fQuit = new TGTextButton(fButtonsFrame, "Quit");
   fQuit->
   SetToolTipText("Exit the application");
   fButList->Add(fQuit);
   fQuit->ChangeBackground(red);
   fQuit->Connect("Clicked()", "KVINDRARunSheetGUI", this, "CloseWindow()");
   //make all buttons same width
   fButtonsFrame->
   SetLayoutHints(new
                  TGLayoutHints(kLHintsExpandX | kLHintsCenterX |
                                kLHintsTop, 2, 2, 0, 0));
   fButtonsFrame->Show();
   fButtonsFrame->Resize(fButtonsFrame->GetDefaultWidth(),
                         fButtonsFrame->GetDefaultHeight());
   vf->AddFrame(fButtonsFrame,
                new TGLayoutHints(kLHintsTop | kLHintsCenterX, 0, 0, 0,
                                  10));

   fButtonsFrame = new TGVButtonGroup(vf, "");
   fSetSystem = new TGTextButton(fButtonsFrame, "Systems");
   fSetSystem->SetToolTipText("Modify reaction systems or set system for the selected run(s)");
   fSetSystem->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                       "SetSystem()");
   fSetSystem->ChangeBackground(cyan);
   fButList->Add(fSetSystem);
   fUnSetSystem = new TGTextButton(fButtonsFrame, "Unset System");
   fUnSetSystem->SetToolTipText("Unset the reaction system for the selected run(s)");
   fUnSetSystem->ChangeBackground(cyan);
   fUnSetSystem->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                         "UnsetSystem()");
   fButList->Add(fUnSetSystem);
   fAddComment = new TGTextButton(fButtonsFrame, "Add comment");
   fAddComment->SetToolTipText("Add a comment to selected run");
   fAddComment->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                        "SetRunComment()");
   fAddComment->ChangeBackground(yellow);
   fButList->Add(fAddComment);
   fAddCahier = new TGTextButton(fButtonsFrame, "Logbook");
   fAddCahier->
   SetToolTipText
   ("Add a reference to the experiment logbook for selected run");
   fAddCahier->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                       "SetRunCahier()");
   fAddCahier->ChangeBackground(yellow);
   fButList->Add(fAddCahier);
   fSetTrigger = new TGTextButton(fButtonsFrame, "Set Trigger");
   fSetTrigger->
   SetToolTipText
   ("Set the multiplicity trigger for the selected run(s)");
   fSetTrigger->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                        "SetRunTrigger()");
   fSetTrigger->ChangeBackground(yellow);
   fButList->Add(fSetTrigger);
   fPrintRun = new TGTextButton(fButtonsFrame, "Print run");
   fPrintRun->SetToolTipText("Print all informations on selected run");
   fPrintRun->Connect("Clicked()", "KVINDRARunSheetGUI", this,
                      "PrintRun()");
   fPrintRun->ChangeBackground(magenta);
   fButList->Add(fPrintRun);
   //make all buttons same width
   fButtonsFrame->
   SetLayoutHints(new
                  TGLayoutHints(kLHintsExpandX | kLHintsCenterX |
                                kLHintsTop, 2, 2, 0, 0));
   fButtonsFrame->Show();
   fButtonsFrame->Resize(fButtonsFrame->GetDefaultWidth(),
                         fButtonsFrame->GetDefaultHeight());
   vf->AddFrame(fButtonsFrame,
                new TGLayoutHints(kLHintsTop | kLHintsCenterX, 0, 0, 0,
                                  0));

   fHframe->AddFrame(vf,
                     new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 20,
                                       10, 2, 2));

   //disable all buttons
   DisableButtons();

   //list view for runs
   fRunList = new KVListView(KVINDRADBRun::Class(), fHframe, 350, 250);
   fRunList->SetDataColumns(9);
   fRunList->SetDataColumn(0, "Run", "GetNumber", kTextRight);
   fRunList->SetDataColumn(1, "Events", "", kTextRight);
   fRunList->SetDataColumn(2, "System", "GetSystemName");
   fRunList->SetDataColumn(3, "Start", "GetStartDatime");
   fRunList->GetDataColumn(3)->SetIsDateTime(KVDatime::kGANACQ);
   fRunList->SetDataColumn(4, "End", "GetEndDatime");
   fRunList->GetDataColumn(4)->SetIsDateTime(KVDatime::kGANACQ);
   fRunList->SetDataColumn(5, "Trigger", "GetTriggerString");
   fRunList->SetDataColumn(6, "DeadTime (%)", "GetTMpercent");
   fRunList->SetDataColumn(7, "Comments", "", kTextLeft);
   fRunList->SetDataColumn(8, "Logbook", "", kTextLeft);
   fRunList->ActivateSortButtons();
   // disable context menu & Browse functions
   fRunList->AllowBrowse(kFALSE);
   fRunList->AllowContextMenu(kFALSE);

   fRunList->Connect("SelectionChanged()", "KVINDRARunSheetGUI", this,
                     "SelectionChanged()");

   fHframe->AddFrame(fRunList,
                     new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                       kLHintsExpandX | kLHintsExpandY, 30,
                                       10, 10, 10));

   AddFrame(fHframe,
            new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0,
                              0));

   SetWindowName("KVDataBaseGUI");

   //layout & draw window
   MapSubwindows();
   Resize(GetDefaultSize());
   MapWindow();
   SetWMSize(900, 600);

   //initialise data repository manager if needed
   if (!gDataRepositoryManager) {
      new KVDataRepositoryManager;
      gDataRepositoryManager->Init();
   }
   //open last used dataset
   KVString dataset = GUIenv->GetValue("KVDataBaseGUI.DataSet", "");
   if (dataset == "") {
      //open dataset dialog box - user must choose dataset
      new KVIRSGChooseDataSetDialog(gClient->GetRoot(), this, 10, 10);
      GUIenv->SetValue("KVDataBaseGUI.DataSet", gDataSet->GetName());
      GUIenv->SaveLevel(kEnvUser);
   } else {
      // check dataset exists
      KVDataSet* ds = gDataSetManager->GetDataSet(dataset.Data());
      if (ds) ds->cd();
      else {
         //open dataset dialog box - user must choose dataset
         new KVIRSGChooseDataSetDialog(gClient->GetRoot(), this, 10, 10);
         GUIenv->SetValue("KVDataBaseGUI.DataSet", gDataSet->GetName());
         GUIenv->SaveLevel(kEnvUser);
      }
   }
   //initialise list of runs
   UpdateListOfRuns();

   //set directory for runsheets
   fReader.SetRunSheetDir(
      GUIenv->GetValue(Form("KVDataBaseGUI.RunSheetDir.%s", gDataSet->GetName()), "./")
   );
   fDirLabel->SetText(fReader.GetRunSheetDir());
}

void KVINDRARunSheetGUI::ChangeDataSet()
{
   //open dialog to choose dataset
   new KVIRSGChooseDataSetDialog(gClient->GetRoot(), this, 10, 10);
   GUIenv->SetValue("KVDataBaseGUI.DataSet", gDataSet->GetName());
   GUIenv->SaveLevel(kEnvUser);
   //set directory for runsheets
   fReader.SetRunSheetDir(
      GUIenv->GetValue(Form("KVDataBaseGUI.RunSheetDir.%s", gDataSet->GetName()), "./")
   );
   fDirLabel->SetText(fReader.GetRunSheetDir());
   //initialise list of runs
   UpdateListOfRuns();
}

KVINDRARunSheetGUI::~KVINDRARunSheetGUI()
{
   //close window
   UnmapWindow();
   if (fSelectedEntries) delete fSelectedEntries;
   delete fButList;
   fButList = 0;
   if (GUIenv) delete GUIenv;
}

void KVINDRARunSheetGUI::CloseWindow()
{
   //close viewer
   gApplication->Terminate();
}

void KVINDRARunSheetGUI::DisableButtons()
{
   //Disable all buttons
   fButList->R__FOR_EACH(TGTextButton, SetEnabled)(kFALSE);
}

void KVINDRARunSheetGUI::EnableButtons()
{
   //Enable buttons when a run is selected, depending on how many are selected
   //"Read runsheets" only enabled if directory and file format have been given
   //"Save DB"/"Create runlist" only if some runs are in the runlist
   //"Load DB" active if no dataset/base defined
   if (strcmp(fReader.GetRunSheetDir(), "")
         && strcmp(fReader.GetFileFormat(), "")) {
      fUpdate->SetEnabled(kTRUE);
   } else {
      fUpdate->SetEnabled(kFALSE);
   }
   if (fLastRun >= fFirstRun) {
      fSaveDB->SetEnabled(kTRUE);
   } else {
      fSaveDB->SetEnabled(kFALSE);
   }
   if (GetNSelected() == 1) {
      fPrintRun->SetEnabled(kTRUE);
   } else
      fPrintRun->SetEnabled(kFALSE);
   if (GetNSelected() >= 1) {
      fAddComment->SetEnabled(kTRUE);
      fAddCahier->SetEnabled(kTRUE);
      fSetTrigger->SetEnabled(kTRUE);
      fUnSetSystem->SetEnabled(kTRUE);
   }
   fSetSystem->SetEnabled(kTRUE);
   fChangeDataSet->SetEnabled(kTRUE);
   fQuit->SetEnabled(kTRUE);
}

void KVINDRARunSheetGUI::SetRunSheetDir()
{
   static TString dir(".");
   TGFileInfo fi;
   fi.fIniDir = StrDup(dir);
   new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
   if (fi.fFilename) {
      dir = fi.fIniDir;
      cout << "directory = " << dir.Data() << endl;
      fReader.SetRunSheetDir(dir.Data());
      GUIenv->SetValue(
         Form("KVDataBaseGUI.RunSheetDir.%s", gDataSet->GetName()),
         dir.Data());
      GUIenv->SaveLevel(kEnvUser);
      //update directory label field
      fDirLabel->SetText(dir.Data());
      Layout();                 //needed according to doc for TGLabel::SetText
      EnableButtons();
   }
}

void KVINDRARunSheetGUI::FillListOfRuns()
{
   // if gINDRADB exists, we use it to fill the list box with each run in the database,
   // sorted according to increasing run number

   if (gIndraDB && gIndraDB->GetRuns() && gIndraDB->GetRuns()->GetSize()) {

      fRunList->Display(gIndraDB->GetRuns());
      // sort runs
      fRunList->Sort(0);
      fFirstRun = ((KVINDRADBRun*)fRunList->GetFirstInList())->GetNumber();
      fLastRun = ((KVINDRADBRun*)fRunList->GetLastInList())->GetNumber();
      if (fFirstRun > fLastRun) {
         // sorting was in reverse order
         fRunList->Sort(0);
         fFirstRun = ((KVINDRADBRun*)fRunList->GetFirstInList())->GetNumber();
         fLastRun = ((KVINDRADBRun*)fRunList->GetLastInList())->GetNumber();
      }
   }
}

void KVINDRARunSheetGUI::ClearListOfRuns()
{
   //remove all entries from grid list box

   fRunList->RemoveAll();
   fFirstRun = 0;
   fLastRun = -1;
   fSelectedRun = 0;
   if (fSelectedEntries) {
      delete fSelectedEntries;
      fSelectedEntries = 0;
   }
}

void KVINDRARunSheetGUI::UpdateListOfRuns()
{
   //fill list from gIndraDB
   //buttons are disabled, as any selected run is deselected
   ClearListOfRuns();
   FillListOfRuns();
   SelectionChanged();
}

void KVINDRARunSheetGUI::UpdateList()
{
   //called when "Update list" button is pressed
   //KVINDRARunSheetReader is used to read all available runsheets
   //then displayed list is updated
   //File format is first read from the text entry field
   //Database runlist is saved
   //We also update the list of raw data files in the repository
   Int_t read = fReader.ReadAllRunSheets() ;
   cout << "Read " << read << " runsheets" << endl;
   cout << "fReader.IsMakeTree()=" << fReader.IsMakeTree() << endl;
   gDataBase->Save("Runlist");
   ClearListOfRuns();
   gDataSet->UpdateAvailableRuns("raw");
   FillListOfRuns();
   SelectionChanged();
}

void KVINDRARunSheetGUI::SelectionChanged()
{
   //called any time the selection of runs changes
   //each time, we update:
   //fSelectedEntries : the list of all selected runs (KVINDRADBRun objects)
   //GetNSelected() : the number of selected entries
   //fSelectedRun : the last selected run (=the only run selected if GetNSelected==1)
   //The buttons' state is modified according to the number of selected runs

   if (fSelectedEntries) {
      delete fSelectedEntries;
      fSelectedEntries = 0;
   }
   fSelectedEntries = fRunList->GetSelectedObjects();
   if (fSelectedEntries->GetEntries())
      fSelectedRun = (KVINDRADBRun*)fRunList->GetLastSelectedObject();
   else fSelectedRun = 0;
   DisableButtons();
   EnableButtons();
}

Int_t KVINDRARunSheetGUI::GetNSelected()
{
   //returns current number of selected items in grid list
   return (fSelectedEntries ? fSelectedEntries->GetSize() : 0);
}

void KVINDRARunSheetGUI::SaveAll()
{
   //Write the Runlist.csv, Systems.dat and update (and save)
   //the database
   gIndraDB->Save("Systems");
   gIndraDB->Save("Runlist");
   ClearListOfRuns();
   gDataSet->GetDataBase("update");
   UpdateListOfRuns();
}

void KVINDRARunSheetGUI::SetRunComment()
{
   fInputString = "";
   if (fSelectedRun)
      fInputString = fSelectedRun->GetComments();
   Bool_t ok;
   new KVInputDialog(this, "Enter comment for run(s)", &fInputString, &ok);
   if (ok) {
      if (GetNSelected() == 1)
         fSelectedRun->SetComments(fInputString.Data());
      else if (GetNSelected() > 1) {
         //multiselection
         TIter next(fSelectedEntries);
         KVINDRADBRun* run;
         while ((run = (KVINDRADBRun*)next())) run->SetComments(fInputString.Data());
      }
   }
}

void KVINDRARunSheetGUI::SetRunCahier()
{
   fInputString = "";
   if (fSelectedRun)
      fInputString = fSelectedRun->GetCahier();
   Bool_t ok;
   new KVInputDialog(this, "Enter log-book reference for run(s)",
                     &fInputString, &ok);
   if (ok) {
      if (GetNSelected() == 1)
         fSelectedRun->SetCahier(fInputString.Data());
      else if (GetNSelected() > 1) {
         //multiselection
         TIter next(fSelectedEntries);
         KVINDRADBRun* run;
         while ((run = (KVINDRADBRun*)next())) run->SetCahier(fInputString.Data());
      }
   }
}

void KVINDRARunSheetGUI::SetRunTrigger()
{
   fInputString = "";
   if (fSelectedRun)
      fInputString.Form("%d", fSelectedRun->GetTrigger());
   Bool_t ok;
   new KVInputDialog(this, "Enter multiplicity trigger for run(s)",
                     &fInputString, &ok,
                     "Enter minimum multiplicity, i.e. for M>=4 type '4'");
   if (ok && fInputString.IsDigit()) {
      Int_t trig = fInputString.Atoi();
      if (GetNSelected() == 1)
         fSelectedRun->SetTrigger(trig);
      else if (GetNSelected() > 1) {
         //multiselection
         TIter next(fSelectedEntries);
         KVINDRADBRun* run;
         while ((run = (KVINDRADBRun*)next())) {
            run->BlockSignals();
            run->SetTrigger(trig);
            run->BlockSignals(0);
         }
         UpdateListOfRuns();
      }
   }
}

void KVINDRARunSheetGUI::SetFileFormat()
{
   fInputString = fReader.GetFileFormat();
   Bool_t ok;
   new KVInputDialog(this, "Enter format string for runsheet filenames",
                     &fInputString, &ok,
                     "e.g. RUN_%04d.TXT for filenames such as RUN_0123.TXT");
   if (ok) {
      fReader.SetFileFormat(fInputString.Data());
      GUIenv->SetValue("KVDataBaseGUI.FileFormat", fInputString.Data());
      GUIenv->SaveLevel(kEnvUser);
      EnableButtons();
      fFormLabel->SetText(fInputString.Data());
      Layout();
   }
}

void KVINDRARunSheetGUI::StartTreeViewer()
{
   fReader.StartTreeViewer();
}

void KVINDRARunSheetGUI::PrintRun()
{
   if (fSelectedRun)
      fSelectedRun->Print();
}

void KVINDRARunSheetGUI::SetSystem()
{
   //Open dialog box for setting system properties for runs
   KVDBSystem* run_system = 0;
   if (fSelectedRun) run_system = fSelectedRun->GetSystem();
   KVNumberList runlist;
   if (GetNSelected() == 1)
      runlist.Add(fSelectedRun->GetNumber());
   else if (GetNSelected() > 1) {
      //multiselection
      TIter next(fSelectedEntries);
      KVINDRADBRun* run;
      while ((run = (KVINDRADBRun*)next())) runlist.Add(run->GetNumber());
   }
   new KVDBSystemDialog(gClient->GetRoot(), this, run_system, runlist, 10, 10);
}

void KVINDRARunSheetGUI::UnsetSystem()
{
   //Unset the system for selected runs
   //new Systems.dat file will be saved
   if (GetNSelected() == 1) {
      if (fSelectedRun->GetSystem()) {
         cout << "Unsetting system (" << fSelectedRun->GetSystem()->GetName() << " for run " << fSelectedRun->GetNumber() << endl;
         fSelectedRun->UnsetSystem();
      }
   } else if (GetNSelected() > 1) {
      //multiselection
      TIter next(fSelectedEntries);
      KVINDRADBRun* run;
      while ((run = (KVINDRADBRun*)next())) {
         if (run->GetSystem()) {
            cout << "Unsetting system (" << run->GetSystem()->GetName() << " for run " << run->GetNumber() << endl;
            run->UnsetSystem();
         }
      }
   }
   gDataBase->Save("Systems");
   UpdateListOfRuns();
}

//______________________________________________________________//

ClassImp(KVIRSGChooseDataSetDialog)

KVIRSGChooseDataSetDialog::KVIRSGChooseDataSetDialog
(const TGWindow* p, const TGWindow* main, UInt_t w, UInt_t h)
{

   // main frame
   fMain = new TGTransientFrame(p, main, w, h);
   fMain->SetLayoutBroken(kTRUE);
   fMain->Connect("CloseWindow()", "KVIRSGChooseDataSetDialog", this,
                  "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   fLabel = new TGLabel(fMain, "Choose dataset:");
   fLabel->SetTextJustify(36);
   fMain->AddFrame(fLabel,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2,
                                     2));
   fLabel->MoveResize(8, 8, 208, 24);

   gClient->GetColorByName("#ffffff", ucolor);

   // combo box
   fComboBox = new TGComboBox(fMain);

   //fill list of datasets
   if (gDataSetManager) {
      for (int i = 0; i < gDataSetManager->GetNtotal(); i++) {
         fComboBox->AddEntry(gDataSetManager->GetDataSet(i)->GetName(), i);
      }
   }

   fComboBox->Connect("Selected(Int_t)", "KVIRSGChooseDataSetDialog", this,
                      "SetDataSet(Int_t)");
   fComboBox->Resize(208, 22);
   fComboBox->Select(0);
   fDataSetIndex = -1;
   fMain->AddFrame(fComboBox,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2,
                                     2));
   fComboBox->MoveResize(8, 40, 208, 22);

   TGTextButton* fTextButton = new TGTextButton(fMain, "OK");
   fTextButton->SetTextJustify(36);
   fTextButton->Resize(80, 24);
   fMain->AddFrame(fTextButton,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2,
                                     2));
   fTextButton->MoveResize(72, 72, 80, 24);

   fTextButton->Connect("Clicked()", "KVIRSGChooseDataSetDialog", this,
                        "YouPressedOK()");

   fMain->MapSubwindows();

   fMain->Resize(fMain->GetDefaultSize());
   fMain->CenterOnParent();
   fMain->SetWindowName("Choose dataset dialog box");
   fMain->MapWindow();
   fMain->Resize(223, 110);
   //nothing happens until window closes
   gClient->WaitFor(fMain);
}

KVIRSGChooseDataSetDialog::~KVIRSGChooseDataSetDialog()
{
   //Delete all widgets

   if (fMain) {
      delete fMain;
      fMain = 0;
   }
}

void KVIRSGChooseDataSetDialog::DoClose()
{
   TTimer::SingleShot(150, "KVIRSGChooseDataSetDialog", this,
                      "CloseWindow()");
}

void KVIRSGChooseDataSetDialog::CloseWindow()
{
   delete this;
}

void KVIRSGChooseDataSetDialog::YouPressedOK()
{
   //Called when OK button is pressed
   if (fDataSetIndex > -1) gDataSetManager->GetDataSet(fDataSetIndex)->cd();
   DoClose();
}
#ifdef __WITHOUT_TGBUTTON_SETENABLED
#undef SetEnabled
#endif
