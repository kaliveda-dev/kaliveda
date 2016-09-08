//
// Implementation de KVDataAnalysisLauncher
//

#include "KVDataAnalysisLauncher.h"
#include "TGLabel.h"
#include "TGMsgBox.h"
#include "TColor.h"
#include "TGButtonGroup.h"
#include "TGFileDialog.h"
#include "TObjString.h"
#include "TDatime.h"
#include "TApplication.h"
#include "TSocket.h"
#include "TGToolTip.h"
#include "TGFileDialog.h"
#include "Riostream.h"
#include "TClass.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "KVDataSet.h"
#include "KVDataAnalysisTask.h"
#include "KVDBSystem.h"
#include "KV2Body.h"
#include "KVNucleus.h"

#include "KVINDRAReconDataAnalyser.h"
#include "KVBatchSystemManager.h"
#include "TSystemDirectory.h"
#include "KVInputDialog.h"
#include "KVBatchSystemGUI.h"

#include <KVRunFile.h>

#define TTDELAY 750

//maximum length of runlist shown next to "Selected Runs :"
#define MAX_LENGTH_SELECTED_RUNS 40

//#define KVDAL_DEBUG

using namespace std;

ClassImp(KVGFileList)

//__________________________________________
void KVGFileList::Init(TString& fileList, const Char_t* title)
{
   // Init window
   TGLayoutHints* eXeY = new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                           kLHintsExpandX | kLHintsExpandY,
                                           1, 1, 1, 1);
   TGLayoutHints* eX = new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                         kLHintsExpandX,
                                         1, 1, 1, 1);

   // File list TGListBox
   lbFileList = new TGListBox(this, LB_Files);
   lbFileList->SetMultipleSelections(kTRUE);
   lbFileList->Resize(350, 96);
   this->AddFrame(lbFileList, eXeY);

   // File Name TGTextEntry
   teFileName = new TGTextEntry(this, "");
   this->AddFrame(teFileName, eX);

   // buttons
   TGCompositeFrame* cf = new TGCompositeFrame(this, 1200, 350, kHorizontalFrame);
   boutAdd = new TGTextButton(cf, "Add File");
   boutAdd->SetToolTipText("Add a file to the list", TTDELAY);
   boutAdd->Connect("Clicked()",
                    "KVGFileList",
                    this,
                    "AddFile()");
   cf->AddFrame(boutAdd, eX);

   boutRem = new TGTextButton(cf, "Remove File");
   boutRem->SetToolTipText("Remove the selected file from the list", TTDELAY);
   boutRem->Connect("Clicked()",
                    "KVGFileList",
                    this,
                    "RemoveFiles()");
   cf->AddFrame(boutRem, eX);

   boutAllRem = new TGTextButton(cf, "Remove All File");
   boutAllRem->SetToolTipText("Remove all files from the list", TTDELAY);
   boutAllRem->Connect("Clicked()",
                       "KVGFileList",
                       this,
                       "RemoveAllFiles()");
   cf->AddFrame(boutAllRem, eX);

   this->AddFrame(cf, eX);

   TGTextButton* bout = new TGTextButton(this, "Done");
   bout->SetToolTipText("Close window", TTDELAY);
   bout->Connect("Clicked()",
                 "KVGFileList",
                 this,
                 "Done()");
   this->AddFrame(bout, eX);

   // Map all widgets
   MapSubwindows();
   Resize(GetDefaultSize());
   SetWindowName(title);
   SetIconName(title);

   fileListString = &fileList;
   fileDialogDir = gSystem->Getenv("PWD");

   entryMax = 0;
}

//__________________________________________
KVGFileList::KVGFileList(TString& fileList, const Char_t* title,
                         const TGWindow* p, const TGWindow* main,
                         Bool_t ok):
   TGTransientFrame(p, main, 10, 10)
{
   // Createur

   if (ok) {
      Init(fileList, title);
      InitFileList();
      MapRaised();
      fClient->WaitFor(this);
   }
}

//__________________________________________
KVGFileList::~KVGFileList()
{
   // Destructeur
   delete lbFileList;
   delete teFileName;
   delete boutAdd;
   delete boutRem;
   delete boutAllRem;
}

//__________________________________________
void KVGFileList::InitFileList()
{
   // Init the file name list box from a TString. Add all fields separated by a
   // white space.
   TObjArray* oa = fileListString->Tokenize(" ");
   TIter next(oa);
   TObjString* ost = 0;
   while ((ost = (TObjString*)next())) {
      TString fileName = ost->GetString();
      if (fileName.Length() && !lbFileList->FindEntry(fileName.Data())) {
         lbFileList->AddEntry(new TGString(fileName.Data()), entryMax);
         entryMax++;
      }
   }
   oa->SetOwner(kTRUE);
   delete oa;
   lbFileList->MapSubwindows();
   lbFileList->Layout();
}

//__________________________________________
void KVGFileList::AddFile(void)
{
   // Add a file to the listbox. If the TGTextEntry is ampty, a file open dialog
   // appears
   TString fileName = teFileName->GetText();
   if (!fileName.Length()) {
      if (Char_t* fn = this->GetFileFromDialog()) {
         fileName = fn;
      } else {
         fileName = "";
      }
   }
   if (fileName.Length() &&
         !lbFileList->FindEntry(fileName.Data()) &&
         this->CanAdd(fileName.Data())) {
      lbFileList->AddEntry(new TGString(fileName.Data()), entryMax);
      entryMax++;
   }
   lbFileList->MapSubwindows();
   lbFileList->Layout();
}

//__________________________________________
void KVGFileList::RemoveFiles(void)
{
   // Remove all the selected files from the TGListBox
   TList* sel = new TList();
   lbFileList->GetSelectedEntries(sel);
   TIter next(sel);
   TGLBEntry* e = 0;
   while ((e = (TGLBEntry*)next())) {
      gSystem->Unload(((TGTextLBEntry*)e)->GetText()->GetString());
      lbFileList->RemoveEntry(e->EntryId());
   }
   lbFileList->MapSubwindows();
   lbFileList->Layout();
}

//__________________________________________
void KVGFileList::RemoveAllFiles(void)
{
   // Remove all the files from the TGListBox
   lbFileList->RemoveEntries(0, entryMax);
   entryMax = 0;
   lbFileList->MapSubwindows();
   lbFileList->Layout();
}

//__________________________________________
void KVGFileList::Done(void)
{
   // build the file list string from the content of the TGListBox
   (*fileListString) = "";
   for (Int_t i = 0; i < entryMax; i++) {
      if (TGTextLBEntry* e = (TGTextLBEntry*)lbFileList->GetEntry(i)) {
         if (fileListString->Length())(*fileListString) += " ";
         (*fileListString) += e->GetText()->GetString();
      }
   }
   this->CloseWindow();
}

//__________________________________________
Char_t* KVGFileList::GetFileFromDialog(void)
{
   // Gets the file name from a TGFileDialog
   TGFileInfo* fil = new TGFileInfo();
   const char* filTypes[] = {"Shared Object Files", "*.so",
                             0,                  0
                            };
   fil->fFileTypes = filTypes;
   fil->fIniDir = StrDup(fileDialogDir.Data());
   new TGFileDialog(gClient->GetRoot(), this, kFDOpen, fil);
   fileDialogDir = fil->fIniDir;
   return fil->fFilename;
}

//__________________________________________
Bool_t KVGFileList::CanAdd(const Char_t* fn)
{
   // tells whether the file in ths string fn can be added to the list box
   Bool_t ok = kTRUE;

   FileStat_t fs;
   TString tmp = fn;
   gSystem->ExpandPathName(tmp);
   if (gSystem->GetPathInfo(tmp.Data(), fs)) {
      ok = kFALSE;
      new TGMsgBox(gClient->GetRoot(), this,
                   "File does not exist",
                   Form("The file \"%s\" does not exist. Nothing added.",
                        fn),
                   kMBIconExclamation);
   } else {
      TString fname = fn;
      if (!fname.EndsWith(".so")) {
         ok = kFALSE;
         new TGMsgBox(gClient->GetRoot(), this,
                      "Not a share object",
                      Form("The file \"%s\" is not a shared object. Nothing added.",
                           fn),
                      kMBIconExclamation);

      }
   }
   return ok;
}


ClassImp(KVGDirectoryList)

//__________________________________________
void KVGDirectoryList::Init(TString& fileList, const Char_t* title)
{
   // init window
   KVGFileList::Init(fileList, title);

   boutAdd->SetText("Add Directory");
   boutAdd->SetToolTipText("Add a directory to the list.", TTDELAY);
   boutAdd->Layout();

   boutRem->SetText("Remove Directory");
   boutRem->SetToolTipText("Remove the selected directories from the list.", TTDELAY);
   boutRem->Layout();

   boutAllRem->SetText("Remove All Directories");
   boutAllRem->SetToolTipText("Remove all directories from the list.", TTDELAY);
   boutAllRem->Layout();
}

//__________________________________________
KVGDirectoryList::KVGDirectoryList(TString& fileList, const Char_t* title,
                                   const TGWindow* p, const TGWindow* main,
                                   Bool_t ok):
   KVGFileList(fileList, title, p, main, kFALSE)
{
   // Createur

   if (ok) {
      Init(fileList, title);
      InitFileList();
      MapRaised();
      fClient->WaitFor(this);
   }
}

//__________________________________________
KVGDirectoryList::~KVGDirectoryList()
{
   // Destructeur
}


//__________________________________________
Char_t* KVGDirectoryList::GetFileFromDialog(void)
{
   // Gets the file name from a TGFileDialog
   TGFileInfo* fil = new TGFileInfo();
   const char* filTypes[] = {"Include Files", "*.[h,H]*",
                             0,                  0
                            };
   fil->fFileTypes = filTypes;
   fil->fIniDir = StrDup(fileDialogDir.Data());
   new TGFileDialog(gClient->GetRoot(), this, kFDOpen, fil);
   fileDialogDir = fil->fIniDir;
   if (fil->fFilename)
      return fil->fIniDir;
   else
      return 0;
}

//__________________________________________
Bool_t KVGDirectoryList::CanAdd(const Char_t* fn)
{
   // tells whether the file in ths string fn can be added to the list box
   Bool_t ok = kTRUE;

   FileStat_t fs;
   TString tmp = fn;
   gSystem->ExpandPathName(tmp);
   if (gSystem->GetPathInfo(tmp.Data(), fs)) {
      ok = kFALSE;
      new TGMsgBox(gClient->GetRoot(), this,
                   "Directory does not exist",
                   Form("The directory \"%s\" does not exist. Nothing added.",
                        fn),
                   kMBIconExclamation);
   } else {
      if (!(fs.fMode & kS_IFDIR)) {
         ok = kFALSE;
         new TGMsgBox(gClient->GetRoot(), this,
                      "Not a directory",
                      Form("The path \"%s\" is not a directory. Nothing added.",
                           fn),
                      kMBIconExclamation);

      }
   }
   return ok;
}

ClassImp(KVDataAnalysisLauncher)

//__________________________________________
KVDataAnalysisLauncher::KVDataAnalysisLauncher(const TGWindow* p, UInt_t w, UInt_t h): TGMainFrame(p, w, h)
{
   // Main window width and height can be set using .kvrootrc variables:
   //     KaliVedaGUI.MainGUIWidth:       800
   //     KaliVedaGUI.MainGUIHeight:       600
   // Maximum column width of runlist can be set using:
   //     KaliVedaGUI.MaxColWidth:       500

   entryMax = -1;
   //Initialisation of resource file
   GUIenv = new TEnv(".KVDataAnalysisGUIrc");
   //initialisation of list used by Get/SetResource
   ResourceNames = new TList;
   ResourceNames->Add(new TNamed("Repository", ""));
   ResourceNames->Add(new TNamed("DataSet", ""));
   ResourceNames->Add(new TNamed("Task", ""));
   ResourceNames->Add(new TNamed("System", ""));
   ResourceNames->Add(new TNamed("Trigger", ""));
   NbResNames = ResourceNames->GetEntries();

   // list of user analysis classes present in working directory
   UserClassNames = new TList;
   UserClassNames->SetOwner(kTRUE);

   //initialise repositories, datasets, etc.
   if (!gDataRepositoryManager) {
      new KVDataRepositoryManager();
      gDataRepositoryManager->Init();
   }
   if (gDataRepositoryManager->GetListOfRepositories()->GetEntries() == 0) {
      exit(1);
   }
   ia = 0;
   GetDataAnalyser();//this will set up gBatchSystemManager

   // Creation de l'environnement d'affichage et ajout des 2 boutons a cet
   // environnement

   TGLayoutHints* eXeY = new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                           kLHintsExpandX | kLHintsExpandY,
                                           1, 1, 1, 1);
   TGLayoutHints* eX = new TGLayoutHints(kLHintsLeft | kLHintsTop |
                                         kLHintsExpandX,
                                         1, 1, 1, 1);
   TGLayoutHints* centerX = new TGLayoutHints(kLHintsCenterX | kLHintsTop,
         1, 1, 1, 1);
   TGLayoutHints* LHtopleft = new TGLayoutHints(kLHintsLeft | kLHintsTop,
         1, 1, 1, 1);
   fMainGuiWidth = gEnv->GetValue("KaliVedaGUI.MainGUIWidth", 400);
   fMainGuiHeight = gEnv->GetValue("KaliVedaGUI.MainGUIHeight", 600);

   // Creation des 3 frames dans la fenetre
   TGCompositeFrame* cfSelect = new TGCompositeFrame(this, fMainGuiWidth, 350, kVerticalFrame);

   Int_t justMode = kTextBottom | kTextRight;
   TGCompositeFrame* cf = new TGCompositeFrame(cfSelect, fMainGuiWidth, 350, kHorizontalFrame);
   // Label du Repository
   TGLabel* lab = new TGLabel(cf, "DATA REPOSITORY : ");
   lab->Resize(150, 20);
   lab->SetTextJustify(justMode);
   cf->AddFrame(lab);
   // ComboBox du Repository
   cbRepository = new TGComboBox(cf, CB_DataRepository);
   cbRepository->Select(-1);
   cbRepository->Resize(150, 20);
   cbRepository->Connect("Selected(char*)",
                         "KVDataAnalysisLauncher",
                         this,
                         "SetDataSetList(char*)");
   cf->AddFrame(cbRepository, LHtopleft);
   // cfSelect->AddFrame(cf,eX);

   // cf=new TGCompositeFrame(cfSelect,fMainGuiWidth,350,kHorizontalFrame);
   // Label du Data Set
   lab = new TGLabel(cf, "DATASET : ");
   lab->Resize(150, 20);
   lab->SetTextJustify(justMode);
   cf->AddFrame(lab, new TGLayoutHints(kLHintsLeft | kLHintsTop,
                                       20, 1, 1, 1));
   // ComboBox du Data Set
   cbDataSet = new TGComboBox(cf, CB_DataSet);
   cbDataSet->Select(-1);
   cbDataSet->Resize(150, 20);
   cbDataSet->Connect("Selected(char*)",
                      "KVDataAnalysisLauncher",
                      this,
                      "SetTaskList(char*)");
   cf->AddFrame(cbDataSet, LHtopleft);
   cfSelect->AddFrame(cf, centerX);

   cf = new TGCompositeFrame(cfSelect, fMainGuiWidth, 350, kHorizontalFrame);
   // Label du Task
   lab = new TGLabel(cf, "ANALYSIS TASK : ");
   lab->SetTextJustify(justMode);
   lab->Resize(150, 20);
   cf->AddFrame(lab, new TGLayoutHints(kLHintsLeft | kLHintsTop,
                                       1, 1, 1, 1));
   // ComboBox du Task
   cbTask = new TGComboBox(cf, CB_AnalysisTask);
   cbTask->Select(-1);
   cbTask->Resize(350, 20);
   cbTask->Connect("Selected(int)",
                   "KVDataAnalysisLauncher",
                   this,
                   "SetSystemList(int)");
   cf->AddFrame(cbTask, LHtopleft);
   cfSelect->AddFrame(cf, centerX);

   // cf=new TGCompositeFrame(cfSelect,fMainGuiWidth,350,kHorizontalFrame);
   // Label du systeme
   // lab=new TGLabel(cf,"SYSTEM : ");
   // lab->SetTextJustify(justMode);
   // lab->Resize(150,20);
   // cf->AddFrame(lab,new TGLayoutHints(kLHintsLeft|kLHintsTop,
   //                                    1,1,1,1));
   // ComboBox du systeme
   // cbSystem=new TGComboBox(cf,CB_System);
   // cbSystem->Select(-1);
   // cbSystem->Resize(250,20);
   // cbSystem->Connect("Selected(int)",
   //                       "KVDataAnalysisLauncher",
   //           this,
   //           "SetTriggersList(int)");
   //  cbSystem->Connect("Selected(int)",
   //                        "KVDataAnalysisLauncher",
   //              this,
   //              "SetRunsList(int)");
   // cf->AddFrame(cbSystem,LHtopleft);
   // cfSelect->AddFrame(cf,eX);

   // cf=new TGCompositeFrame(cfSelect,fMainGuiWidth,350,kHorizontalFrame);
   // Label du Trigger
   // lab=new TGLabel(cf,"TRIGGER : ");
   // lab->SetTextJustify(justMode);
   // lab->Resize(150,20);
   // cf->AddFrame(lab,new TGLayoutHints(kLHintsLeft|kLHintsTop,
   //                                    20,1,1,1));
   // ComboBox du systeme
   // cbTrigger=new TGComboBox(cf,CB_Trigger);
   // cbTrigger->Select(-1);
   // cbTrigger->Resize(100,20);
   // cbTrigger->Connect("Selected(int)",
   //                       "KVDataAnalysisLauncher",
   //           this,
   //           "SetTriggerRunsList(int)");
   // cf->AddFrame(cbTrigger,LHtopleft);
   // /* disable trigger selection */
   // cbTrigger->SetEnabled(kFALSE);
   cfSelect->AddFrame(cf, centerX);


   AddFrame(cfSelect, new TGLayoutHints(kLHintsLeft | kLHintsTop
                                        | kLHintsRight | kLHintsExpandX,
                                        10, 10, 1, 1));
   // Systems list
   lvSystems = new KVListView(KVDBSystem::Class(), this, fMainGuiWidth, 150);
   lvSystems->SetDataColumns(5);
   lvSystems->SetMaxColumnSize(gEnv->GetValue("KaliVedaGUI.MaxColWidth", 200));
   lvSystems->SetDataColumn(1, "Zproj");
   lvSystems->SetDataColumn(2, "Ztarget");
   lvSystems->SetDataColumn(3, "Ebeam");
   lvSystems->GetDataColumn(3)->SetDataFormat("%4.1lf");
   lvSystems->SetDataColumn(4, "#Runs", "GetNumberRuns");
   lvSystems->SetDataColumn(0, "Name");
   lvSystems->ActivateSortButtons();
   // disable context menu, Browse & multi-select functions
   lvSystems->AllowBrowse(kFALSE);
   lvSystems->AllowContextMenu(kFALSE);
   lvSystems->AllowMultipleSelection(kFALSE);
   lvSystems->Connect("SelectionChanged()", "KVDataAnalysisLauncher", this, "SystemSelectionChanged()");
   AddFrame(lvSystems, new TGLayoutHints(kLHintsExpandX,
                                         10, 10, 15, 15));

   // Frame pour la liste des runs

   TGCompositeFrame* cfRuns = new TGCompositeFrame(this, fMainGuiWidth, 350, kVerticalFrame);
   lvRuns = new KVListView(KVRunFile::Class(), cfRuns, fMainGuiWidth, 250);
   lvRuns->SetDataColumns(8);
   lvRuns->SetMaxColumnSize(gEnv->GetValue("KaliVedaGUI.MaxColWidth", 200));
   int iicc = 0;
   lvRuns->SetDataColumn(iicc++, "Run", "GetRunNumber");
   lvRuns->SetDataColumn(iicc++, "Trigger", "");
   lvRuns->SetDataColumn(iicc++, "Events", "", kTextRight);
   lvRuns->SetDataColumn(iicc++, "File", "GetName");
   lvRuns->SetDataColumn(iicc++, "Date", "GetFileWritten");
   lvRuns->SetDataColumn(iicc++, "Comments", "", kTextLeft);
   lvRuns->SetDataColumn(iicc++, "Version");
   lvRuns->SetDataColumn(iicc++, "User");
   lvRuns->ActivateSortButtons();
   // disable context menu & Browse functions
   lvRuns->AllowBrowse(kFALSE);
   lvRuns->AllowContextMenu(kFALSE);
   lvRuns->Connect("SelectionChanged()", "KVDataAnalysisLauncher", this, "UpdateListOfSelectedRuns()");
   cfRuns->AddFrame(lvRuns, new TGLayoutHints(kLHintsLeft | kLHintsTop |
                    kLHintsExpandX | kLHintsExpandY,
                    10, 10, 15, 15));

   // Boutons de selection
   TGCompositeFrame* cfSelAll = new TGCompositeFrame(cfRuns, fMainGuiWidth, 20, kHorizontalFrame);
   TGTextButton* bout = new TGTextButton(cfSelAll, "Select All");
   bout->SetToolTipText("Select all runs for the analysis.", TTDELAY);
   bout->Connect("Clicked()",
                 "KVDataAnalysisLauncher",
                 this,
                 "SelectAll()");
   cfSelAll->AddFrame(bout, eX);
   bout = new TGTextButton(cfSelAll, "Deselect All");
   bout->SetToolTipText("Deselect all runs.", TTDELAY);
   bout->Connect("Clicked()",
                 "KVDataAnalysisLauncher",
                 this,
                 "DeselectAll()");
   cfSelAll->AddFrame(bout, eX);
   bout = new TGTextButton(cfSelAll, "Runlist");
   bout->SetToolTipText("Enter list of runs to analyse.", TTDELAY);
   bout->Connect("Clicked()",
                 "KVDataAnalysisLauncher",
                 this,
                 "EnterRunlist()");
   cfSelAll->AddFrame(bout, eX);

   cfRuns->AddFrame(cfSelAll, eX);

   // label des runs selectionnes
   selectedRuns = new TGLabel(cfRuns, "Selected Runs :");
   cfRuns->AddFrame(selectedRuns, eX);

   AddFrame(cfRuns, eXeY);

   // UserClass and DataSelector et nombre d'evenements
   cfAnalysis = new TGCompositeFrame(this, fMainGuiWidth, 20, kVerticalFrame);
   cf = new TGCompositeFrame(cfAnalysis, fMainGuiWidth, 20, kHorizontalFrame);
   // Label for User Class name
   fUserClassLabel = new TGLabel(cf, "User Class");
   cf->AddFrame(fUserClassLabel, eX);
   // Label du data selector (only for KVINDRAReconDataAnalyser)
   fDataSelectorLabel = new TGLabel(cf, "-");
   cf->AddFrame(fDataSelectorLabel, eX);
   // Label du Task
   lab = new TGLabel(cf, "Number of events to read");
   cf->AddFrame(lab, eX);

   cfAnalysis->AddFrame(cf, eX);

   cf = new TGCompositeFrame(cfAnalysis, fMainGuiWidth, 20, kHorizontalFrame);
   //teSelector=new TGTextEntry(cf,"");
   //teSelector->Resize(233,20);
   //teSelector->SetToolTipText("Enter the user analysis class name",TTDELAY);
   //cf->AddFrame(teSelector,eX);
   cbUserClass = new TGComboBox(cf);
   cbUserClass->Select(-1);
   cbUserClass->Resize(150, 20);
   cf->AddFrame(cbUserClass, eX);
   cbUserClass->Connect("Selected(char*)", "KVDataAnalysisLauncher", this,
                        "UserClassSelected(char*)");
   btEditClass = new TGPictureButton(cf, "query_new.xpm");
   btEditClass->SetEnabled(kFALSE);
   btEditClass->Connect("Clicked()", "KVDataAnalysisLauncher", this, "EditUserClassFiles()");
   btEditClass->SetToolTipText(Form("Open analysis class source files in %s", gSystem->Getenv("EDITOR")), TTDELAY);
   cf->AddFrame(btEditClass, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

   teDataSelector = new TGTextEntry(cf, "");
   //teDataSelector->Resize(233,20);
   teDataSelector->SetToolTipText("Enter the KVDataSelector class name.", TTDELAY);
   cf->AddFrame(teDataSelector, eX);

   teNbToRead = new TGNumberEntry(cf, 0);
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMSTYLE
   teNbToRead->SetFormat(TGNumberFormat::kNESInteger, teNbToRead->GetNumAttr());
#else
   teNbToRead->SetNumStyle(TGNumberFormat::kNESInteger);
#endif
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMATTR
   teNbToRead->SetFormat(teNbToRead->GetNumStyle(), TGNumberFormat::kNEANonNegative);
#else
   teNbToRead->SetNumAttr(TGNumberFormat::kNEANonNegative);
#endif
   teNbToRead->GetNumberEntry()->SetToolTipText("Enter the number of events to read.", TTDELAY);
   //teNbToRead->Resize(233,20);
   cf->AddFrame(teNbToRead, eX);

   cfAnalysis->AddFrame(cf, eX);

   this->AddFrame(cfAnalysis, eX);
   // user analysis class options
   cf = new TGCompositeFrame(this, fMainGuiWidth, 20, kHorizontalFrame);
   lab = new TGLabel(cf, "User class options : ");
   lab->SetTextJustify(justMode);
   lab->Resize(150, 20);
   cf->AddFrame(lab, new TGLayoutHints(kLHintsLeft | kLHintsTop,
                                       1, 1, 1, 1));
   teUserOptions = new TGTextEntry(cf, "");
   //teDataSelector->Resize(233,20);
   teUserOptions->SetToolTipText("Comma-separated list of options for user analysis class: PAR1=VAL1,PAR2=VAL2,etc.", TTDELAY);
   cf->AddFrame(teUserOptions, eX);
   AddFrame(cf, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 10, 10, 2, 2));

   cf = new TGCompositeFrame(this, fMainGuiWidth, 20, kHorizontalFrame);
   // Frame for the user's libraries
   bout = new TGTextButton(cf, "User's libraries", B_Libs);
   bout->Connect("Clicked()",
                 "KVDataAnalysisLauncher",
                 this,
                 "SetUserLibraries()");
   cf->AddFrame(bout, eX);

   bout = new TGTextButton(cf, "User's includes", B_Incs);
   bout->Connect("Clicked()",
                 "KVDataAnalysisLauncher",
                 this,
                 "SetUserIncludes()");
   cf->AddFrame(bout, eX);

   this->AddFrame(cf, eX);

   cf = new TGCompositeFrame(this, fMainGuiWidth, 20, kHorizontalFrame);
   TGVerticalFrame* RunMode_frame = new TGVerticalFrame(cf, fMainGuiWidth / 2.,  100);
   // Radio buttons for the running mode
   TString tit = "Running Mode";
   TGHButtonGroup* gb = new TGHButtonGroup(RunMode_frame, tit);
   rbInteractive = new TGRadioButton(gb, "Now!  ");
   rbInteractive->SetToolTipText("Run the analysis straight away", TTDELAY);
   rbBatch = new TGRadioButton(gb, "Batch");
   rbBatch->
   SetToolTipText("Submit the analysis as a batch job", TTDELAY);
   RunMode_frame->AddFrame(gb, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
   gb->Connect("Clicked(Int_t)", "KVDataAnalysisLauncher", this, "SetBatch()");

   TGHorizontalFrame* hf913 = new TGHorizontalFrame(RunMode_frame, 600, 20);
   TGLabel* lab586 = new TGLabel(hf913, "Runs per batch job : ");
   hf913->AddFrame(lab586, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 15, 2, 2));
   runsPerJob = new TGNumberEntry(hf913, 1.0, 5, 0, TGNumberFormat::kNESInteger,
                                  TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, 1.0, 1.0);
   runsPerJob->GetNumberEntry()->SetToolTipText("Number of runs to analyse in each batch job", TTDELAY);
   //teNbToRead->Resize(233,20);
   hf913->AddFrame(runsPerJob, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2));
   RunMode_frame->AddFrame(hf913, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   cf->AddFrame(RunMode_frame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

   justMode = kTextCenterY | kTextLeft;
   TGCompositeFrame* cf2 = new TGCompositeFrame(cf, fMainGuiWidth / 2., 20, kVerticalFrame);
   TGCompositeFrame* cf3 = new TGCompositeFrame(cf2, fMainGuiWidth / 2., 20, kHorizontalFrame);
   lab = new TGLabel(cf3, " Batch Name ");
   lab->SetTextJustify(justMode);
   cf3->AddFrame(lab, new TGLayoutHints(kLHintsLeft, 2, 15, 2, 2));
   chIsBatchNameAuto = new TGCheckButton(cf3, " Auto");
   chIsBatchNameAuto->SetTextJustify(justMode);
   chIsBatchNameAuto->
   SetToolTipText("Builds the batch name automatically by using the Batch Name Format.", TTDELAY);
   chIsBatchNameAuto->Connect("Clicked()",
                              "KVDataAnalysisLauncher", this, "SetBatchNameAuto()");
   cf3->AddFrame(chIsBatchNameAuto, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   cf2->AddFrame(cf3, new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 1, 1));
   teBatchName = new TGTextEntry(cf2, "");
   teBatchName->SetToolTipText("Enter batch name.", TTDELAY);
   cf2->AddFrame(teBatchName, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   teBatchName->Resize(300, 20);
   lab = new TGLabel(cf2, " Batch Name Format");
   lab->SetTextJustify(justMode);
   cf2->AddFrame(lab, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   teBatchNameFormat = new TGTextEntry(cf2, "");
   teBatchNameFormat->
   SetToolTipText(Form("%s %s",
                       "Enter batch name format.",
                       "Examples : $UserClass_$System_$Date or $System_$DataSelector"),
                  TTDELAY);
   cf2->AddFrame(teBatchNameFormat, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   teBatchNameFormat->Resize(300, 20);
   cf->AddFrame(cf2, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

   this->AddFrame(cf, eX);

   // Batch parameters
   //if the default batch system does not have time/memory/disk parameters,
   //we do not display the corresponding widgets
   teBatchMemory = 0;
   teBatchDisk = 0;
   teBatchTime = 0;
   if (gBatchSystemManager->GetDefaultBatchSystem()->IsA()->GetMethodAllAny("SetJobTime")) {
      withBatchParams = kTRUE;
      TGCompositeFrame* cfBatchPar = new TGCompositeFrame(this, fMainGuiWidth, 20, kVerticalFrame);
      cf = new TGCompositeFrame(cfBatchPar, fMainGuiWidth, 20, kHorizontalFrame);
      fBatchMemLab = new TGLabel(cf, "Batch Memory");
      cf->AddFrame(fBatchMemLab, eX);
      fBatchDiskLab = new TGLabel(cf, "Batch Disk");
      cf->AddFrame(fBatchDiskLab, eX);
      fBatchTimeLab = new TGLabel(cf, "Batch Time");
      cf->AddFrame(fBatchTimeLab, eX);
      cfBatchPar->AddFrame(cf, eX);
      cf = new TGCompositeFrame(cfBatchPar, fMainGuiWidth, 20, kHorizontalFrame);
      teBatchMemory = new TGTextEntry(cf, "1G");
      teBatchMemory->SetToolTipText("Enter max memory per job (xK/xM/xG)", TTDELAY);
      teBatchMemory->SetAlignment(kTextRight);
      cf->AddFrame(teBatchMemory, eX);
      teBatchDisk = new TGTextEntry(cf, "200MB");
      teBatchDisk->SetToolTipText("Enter max scratch disk per job (xK/xM/xG)", TTDELAY);
      teBatchDisk->SetAlignment(kTextRight);
      cf->AddFrame(teBatchDisk, eX);
#ifdef USE_KVTIMEENTRY
      teBatchTime = new KVTimeEntry(cf);
      teBatchTime->GetTextEntry()->SetToolTipText("Enter max CPU time per job (ss/mn:ss/hh:mn:ss)", TTDELAY);
#else
      teBatchTime = new TGNumberEntry(cf);
      teBatchTime->GetNumberEntry()->SetToolTipText("Enter max CPU time per job in seconds", TTDELAY);
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMSTYLE
      teBatchTime->SetFormat(TGNumberFormat::kNESInteger, teBatchTime->GetNumAttr());
#else
      teBatchTime->SetNumStyle(TGNumberFormat::kNESInteger);
#endif
#ifdef __WITHOUT_TGNUMBERENTRY_SETNUMATTR
      teBatchTime->SetFormat(teBatchTime->GetNumStyle(), TGNumberFormat::kNEANonNegative);
#else
      teBatchTime->SetNumAttr(TGNumberFormat::kNEANonNegative);
#endif
#endif
      teBatchTime->SetTime(2, 30, 0);
      cf->AddFrame(teBatchTime, eX);
      cfBatchPar->AddFrame(cf, eX);
      this->AddFrame(cfBatchPar, eX);
   } else {
      withBatchParams = kFALSE;
   }

   // Process et Quit
#ifdef KVDAL_DEBUG
   cout << "Creation Process/Quit" << endl;
#endif
   TGCompositeFrame* cfProcess = new TGCompositeFrame(this, fMainGuiWidth, 20, kHorizontalFrame);
   // Bouton de process
   bout = new TGTextButton(cfProcess, "&Process", B_Process);
   bout->SetToolTipText("Run the analysis.", TTDELAY);
   bout->Connect("Clicked()", "KVDataAnalysisLauncher", this, "Process()");
   // bout->Associate(this);
   cfProcess->AddFrame(bout, eX);
   // Bouton de sortie
   bout = new TGTextButton(cfProcess, "&Quit", B_Quit);
   bout->SetToolTipText("Close GUI and quit.", TTDELAY);
   bout->Connect("Clicked()", "KVDataAnalysisLauncher", this, "Exit()");
   // bout->Associate(this);
   cfProcess->AddFrame(bout, eX);

   this->AddFrame(cfProcess, eX);
   // On affiche tout le monde maintenant
   MapSubwindows();

   Resize(GetDefaultSize());

   SetWindowName(Form("KaliVeda Analysis Launcher on %s", gSystem->HostName()));
   SetIconName(Form("KaliVeda Analysis Launcher on %s", gSystem->HostName()));

   MapWindow();
   SetWMSize(fMainGuiWidth, fMainGuiHeight);

   SetRepositoryList();

   FillListOfUserClasses();
   //fill drop down list of user classes in working directory
   SetUserClassList();
   checkCompilation = kFALSE;

   // Reset last known state of interface
   TString tmp(GetResource("Repository", ""));
   SetRepository(tmp.Data());

   tmp = GUIenv->GetValue("KVDataAnalysisLauncher.BatchName", "");
   teBatchName->SetText(tmp.Data());

   tmp = GUIenv->GetValue("KVDataAnalysisLauncher.BatchNameFormat", "");
   teBatchNameFormat->SetText(tmp.Data());
   if (GUIenv->GetValue("KVDataAnalysisLauncher.Batch", kFALSE))
      rbBatch->SetState(kButtonDown);
   else
      rbInteractive->SetState(kButtonDown);
   runsPerJob->SetNumber(GUIenv->GetValue("KVDataAnalysisLauncher.RunsPerJob", 1));

   if (withBatchParams) {
      teBatchMemory->SetText(GUIenv->GetValue("KVDataAnalysisLauncher.BatchMemory", "1G"));
      teBatchDisk->SetText(GUIenv->GetValue("KVDataAnalysisLauncher.BatchDisk", "200M"));
      teBatchTime->SetIntNumber(GUIenv->GetValue("KVDataAnalysisLauncher.BatchTime", 9000));
   }

   fUserLibraries = GUIenv->GetValue("KVDataAnalysisLauncher.UserLibraries", "");
   fUserIncludes = GUIenv->GetValue("KVDataAnalysisLauncher.UserIncludes", "");

   // Connections are made when all the other widgets are mapped
   //teSelector->Connect("TextChanged(const char*)",
   //                    "KVDataAnalysisLauncher",this,"SetAutoBatchName()");
   teDataSelector->Connect("TextChanged(const char*)",
                           "KVDataAnalysisLauncher", this, "SetAutoBatchName()");
   teBatchNameFormat->Connect("TextChanged(const char*)",
                              "KVDataAnalysisLauncher", this, "SetAutoBatchName()");

   if (GUIenv->GetValue("KVDataAnalysisLauncher.AutoBatchName", kFALSE)) {
      chIsBatchNameAuto->SetState(kButtonDown, kFALSE);
   } else {
      chIsBatchNameAuto->SetState(kButtonUp, kFALSE);
   }
   SetBatch();

}

//__________________________________________
KVDataAnalysisLauncher::~KVDataAnalysisLauncher()
{
   // Destructeur
   if (ia) delete ia;
   if (GUIenv) delete GUIenv;
   delete ResourceNames;
   delete UserClassNames;
}

// On va maintenant agir si un evenemnt se produit
//__________________________________________
Bool_t KVDataAnalysisLauncher::ProcessMessage(Long_t msg, Long_t , Long_t)
{
   // Process messages
#ifdef KVDAL_DEBUG
   cout << "KVDataAnalysisLauncher::ProcessMessage " << endl;
   cout << msg << " : " << par1 << " / " << par2 << endl;
#endif
   switch (GET_MSG(msg)) {
      //       case kC_CONTAINER:
      //          switch (GET_SUBMSG(msg)) {
      //             case kCT_ITEMCLICK:
      //                if (par1 == kButton1) {
      //                   cout << "Now with " << lcRuns->NumSelected() << " selected items" << endl;
      //                   if (lcRuns->NumSelected() > 0) {
      //                      UpdateListOfSelectedRuns();
      //                   }
      //                   else {
      //                      ClearListOfSelectedRuns();
      //                   }
      //                }
      //                break;
      //
      //             case kCT_SELCHANGED:
      //                cout << "SELCHANGED: number selected = " << par2 << endl;
      //                break;
      //
      //             default:
      //                break;
      //          }

      default:
         break;
   }
   return kTRUE;
}

//__________________________________________
void KVDataAnalysisLauncher::SetRepositoryList(void)
{
   // Sets the list of all possible repositories in the repository combo box
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(cbRepository);
#else
   cbRepository->RemoveAll();
#endif
   cbRepository->Select(-1);
   TIter next((TList*)gDataRepositoryManager->GetListOfRepositories());
   TObject* o = 0;
   Int_t i = 0;
   while ((o = next())) {
      //if(((KVDataRepository *)o)->GetDataSetManager()->GetNavailable())
      cbRepository->AddEntry(o->GetName(), i);
      i++;
   }

   cbRepository->Layout();

}

//__________________________________________
void KVDataAnalysisLauncher::SetDataSetList(Char_t* repository)
{
   // Sets the list of all available data sets in the data sets combo box
   SetResource("Repository", repository);

   //TString ds=GetDataSet();
   TString ds = GetSavedResource("DataSet", "");

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(cbDataSet);
#else
   cbDataSet->RemoveAll();
#endif
   cbDataSet->Select(-1);
   gDataRepositoryManager->GetRepository(repository)->cd();
   Int_t nbds = gDataSetManager->GetNavailable();
   Int_t i = 0;
   while (i < nbds) {
      cbDataSet->AddEntry(gDataSetManager->GetAvailableDataSet(i + 1)->GetName(), i);
      i++;
   }

   cbDataSet->Layout();

   if (ds.Length()) {
      SetDataSet(ds.Data());
   } else {
      SetTask();
   }

}

//__________________________________________
void KVDataAnalysisLauncher::SetTaskList(Char_t* dataset)
{
   // Sets the list of all possible tasks in the tasks combo box
   // Called when a new dataset is selected in the dropdown list

   SetResource("DataSet", dataset);

   //Info("SetTaskList", "Called with dataset = %s", dataset);

   TString ds = GetSavedResource("Task", "");

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(cbTask);
#else
   cbTask->RemoveAll();
#endif
   cbTask->Select(-1);
#ifdef KVDAL_DEBUG
   cout << "DataSet : [" << dataset << "]" <<  endl;
#endif
   gDataSetManager->GetDataSet(dataset)->cd();
   Int_t nbt = gDataSet->GetNtasks();

   GetDataAnalyser()->SetDataSet(gDataSet);
   //no systems defined for dataset ?
   //  noSystems=(!gIndraDB || !gIndraDB->GetSystems()->GetSize());

   noSystems = (!gDataBase || !((KVDBTable*)((KVDataBase*)gROOT->FindObjectAny(gDataSet->GetName()))->GetTable("Systems"))->GetRecords()->GetSize());
   if (noSystems) lvSystems->RemoveAll();

   Int_t i = 0;
   while (i < nbt) {
      cbTask->AddEntry(gDataSet->GetAnalysisTask(i + 1)->GetType(), i);
      i++;
   }

   cbTask->Layout();

   if (ds.Length()) {
      SetTask(ds.Data());
   } else {
      SetSystem();
   }

   //if no systems are defined for dataset, need to set list of triggers now
   //if(noSystems) SetTriggersList(0);
}

KVDataAnalyser* KVDataAnalysisLauncher::GetDataAnalyser(KVDataAnalysisTask* task)
{
   //Get analyser for task
   //If task = 0 we return the current analyser

   if (!task) {
      if (!ia) ia = new KVDataAnalyser;
   } else {
      if (ia) delete ia;
      ia = KVDataAnalyser::GetAnalyser(task->GetDataAnalyser());
      if (!ia) ia = new KVDataAnalyser;
      ia->SetAnalysisTask(task);
   }
   return ia;
}

//__________________________________________
void KVDataAnalysisLauncher::SetSystemList(Int_t itask)
{
   // Sets the list of all possible systems in the system list
   // Called every time a task is selected

   KVDataAnalysisTask* task = gDataSet->GetAnalysisTask(itask + 1);

   GetDataAnalyser(task);

   SetResource("Task", task->GetTitle());

   //if needed, set the user class and KVDataSelector fields and/or disable them
   fDataSelectorLabel->SetText("-");
   teDataSelector->SetEnabled(kFALSE);
   teDataSelector->SetToolTipText("");
   if (!task->WithUserClass()) {
      //no user class required
      //teSelector->SetEnabled(kFALSE);
      //teSelector->SetText(task->GetUserBaseClass());
      //teSelector->SetToolTipText("Default analysis class name");
      DisableUserClassList();
   } else {
      //user class required
      //teSelector->SetEnabled(kTRUE);
      //teSelector->SetToolTipText(Form("Enter name of user analysis class derived from %s", task->GetUserBaseClass()));
      EnableUserClassList();
      //is data selector an option ?
      if (GetDataAnalyser()->IsA()->GetMethodAllAny("SetKVDataSelector")) {
         fDataSelectorLabel->SetText("Data Selector");
         teDataSelector->SetEnabled(kTRUE);
         teDataSelector->SetToolTipText("Enter name of data selector class derived from KVDataSelector");
      }
   }
   //update display
   cfAnalysis->Layout();

   TString ds = GetSavedResource("System", "");
   lastSelectedSystem = 0;
   if (!noSystems) {
      TList* sys_list = gDataSet->GetListOfAvailableSystems(task);
      if (sys_list) {
         lvSystems->Display(sys_list);
         delete sys_list;
      }
   }
   if (ds.Length()) {
      SetSystem(ds.Data());
   } else {
      //Info("SetSystemList","no selected system");
      SetRunsList();
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetRunsList()
{
   // Sets the list of all available runs in the runs list box

   SetResource("Trigger", "All");
   listOfRuns.Clear();
   listOfSystemRuns.Clear();
   if (entryMax > -1) {
      lvRuns->RemoveAll();
      entryMax = -1;
   }

   KVDataAnalysisTask* task = gDataSet->GetAnalysisTask(cbTask->GetSelected() + 1);
   KVDBSystem* system = 0;
   if (!noSystems) {
      // case where systems are defined for dataset and user has
      // selected a specific system
      system = lastSelectedSystem;
      GetDataAnalyser()->SetSystem(system);
      if (!system) {
         // no system selected
         // clear runs list
         SetRuns();
         return;
      }
   }
   //Info("SetRunsList","system=%p",system);

   if (IsBatchNameAuto()) {
      SetAutoBatchName();
   }

   //Setting name of system in ressources file
   if (!noSystems) {
      // dataset with defined systems
      if (system) {
         // user has chosen a system
         SetResource("System", system->GetName());
      } else {
         // user chose "All" for system
         SetResource("System", "All");
      }
   } else {
      // no systems defined for dataset
      SetResource("System", "Unknown");
   }

   KVNumberList run_list = gDataSet->GetRunList(task->GetPrereq(), system);
   lvRuns->RemoveAll();
   list_of_runs.reset(gDataSet->GetListOfAvailableSystems(task, system));
   entryMax = list_of_runs->GetEntries();
   listOfSystemRuns = run_list;
   lvRuns->Display(list_of_runs.get());

   TString ds = GetSavedResource("RunsList", "");
   //Info("SetRunsList", "SetRuns");
   SetRuns(ds.Data());

   // Set saved user class, data selector, number of events for current
   // repository, dataset, task, system, trigger & runs
   ds = GetSavedResource("UserClass", "");
   SetUserClass(ds.Data());
   ds = GetSavedResource("KVDataSelector", "");
   teDataSelector->SetText(ds.Data());
   ds = GetSavedResource("NbEventsToRead", "");
   teNbToRead->SetIntNumber(ds.Atoi());

}


//__________________________________________
void KVDataAnalysisLauncher::SelectAll(void)
{
   // Select all runs currently in the displayed list of runs
   lvRuns->SelectAll();
   UpdateListOfSelectedRuns();
}

//__________________________________________
void KVDataAnalysisLauncher::DeselectAll(void)
{
   // Deselect all runs currently in the displayed list of runs
   lvRuns->UnSelectAll();
   UpdateListOfSelectedRuns();
}

//__________________________________________
void KVDataAnalysisLauncher::Process(void)
{
   // Run the analysis task

   TString oriIncludePath = gSystem->GetIncludePath();

   if (gDataRepository->IsRemote()) {
      cout << "Checking connection to remote repository." << endl;
      if (!gDataRepository->IsConnected()) {
         cout << "Connection to server refused" << endl;
         cout << "Process aborted." << endl;
         WarningBox("Connection refused", "Connection to server refused\nProcess aborted.");
         return;
      }
   }

   KVDataAnalysisTask* task = gDataSet->GetAnalysisTask(cbTask->GetSelected() + 1);
   KVDataAnalyser* datan = GetDataAnalyser(task);

   //set global pointer to analyser
   gDataAnalyser = datan;

   datan->SetDataSet(gDataSet);
   datan->SetAnalysisTask(task);
   if (listOfRuns.GetNValues()) {
      datan->SetRuns(listOfRuns, kFALSE);
   } else {
      WarningBox("Empty Run List", "The list of runs to process is empty.");
      return;
   }

   if (fUserIncludes.Length()) {
      datan->SetUserIncludes(fUserIncludes.Data());
   }
   if (fUserLibraries.Length()) {
      datan->SetUserLibraries(fUserLibraries.Data());
   }

   //Need a user class for the analysis ?
   TString kvs(GetUserClass());
   if (task->WithUserClass()) {
      //read user's class name from input box
      if (kvs.Length()) {
         datan->SetUserClassOptions(teUserOptions->GetText());
         //Info("Process","setting user class now for analyser %s check=%d", datan->ClassName(),checkCompilation);
         datan->SetUserClass(kvs.Data(), checkCompilation);
         if (datan->IsUserClassValid())
            checkCompilation = kFALSE;
         else {
            // compilation failed. abort processing.
            delete ia;
            ia = 0;
            checkCompilation = kTRUE;
            if (WarningBox("Compilation failed", "Please correct mistakes in user analysis class", kTRUE)) EditUserClassFiles();
            return;
         }
      } else {
         delete ia;
         ia = 0;
         checkCompilation = kTRUE;
         WarningBox("No User Class", "Please enter the user analysis class name.");
         return;
      }
   } else if (strcmp(task->GetUserBaseClass(), "")) {
      //task with default "user" class (i.e. UserClass=no but UserClass.BaseClass!="")
      datan->SetUserClass(task->GetUserBaseClass(), kFALSE);
   }
   if (datan->InheritsFrom("KVINDRAReconDataAnalyser"))
      ((KVINDRAReconDataAnalyser*)datan)->SetKVDataSelector(teDataSelector->GetText());
   Long64_t nbEventRead = (Long64_t)teNbToRead->GetIntNumber();
   // if in batch and nbEventRead>0, ask confirmation
   if (IsBatch() && nbEventRead) {
      if (!WarningBox("Read all events in batch mode?",
                      "This will submit batch jobs which will not read all events.\nAre you sure that is what you want?",
                      kTRUE)) {
         delete ia;
         ia = 0;
         return;
      }
   }
   datan->SetNbEventToRead((Long64_t)teNbToRead->GetIntNumber());
   SetResource("RunsList", listOfRuns.AsString());
   SetResource("UserClassOptions", teUserOptions->GetText());
   SetResource("KVDataSelector", teDataSelector->GetText());
   SetResource("NbEventsToRead", Form("%.0f", teNbToRead->GetNumber()));
   GUIenv->SetValue("KVDataAnalysisLauncher.BatchName", teBatchName->GetText());
   if (withBatchParams) {
      GUIenv->SetValue("KVDataAnalysisLauncher.BatchMemory", teBatchMemory->GetText());
      GUIenv->SetValue("KVDataAnalysisLauncher.BatchDisk", teBatchDisk->GetText());
      GUIenv->SetValue("KVDataAnalysisLauncher.BatchTime", (Int_t)teBatchTime->GetIntNumber());
   }
   GUIenv->SaveLevel(kEnvUser);
   if (IsBatch()) {
      gBatchSystemManager->GetDefaultBatchSystem()->cd();
      gBatchSystem->Clear();
      if (IsBatchNameAuto()) gBatchSystem->SetJobName(teBatchNameFormat->GetText());
      else gBatchSystem->SetJobName(teBatchName->GetText());
      gBatchSystem->SetJobMemory(teBatchMemory->GetText());
      gBatchSystem->SetJobDisk(teBatchDisk->GetText());
      gBatchSystem->SetJobTime((Int_t)teBatchTime->GetIntNumber());
      gBatchSystem->SetRunsPerJob(runsPerJob->GetNumber());
      gBatchSystem->SetMultiJobsMode(runsPerJob->GetNumber() < listOfRuns.GetNValues());
      datan->SetBatchSystem(gBatchSystem);
   } else {
      datan->SetBatchSystem(0);
   }
   datan->Run();

   //if no batch system GUI window is already open, open it
   //if(IsBatch() && !KVBatchSystemGUI::IsOpen()) new KVBatchSystemGUI;

   gSystem->SetIncludePath(oriIncludePath.Data());
}

//__________________________________________
const Char_t* KVDataAnalysisLauncher::GetRepository(void)
{
   if (TGTextLBEntry* e = (TGTextLBEntry*)cbRepository->GetSelectedEntry()) {
      return e->GetText()->GetString();
   } else {
      return "";
   }
}

//__________________________________________
const Char_t* KVDataAnalysisLauncher::GetDataSet(void)
{
   if (TGTextLBEntry* e = (TGTextLBEntry*)cbDataSet->GetSelectedEntry()) {
      return e->GetText()->GetString();
   } else {
      return "";
   }
}

//__________________________________________
const Char_t* KVDataAnalysisLauncher::GetTask(void)
{
   if (TGTextLBEntry* e = (TGTextLBEntry*)cbTask->GetSelectedEntry()) {
      return e->GetText()->GetString();
   } else {
      return "";
   }
}

//__________________________________________
const Char_t* KVDataAnalysisLauncher::GetSystem(void)
{
   if (noSystems) return "";
   KVDBSystem* sys = (KVDBSystem*)lvSystems->GetLastSelectedObject();
   if (sys)
      return sys->GetName();
   else
      return "";
}


//__________________________________________
const Char_t* KVDataAnalysisLauncher::GetRuns(void)
{
   return listOfRuns.AsString();
}

//__________________________________________
void KVDataAnalysisLauncher::SetRepository(const Char_t* r)
{
   TGLBEntry* e = 0;
   if ((e = cbRepository->FindEntry(r))) {
      Int_t i = e->EntryId();
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
      cbRepository->Select(i);
#else
      cbRepository->Select(i, kFALSE);
#endif
      SetDataSetList((Char_t*)r);
   } else {
      SetDataSet();
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetDataSet(const Char_t* r)
{
   if (!strcmp(r, "")) {
      //remove all datasets because no repository has been chosen yet
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
      RemoveAll(cbDataSet);
#else
      cbDataSet->RemoveAll();
#endif
      cbDataSet->Select(-1);
      SetResource("Repository", "");
      SetTask();
   } else {
      TGLBEntry* e = 0;
      if ((e = cbDataSet->FindEntry(r))) {
         Int_t i = e->EntryId();
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
         cbDataSet->Select(i);
#else
         cbDataSet->Select(i, kFALSE);
#endif
         SetTaskList((Char_t*)r);
      } else {
         SetTask();
      }
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetTask(const Char_t* r)
{
   if (!strcmp(r, "")) {
      //remove all tasks from list because no dataset chosen yet
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
      RemoveAll(cbTask);
#else
      cbTask->RemoveAll();
#endif
      cbTask->Select(-1);
      SetResource("DataSet", "");
      SetSystem();
   } else {
      TGLBEntry* e = 0;
      if ((e = cbTask->FindEntry(r))) {
         Int_t i = e->EntryId();
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
         cbTask->Select(i);
#else
         cbTask->Select(i, kFALSE);
#endif
         SetSystemList(i);
      } else {
         SetSystem();
      }
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetSystem(const Char_t* r)
{
   //Info("SetSystem","system=%s",r);
   if (!strcmp(r, "")) {
      //remove all systems from list because no task chosen yet
      lvSystems->RemoveAll();
      //if( !noSystems ) SetTrigger();
      SetResource("Task", "");
      //empty list of analysis classes and disable it
      DisableUserClassList();
      //Info("SetSystem","SetRuns()");
      SetRuns();
      lvRuns->RemoveAll();
   } else {
      lvSystems->ActivateItemWithColumnData("Name", r);
      SystemSelectionChanged();
   }
}

void KVDataAnalysisLauncher::SystemSelectionChanged()
{
   KVDBSystem* system = (KVDBSystem*)lvSystems->GetLastSelectedObject();
   //Info("SystemSelectionChanged","system=%p lastSelectedSystem=%p",system,lastSelectedSystem);
   if (system == lastSelectedSystem) return;
   lastSelectedSystem = system;
   GetDataAnalyser()->SetSystem(system);
   SetRunsList();
}

//__________________________________________
void KVDataAnalysisLauncher::SetRuns(const Char_t* r)
{
   if (!strcmp(r, "")) {
      ClearListOfSelectedRuns();
      SetResource("Trigger", "All");
      SetResource("RunsList", "");
   } else {
      listOfRuns.SetList(r);
      listOfRuns.Inter(listOfSystemRuns);

      if (listOfRuns.GetNValues()) {
         lvRuns->ActivateItemsWithColumnData("Run", listOfRuns);
         SetResource("RunsList", listOfRuns.AsString());
      }
      UpdateListOfSelectedRuns();
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetBatch(void)
{
   // Set the resource KVDataAnalysisLauncher.Batch according
   // to whether radio button 'Batch' is down or up
   // Enable or disable 'runs per job' number entry accordingly.
   // If enabled, we set the resource KVDataAnalysisLauncher.RunsPerJob
   // according to the current value.
   GUIenv->SetValue("KVDataAnalysisLauncher.Batch", IsBatch());
   if (IsBatch()) {
      runsPerJob->SetState(kTRUE);
      GUIenv->SetValue("KVDataAnalysisLauncher.RunsPerJob", (Int_t)runsPerJob->GetNumber());
      if (!chIsBatchNameAuto->IsEnabled()) {
         chIsBatchNameAuto->SetEnabled(kTRUE);
      }
      SetBatchNameAuto();
      if (teBatchMemory && teBatchTime && teBatchDisk) {
         teBatchMemory->SetState(kTRUE);
         teBatchTime->SetState(kTRUE);
         teBatchDisk->SetState(kTRUE);
      }
   } else {
      runsPerJob->SetState(kFALSE);
      teBatchName->SetState(kFALSE);
      teBatchNameFormat->SetState(kFALSE);
      chIsBatchNameAuto->SetEnabled(kFALSE);
      if (teBatchMemory && teBatchTime && teBatchDisk) {
         teBatchMemory->SetState(kFALSE);
         teBatchTime->SetState(kFALSE);
         teBatchDisk->SetState(kFALSE);
      }
   }
   GUIenv->SaveLevel(kEnvUser);
}

//__________________________________________
void KVDataAnalysisLauncher::SetBatchNameAuto(void)
{
   // Set the resource value according to the check box chIsBatch
   if (IsBatchNameAuto()) {
      teBatchName->SetState(kFALSE);
      teBatchNameFormat->SetState(kTRUE);
   } else {
      teBatchName->SetState(kTRUE);
      teBatchNameFormat->SetState(kFALSE);
   }
   GUIenv->SetValue("KVDataAnalysisLauncher.AutoBatchName", IsBatchNameAuto());
   GUIenv->SaveLevel(kEnvUser);
   SetAutoBatchName();
}

//__________________________________________
void KVDataAnalysisLauncher::SetAutoBatchName(void)
{
   // Set the automatic batch name
   if (!IsBatchNameAuto()) {
      return;
   }
   if (strcmp(teBatchNameFormat->GetText(),
              GUIenv->GetValue("KVDataAnalysisLauncher.BatchNameFormat", ""))) {
      GUIenv->SetValue("KVDataAnalysisLauncher.BatchNameFormat",
                       teBatchNameFormat->GetText());
      GUIenv->SaveLevel(kEnvUser);
   }

   // check if the pointers are valid before doing anything...
   if (GetDataAnalyser()) {
      if (GetDataAnalyser()->GetAnalysisTask()) {
         if (GetDataAnalyser()->GetAnalysisTask()->WithUserClass()) {
            GetDataAnalyser()->SetUserClass(GetUserClass() , kFALSE);
            if (GetDataAnalyser()->InheritsFrom("KVINDRAReconDataAnalyser"))
               ((KVINDRAReconDataAnalyser*)GetDataAnalyser())->SetKVDataSelector(teDataSelector->GetText());
         }
      }
      teBatchName->SetText(GetDataAnalyser()->ExpandAutoBatchName(teBatchNameFormat->GetText()));
   }
}

//__________________________________________
void KVDataAnalysisLauncher::SetUserLibraries(void)
{
   // Set the User's libraries
   TString ori = fUserLibraries.Data();
   new KVGFileList(fUserLibraries, "User's Libraries",
                   gClient->GetRoot(), this);
   GUIenv->SetValue("KVDataAnalysisLauncher.UserLibraries", fUserLibraries.Data());
   GUIenv->SaveLevel(kEnvUser);
   checkCompilation = ori.CompareTo(fUserLibraries);
}

//__________________________________________
void KVDataAnalysisLauncher::SetUserIncludes(void)
{
   // Set the User's includes
   TString ori = fUserIncludes.Data();
   new KVGDirectoryList(fUserIncludes, "User's Includes",
                        gClient->GetRoot(), this);
   GUIenv->SetValue("KVDataAnalysisLauncher.UserIncludes", fUserIncludes.Data());
   GUIenv->SaveLevel(kEnvUser);
   checkCompilation = ori.CompareTo(fUserIncludes);
}


//__________________________________________
Bool_t KVDataAnalysisLauncher::WarningBox(const char* title, const char* msg, Bool_t confirm)
{
   // Warning box in case of problems
   // if confirm=kTRUE we ask for a yes/no answer from the user:
   //     if 'yes' is pressed, we return kTRUE, if 'no', kFALSE.
   // by default, only a 'dismiss' button is shown, and this method always returns kTRUE.

   Bool_t reply = kTRUE;
   if (!confirm)
      new TGMsgBox(gClient->GetRoot(), this, title, msg, kMBIconExclamation);
   else {
      Int_t ret_code = 0;
      new TGMsgBox(gClient->GetRoot(), this, title, msg, kMBIconExclamation, kMBYes | kMBNo, &ret_code);
      reply = (ret_code & kMBYes);
   }
   return reply;
}

//__________________________________________

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
void KVDataAnalysisLauncher::RemoveAll(TGComboBox* box)
{
   //replaces functionality of TGComboBox::RemoveAll for ROOT versions < 5.11/02

   Int_t n = box->GetListBox()->GetNumberOfEntries();
   if (n) box->RemoveEntries(0, n - 1);
   if (box->GetSelectedEntry()) {
      ((TGTextLBEntry*)box->GetSelectedEntry())->SetTitle("");
      fClient->NeedRedraw(box->GetSelectedEntry());
   } else {
      box->GetTextEntry()->SetTitle("");
      fClient->NeedRedraw(box->GetTextEntry());
   }
}
void KVDataAnalysisLauncher::RemoveAll(TGListBox* box)
{
   //replaces functionality of TGListBox::RemoveAll for ROOT versions < 5.11/02

   Int_t n = box->GetNumberOfEntries();
   if (n) box->RemoveEntries(0, n - 1);
   if (box->GetSelectedEntry()) {
      ((TGTextLBEntry*)box->GetSelectedEntry())->SetTitle("");
      fClient->NeedRedraw(box->GetSelectedEntry());
   }
}
#endif

//_________________________________________________________________________________________________

void KVDataAnalysisLauncher::SetResource(const Char_t* name, const Char_t* value)
{
   // Handles resource file ".KVDataAnalysisGUIrc"
   // We store the current state of the interface using the following resource names:
   //
   // Repository              KVDataAnalysisLauncher.Repository
   // DataSet                   KVDataAnalysisLauncher.DataSet
   // Task                         KVDataAnalysisLauncher.Task
   // System                    KVDataAnalysisLauncher.System
   // Trigger                    KVDataAnalysisLauncher.Trigger
   // RunsList                  KVDataAnalysisLauncher.RunsList
   // UserClass               KVDataAnalysisLauncher.UserClass
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead
   //
   // We also keep a "memory" of all selected configurations using the following
   // resource names:
   //
   // DataSet                   KVDataAnalysisLauncher.DataSet.[repository]
   // Task                         KVDataAnalysisLauncher.Task.[repository].[dataset]
   // System                    KVDataAnalysisLauncher.System.[repository].[dataset].[task]
   // Trigger                    KVDataAnalysisLauncher.Trigger.[repository].[dataset].[task].[system]
   // RunsList                  KVDataAnalysisLauncher.RunsList.[repository].[dataset].[task].[system].[trigger]
   // UserClass               KVDataAnalysisLauncher.UserClass.[repository].[dataset].[task].[system].[trigger]
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector.[repository].[dataset].[task].[system].[trigger]
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead.[repository].[dataset].[task].[system].[trigger]
   //
   // N.B. [task]:  it is the NAME of the task which is used in the resource name.
   // N.B.2. [system]:  it is the name given by SystemBatchName() which is used in the resource name.
   // N.B.3. [trigger]:  for the resource name we take "M > 4", "M>=8" "All" etc. and replace all ' ', '>' or '=' by ''
   //                          to give "M4", "M8", "All", etc.

   TString res, ful_res;

   BuildResourceName(name, res, ful_res);

   // save current value of resource
   GUIenv->SetValue(res.Data(), value);

   // save resource for future use if possible
   if (ful_res.Length()) GUIenv->SetValue(ful_res.Data(), value);

   GUIenv->SaveLevel(kEnvUser);
}

//_________________________________________________________________________________________________

const Char_t* KVDataAnalysisLauncher::GetResource(const Char_t* name, const Char_t* defaultvalue)
{
   // Handles resource file ".KVDataAnalysisGUIrc"
   //
   // We return the current value of the resource "name"

   TString res = name;
   res.Prepend("KVDataAnalysisLauncher.");
   return GUIenv->GetValue(res.Data(), defaultvalue);
}

//_________________________________________________________________________________________________

const Char_t* KVDataAnalysisLauncher::GetSavedResource(const Char_t* name, const Char_t* defaultvalue)
{
   // Handles resource file ".KVDataAnalysisGUIrc"
   //
   // We look for a stored value of the resource "name" corresponding to the current
   // values of all the resources which come before "name" in the list :
   //    Repository
   //    DataSet
   //    Task
   //    System
   //    Trigger
   // These values are stored in resources with names like:
   //    KVDataAnalysisLauncher.[name].[repository].[dataset]...
   //
   // If no stored value is found, the defaultvalue is returned

   TString res, ful_res;

   BuildResourceName(name, res, ful_res);

   if (!ful_res.Length()) {
      //cout << "GetSavedResource: current resource : " << res.Data() << endl;
      return GUIenv->GetValue(res.Data(), defaultvalue);
   }

   //cout << "GetSavedResource: saved resource : " << ful_res.Data() << endl;
   return GUIenv->GetValue(ful_res.Data(), defaultvalue);
}

//____________________________________________________________________________________________________

const Char_t* KVDataAnalysisLauncher::SystemBatchName(void)
{
   // Get the system name for the batch name

   TString tmp = GetSystem();
   if (tmp == "All") return GetSystem();
   return GetDataAnalyser()->SystemBatchName();
}

//____________________________________________________________________________________________________

void KVDataAnalysisLauncher::BuildResourceName(const Char_t* name, TString& cur_res, TString& saved_res)
{
   // Build the full resource names for storing DataSet, Task, etc.
   //
   // We store the current state of the interface using the following resource names:
   //
   //  "name"                       "cur_res"
   // Repository              KVDataAnalysisLauncher.Repository
   // DataSet                   KVDataAnalysisLauncher.DataSet
   // Task                         KVDataAnalysisLauncher.Task
   // System                    KVDataAnalysisLauncher.System
   // Trigger                    KVDataAnalysisLauncher.Trigger
   // RunsList                  KVDataAnalysisLauncher.RunsList
   // UserClass               KVDataAnalysisLauncher.UserClass
   // UserClassOptions        KVDataAnalysisLauncher.UserClassOptions
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead
   //
   // We also keep a "memory" of all selected configurations using the following
   // resource names:
   //
   //  "name"                       "saved_res"
   // DataSet                   KVDataAnalysisLauncher.DataSet.[repository]
   // Task                         KVDataAnalysisLauncher.Task.[repository].[dataset]
   // System                    KVDataAnalysisLauncher.System.[repository].[dataset].[task]
   // Trigger                    KVDataAnalysisLauncher.Trigger.[repository].[dataset].[task].[system]
   // RunsList                  KVDataAnalysisLauncher.RunsList.[repository].[dataset].[task].[system].[trigger]
   // UserClass               KVDataAnalysisLauncher.UserClass.[repository].[dataset].[task].[system].[trigger]
   // UserClassOptions        KVDataAnalysisLauncher.UserClassOptions.[repository].[dataset].[task].[system].[trigger].[class]
   // KVDataSelector      KVDataAnalysisLauncher.KVDataSelector.[repository].[dataset].[task].[system].[trigger]
   // NbEventsToRead    KVDataAnalysisLauncher.NbEventsToRead.[repository].[dataset].[task].[system].[trigger]
   //
   // N.B. [task]:  it is the NAME of the task which is used in the resource name.
   // N.B.2. [system]:  it is the name given by SystemBatchName() which is used in the resource name.
   // N.B.3. [trigger]:  for the resource name we take "M > 4", "M>=8" "All" etc. and replace all ' ', '>' or '=' by ''
   //                          to give "M4", "M8", "All", etc.
   //
   // If name = "Repository", saved_res="" as we do not save it.
   // If the resource cannot be saved because one of the resources that is needed to form the
   // full resource name has not been set, saved_res="".

   //Resource name for current value
   cur_res = name;
   cur_res.Prepend("KVDataAnalysisLauncher.");

   //Build full name for save
   Int_t index = -1;
   saved_res = "";
   // look for resource name in list ResourceNames
   TObject* resource = 0;
   if ((resource = ResourceNames->FindObject(name))) {
      //get index in resource table
      index = ResourceNames->IndexOf(resource);
   }

   if (index == 0) { // resource name = "Repository"; nothing more to do
      return;
   }

   if (index == -1)  index = NbResNames;   // force loop to end of list

   // resource name is written in format KVDataAnalysisLauncher.[name].[repository]....
   // where the suffixed resource values are all those in the list before the named resource
   // i.e. for name = "Task" we write the resource KVDataAnalysisLauncher.Task.[repository].[dataset]

   saved_res = cur_res;
   TIter next_res(ResourceNames);
   Int_t i = 0;
   Bool_t ok = kTRUE;

   while ((resource = next_res()) && (i++ < index)) {

      TString tmp(GetResource(resource->GetName()));
      TString res;
      if (tmp == "") {
         // one of required resources is not set - none of following resources will be set either
         // we cannot save this resource
         ok = kFALSE;
         break;
      }
      if (!strcmp(resource->GetName(), "Task") && gDataSet) {
         // translate title to name for task
         KVDataAnalysisTask* tsk = gDataSet->GetAnalysisTask(tmp.Data());
         if (tsk) res.Form(".%s", tsk->GetName());
      } else if (!strcmp(resource->GetName(), "System")) {
         // use SystemBatchName
         res.Form(".%s", SystemBatchName());
      } else if (!strcmp(resource->GetName(), "Trigger")) {
         // turn "M > 4" into "M4"
         tmp.ReplaceAll(" ", "");
         tmp.ReplaceAll(">", "");
         tmp.ReplaceAll("=", "");
         res.Form(".%s", tmp.Data());
      } else {
         res.Form(".%s", GetResource(resource->GetName()));
      }
      saved_res += res;
   }
   if (!strcmp(name, "UserClassOptions")) {
      if (strcmp("", GetResource("UserClass", ""))) saved_res += Form(".%s", GetResource("UserClass", ""));
      else ok = kFALSE;
   }

   if (!ok) saved_res = "";
}

//____________________________________________________________________________________________________

void KVDataAnalysisLauncher::FillListOfUserClasses()
{
   // Look at files in working directory & deduce list of user analysis classes.
   // We look for any file ending in '.h'. If we can find a corresponding '.cpp' or '.C' or '.cxx',
   // we consider that it is a user analysis class. This list is used to fill the "User Class"
   // drop-down list.

   TSystemDirectory dir("LocDir", ".");
   TList* lf = dir.GetListOfFiles();
   if (!lf) return;
   UserClassNames->Clear();
   //loop over file names
   TIter next(lf);
   while (TObject* file = next()) {

      // fill list with all '.h' files
      TString tmp(file->GetName());
      if (tmp.EndsWith(".h")) {
         //strip '.h' from filename
         tmp.Remove(tmp.Index(".h"));
         UserClassNames->Add(new TNamed(tmp.Data(), tmp.Data()));
      }

   }
   delete lf;
   // now check that implementation files exist for all '.h' we found
   TIter next_cl(UserClassNames);
   KVString imp, dec;
   while (TNamed* clh = (TNamed*)next_cl()) {
      if (!KVBase::FindClassSourceFiles(clh->GetName(), imp, dec)) clh->SetName("_INVALID_");
   }
   // remove all invalid class names
   while (TObject* obj = UserClassNames->FindObject("_INVALID_")) {
      UserClassNames->Remove(obj);
      delete obj;
   }
   // Info("FillListOfUserClasses", "User classes found in working directory:");
   // UserClassNames->ls();
}

//__________________________________________

void KVDataAnalysisLauncher::SetUserClassList()
{
   // Sets the list of all available user classes in the drop down list

   //Info("SetUserClassList", "called");

#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(cbUserClass);
#else
   cbUserClass->RemoveAll();
#endif
   cbUserClass->Select(-1);

   Int_t nbcl = UserClassNames->GetEntries();
   Int_t i = 0;
   cbUserClass->AddEntry("" , i++);
   while (i < nbcl + 1) {
      cbUserClass->AddEntry(UserClassNames->At(i - 1)->GetName() , i);
      i++;
   }
   cbUserClass->Layout();
}

//__________________________________________

void KVDataAnalysisLauncher::UserClassSelected(char* class_name)
{
   // Called when a user class is selected in the combo box.
   // Updates batch name if 'auto batch name' is selected.

   //Info("UserClassSelected", "User class selected : %s", class_name);

   if (IsBatchNameAuto()) SetAutoBatchName();

   // save resource
   SetResource("UserClassOptions", teUserOptions->GetText());
   SetResource("UserClass", class_name);
   teUserOptions->SetText(GetSavedResource("UserClassOptions", ""));
   SetResource("UserClassOptions", teUserOptions->GetText());
   if (strcmp("", class_name)) btEditClass->SetEnabled(kTRUE);
   else btEditClass->SetEnabled(kFALSE);
   checkCompilation = kTRUE;
}

//__________________________________________

void KVDataAnalysisLauncher::SetUserClass(const Char_t* class_name)
{
   // Sets selected user class in combo box according to e.g. a previously stored resource value.
   // Updates batch name if 'auto batch name' is selected.
   // We update the resource corresponding to the current state of the interface.

   //Info("SetUserClass", "Set user class : %s", class_name);

   // look for user class in list
   TGLBEntry* e = cbUserClass->FindEntry(class_name);
   //cbUserClass->EnableTextInput(kFALSE);

   if (e) {
      //Info("SetUserClass", "Found class in list");

      Int_t i = e->EntryId();
#ifdef __WITHOUT_TGCOMBOBOX_SELECT_BOOL_T
      cbUserClass->Select(i);
#else
      cbUserClass->Select(i, kFALSE);
#endif
      // save current user class options
      //SetResource( "UserClassOptions", teUserOptions->GetText() );
      // save current user class
      SetResource("UserClass", class_name);
      teUserOptions->SetText(GetSavedResource("UserClassOptions", ""));
      btEditClass->SetEnabled(kTRUE);
   }

   else
      // unknown user class
   {
      //Info("SetUserClass", "Class not found in list");
      cbUserClass->Select(-1);
      SetResource("UserClass", "");

      btEditClass->SetEnabled(kFALSE);
   }
   checkCompilation = kTRUE;
   //cbUserClass->EnableTextInput(kTRUE);
}

//__________________________________________

const Char_t* KVDataAnalysisLauncher::GetUserClass()
{
   // Returns currently selected user class name

   if (TGTextLBEntry* e = (TGTextLBEntry*)cbUserClass->GetSelectedEntry()) {
      return e->GetText()->GetString();
   } else {
      return "";
   }
}

//__________________________________________

void KVDataAnalysisLauncher::DisableUserClassList()
{
   // Remove all entries from user class combo box & disable text entry

   //Info("DisableUserClassList", "called");
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   RemoveAll(cbUserClass);
#else
   cbUserClass->RemoveAll();
#endif
   cbUserClass->Select(-1);
   cbUserClass->SetEnabled(kFALSE);
   //cbUserClass->EnableTextInput(kFALSE);
   teUserOptions->SetEnabled(kFALSE);
}

//__________________________________________

void KVDataAnalysisLauncher::EnableUserClassList()
{
   // Reenable user class combo box & text entry,
   // fill list with all known user classes & select the one corresponding
   // to the current environment

   //Info("EnableUserClassList", "called");

   SetUserClassList();
   SetUserClass(GetSavedResource("UserClass", ""));
   //cbUserClass->EnableTextInput(kTRUE);
   cbUserClass->SetEnabled(kTRUE);
   teUserOptions->SetEnabled(kTRUE);
}

void KVDataAnalysisLauncher::EditUserClassFiles()
{
   // If environment variable $EDITOR is set, and if the currently selected
   // user class has available source files, we open them in the
   // user's favourite editor

   TString editor = gSystem->Getenv("EDITOR");
   if (editor == "") return;
   TString uclass = GetUserClass();
   if (uclass == "") return;
   KVString imp, dec;
   if (!KVBase::FindClassSourceFiles(uclass, imp, dec)) return;
   gSystem->Exec(Form("%s %s %s &", editor.Data(), imp.Data(), dec.Data()));
}

//__________________________________________

void KVDataAnalysisLauncher::EnterRunlist()
{
   // Called when user presses "Runlist" button.
   // Open dialogue box in which a runlist can be entered.
   // The runs in the runlist will be selected.

   TString runs = listOfRuns.AsString();
   Bool_t ok = kFALSE;
   new KVInputDialog(this, "Enter list of runs", &runs, &ok);
   if (ok) {
      DeselectAll();
      SetRuns(runs.Data());
   }
}

//__________________________________________

void KVDataAnalysisLauncher::UpdateListOfSelectedRuns()
{
   // Called when the selected runs in TGListView lvRuns change.
   // We update the KVNumberList listOfRuns according to the current selection
   // we modify the limits of the 'runs per job' widget

   listOfRuns.Clear();
   TList* novolist = lvRuns->GetSelectedObjects();
   if (novolist->GetEntries() > 0) {
      TIter nxt(novolist);
      KVRunFile* s = 0;
      while ((s = (KVRunFile*)nxt())) listOfRuns.Add(s->GetRunNumber());
   }
   delete novolist;
   SetResource("RunsList", listOfRuns.AsString());
   selectedRuns->SetText(Form(" Selected Runs : %s", listOfRuns.AsString(MAX_LENGTH_SELECTED_RUNS)));
   fClient->NeedRedraw(selectedRuns);
   SetRunsPerJobLimits();
}

//__________________________________________

void KVDataAnalysisLauncher::ClearListOfSelectedRuns()
{
   // Empty displayed list of selected runs
   listOfRuns.Clear();
   SetResource("RunsList", "");
   selectedRuns->SetText(Form(" Selected Runs : %s", listOfRuns.AsString(MAX_LENGTH_SELECTED_RUNS)));
   fClient->NeedRedraw(selectedRuns);
}
