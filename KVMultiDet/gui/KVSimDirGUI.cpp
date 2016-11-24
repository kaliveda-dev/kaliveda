//Created by KVClassFactory on Wed Jul 18 11:55:11 2012
//Author: John Frankland,,,

#include "KVSimDirGUI.h"
#include "KVSimDir.h"
#include "KVDataSetManager.h"
#include "KVDataSet.h"
#include "KVDataBase.h"
#include "KVDBSystem.h"
#include "KVDBRun.h"
#include "KVSimFile.h"
#include "KVFileDialog.h"
#include "KVInputDialog.h"
#include "KV2Body.h"
#include "TGLabel.h"
#include "TGButtonGroup.h"
#include "TGMsgBox.h"
#include "TSystem.h"
#include "TChain.h"
#include <iostream>
using namespace std;

#include "TProof.h"

ClassImp(KVSimDirGUI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimDirGUI</h2>
<h4>GUI for simulated data</h4>
<img alt="KVSimDirGUI" src="http://indra.in2p3.fr/KaliVedaDoc/images/KVSimDirGUI.png"><br><br>
<h3>See documentation <a href="KVSimDirGUIDoc/KVSimDirGUI.html">here</a>.</h3>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSimDirGUI::KVSimDirGUI()
   : fGuirc(".kvsimdirguirc")
{
   // Default constructor
   // main frame

   fWithPROOF = kFALSE;

   // make Aclic create all *.so *.d files in separate temporary directories
   //this is to avoid problems with KVParticleCondition when using PROOF
   gSystem->SetBuildDir(gSystem->TempDirectory());

   MainFrame = new TGMainFrame(gClient->GetRoot(), 10, 10, kMainFrame | kVerticalFrame);
   MainFrame->SetName("KaliVedaSim GUI");

   TGHorizontalFrame* hftop = new TGHorizontalFrame(MainFrame, 10, 10, kHorizontalFrame);

   TGCompositeFrame* vf = new TGVerticalFrame(hftop, 200, 500, kVerticalFrame);
   // sim dir buttons
   TGCompositeFrame* hf = new TGHorizontalFrame(vf, 10, 10, kHorizontalFrame);
   BaddDir = new TGPictureButton(hf, gClient->GetPicture("package_add.xpm"));
   hf->AddFrame(BaddDir, new TGLayoutHints(kLHintsLeft | kLHintsTop, 3, 2, 2, 2));
   BaddDir->Resize(40, 40);
   BaddDir->SetToolTipText("Add directory");
   BaddDir->Connect("Clicked()", "KVSimDirGUI", this, "AddSimDir()");
   BremDir = new TGPictureButton(hf, gClient->GetPicture("package_delete.xpm"));
   hf->AddFrame(BremDir, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   BremDir->Resize(40, 40);
   BremDir->SetToolTipText("Remove directory");
   BremDir->Connect("Clicked()", "KVSimDirGUI", this, "RemSimDir()");
   BrefreshDir = new TGPictureButton(hf, gClient->GetPicture("refresh2.xpm"));
   hf->AddFrame(BrefreshDir, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   BrefreshDir->Resize(40, 40);
   BrefreshDir->SetToolTipText("Update");
   BrefreshDir->Connect("Clicked()", "KVSimDirGUI", this, "RefreshSimDir()");

   vf->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

   // canvas widget
   fDirListCanvas = new TGCanvas(vf, 150, 400);
   // canvas viewport
   fDirListViewPort = fDirListCanvas->GetViewPort();
   // list tree
   fDirListTree = new TGListTree(fDirListCanvas, kHorizontalFrame);

   popen = gClient->GetPicture("package_add.xpm");
   pclose = gClient->GetPicture("package_add.xpm");

   fDirListViewPort->AddFrame(fDirListTree);
   fDirListTree->SetLayoutManager(new TGHorizontalLayout(fDirListTree));
   fDirListTree->MapSubwindows();
   fDirListTree->Connect("Clicked(TGListTreeItem*,Int_t)", "KVSimDirGUI", this, "SelectSimDir(TGListTreeItem*,Int_t)");
   fDirListCanvas->SetContainer(fDirListTree);
   fDirListCanvas->MapSubwindows();
   vf->AddFrame(fDirListCanvas, new TGLayoutHints(kLHintsTop | kLHintsExpandY, 5, 2, 2, 10));

   hftop->AddFrame(vf, new TGLayoutHints(kLHintsExpandY | kLHintsTop, 2, 2, 2, 2));

   /* lists of data in simdir */
   vf = new TGVerticalFrame(hftop, 600, 500, kVerticalFrame);
   TGGroupFrame* group = new TGGroupFrame(vf, "Simulations");
   fLVsimData = new KVListView(KVSimFile::Class(), group, 550, 200);
   fLVsimData->SetDataColumns(3);
   fLVsimData->SetDataColumn(0, "Name");
   fLVsimData->SetDataColumn(1, "Info", "GetTitle");
   fLVsimData->SetDataColumn(2, "Events");
   fLVsimData->ActivateSortButtons();
   group->AddFrame(fLVsimData, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 10, 10));
   vf->AddFrame(group, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
   group = new TGGroupFrame(vf, "Filtered Simulations");
   fLVfiltData = new KVListView(KVSimFile::Class(), group, 550, 200);
   fLVfiltData->SetDataColumns(7);
   fLVfiltData->SetDataColumn(0, "Simulation", "GetOriginalFile");
   fLVfiltData->SetDataColumn(1, "DataSet");
   fLVfiltData->SetDataColumn(2, "System");
   fLVfiltData->SetDataColumn(3, "Run");
   fLVfiltData->SetDataColumn(4, "Geometry");
   fLVfiltData->SetDataColumn(5, "FilterType");
   fLVfiltData->SetDataColumn(6, "Events");
   fLVfiltData->ActivateSortButtons();
   group->AddFrame(fLVfiltData, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 10, 10));
   vf->AddFrame(group, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

   hftop->AddFrame(vf, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10));
   MainFrame->AddFrame(hftop, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 1, 1, 1, 1));

   fAnalysisTabs = new TGTab(MainFrame);
   /* Analysis Tab */
   fAnalTab = fAnalysisTabs->AddTab("Data Analysis");
   fAnalTab->ChangeOptions(kVerticalFrame);
   vf = new TGVerticalFrame(fAnalTab, 600, 200, kVerticalFrame);
   hf = new TGHorizontalFrame(vf, 10, 10, kHorizontalFrame);
   TGLabel* lab = new TGLabel(hf, "Analysis class : ");
   hf->AddFrame(lab, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
   fTEAnalysisClassFileName = new TGTextEntry(hf, new TGTextBuffer(256));
   fTEAnalysisClassFileName->Resize(650, fTEAnalysisClassFileName->GetDefaultHeight());
   hf->AddFrame(fTEAnalysisClassFileName, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   TGPictureButton* change_class = new TGPictureButton(hf, gClient->GetPicture("bld_open.png"));
   change_class->Resize(fTEAnalysisClassFileName->GetDefaultHeight(), fTEAnalysisClassFileName->GetDefaultHeight());
   change_class->SetToolTipText("Change class");
   change_class->Connect("Clicked()", "KVSimDirGUI", this, "SelectAnalysisClass()");
   hf->AddFrame(change_class, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2));
   vf->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsExpandY, 2, 2, 2, 2));
   hf = new TGHorizontalFrame(vf, 10, 10, kHorizontalFrame);
//   lab = new TGLabel(hf, "Events : ");
//   hf->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
//   fCBAllEvents = new TGCheckButton(hf, "all");
//   hf->AddFrame(fCBAllEvents, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
//   fNENumberEvents = new TGNumberEntry(hf, 1, 10, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMin, 1);
//   hf->AddFrame(fNENumberEvents, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
//   proof_analysis_ana = new TGPictureButton(hf, gClient->GetPicture("proof_base.xpm"));
//   proof_analysis_ana->Connect("Pressed()", "KVSimDirGUI", this, "EnableProof()");
//   proof_analysis_ana->Connect("Released()", "KVSimDirGUI", this, "DisableProof()");
//   proof_analysis_ana->SetToolTipText("Enable PROOF");
//   proof_analysis_ana->Resize(40, 40);
//   proof_analysis_ana->AllowStayDown(kTRUE);
//   hf->AddFrame(proof_analysis_ana, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 400, 2, 2, 2));
//   TGPictureButton* launch_analysis = new TGPictureButton(hf, gClient->GetPicture("query_submit.xpm"));
////   launch_analysis->Connect("Clicked()", "KVSimDirGUI", this, "RunAnalysis()");
//   launch_analysis->Connect("Clicked()", "KVSimDirGUI", this, "Run()");
//   launch_analysis->SetToolTipText("Run analysis");
//   launch_analysis->Resize(40, 40);
//   hf->AddFrame(launch_analysis, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 2, 2));
   vf->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsExpandY, 2, 2, 2, 2));
//   fCBAllEvents->Connect("Toggled(Bool_t)", "KVSimDirGUI", this, "EnableEventNumberEntry(Bool_t)");
//   fCBAllEvents->SetState(kButtonDown, kTRUE);

   fAnalTab->AddFrame(vf, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

   /* Filter Tab */
   fFiltTab = fAnalysisTabs->AddTab("Filtering");
   fFiltTab->ChangeOptions(kVerticalFrame);
   vf = new TGHorizontalFrame(fFiltTab, 600, 200, kHorizontalFrame);


   /*system and dataset*/
   hf = new TGGroupFrame(vf, "Experimental condition", kVerticalFrame);
//   new TGGroupFrame()
//   lab = new TGLabel(hf, "Experimental conditions : ");
//   hf->AddFrame(lab, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));

   TGCompositeFrame* tmpf = new TGHorizontalFrame(hf, 600, 200, kHorizontalFrame);
   fCBdataset = new TGComboBox(tmpf);
   if (!gDataSetManager) {
      KVDataSetManager* dsm = new KVDataSetManager;
      dsm->Init();
   }
   // fill list of datasets
   Int_t ndatasets = gDataSetManager->GetNtotal();
   fCBdataset->AddEntry("Choose dataset...", 0);
   for (int i = 1; i <= ndatasets; i++) {
      fCBdataset->AddEntry(gDataSetManager->GetDataSet(i - 1)->GetName(), i);
   }
   fCBdataset->Layout();
   fCBdataset->Resize(200, 20);
   fCBdataset->Select(0, kFALSE);
   tmpf->AddFrame(fCBdataset, new TGLayoutHints(kLHintsLeft | kLHintsTop, 10, 2, 2, 2));
   fCBdataset->Connect("Selected(const char*)", "KVSimDirGUI", this, "SelectDataSet(const char*)");
   hf->AddFrame(tmpf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

   tmpf = new TGHorizontalFrame(hf, 600, 200, kHorizontalFrame);
   fCBsystem = new TGComboBox(tmpf);
   fCBsystem->AddEntry("Choose system...", 0);
   fCBsystem->Layout();
   fCBsystem->Resize(200, 20);
   fCBsystem->Select(0, kFALSE);
   tmpf->AddFrame(fCBsystem, new TGLayoutHints(kLHintsLeft | kLHintsTop, 10, 2, 2, 2));
   fCBsystem->Connect("Selected(const char*)", "KVSimDirGUI", this, "SelectSystem(const char*)");

   fTESystem = new TGTextEntry(tmpf, new TGTextBuffer(200));
   fTESystem->Resize(200, fTESystem->GetDefaultHeight());
   fTESystem->SetToolTipText("[Projectile_Symbol]+[Target_Symbol]@[Incident_Energy]MeV/A (ex: 129Xe+119Sn@50.0MeV/A)");
   tmpf->AddFrame(fTESystem, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

   hf->AddFrame(tmpf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

   tmpf = new TGHorizontalFrame(hf, 600, 200, kHorizontalFrame);
   fCBrun = new TGComboBox(tmpf);
   fCBrun->AddEntry("Choose run...", 0);
   fCBrun->Layout();
   fCBrun->Resize(200, 20);
   fCBrun->Select(0, kFALSE);
   tmpf->AddFrame(fCBrun, new TGLayoutHints(kLHintsLeft | kLHintsTop, 10, 2, 2, 2));
   fCBrun->Connect("Selected(const char*)", "KVSimDirGUI", this, "SelectRun(const char*)");

//   fTERunNumber = new TGTextEntry(tmpf, new TGTextBuffer(200));
//   fTERunNumber->Resize(200, fTERunNumber->GetDefaultHeight());
//   tmpf->AddFrame(fTERunNumber, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   hf->AddFrame(tmpf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

   vf->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 10, 2));


   hf = new TGHorizontalFrame(vf, 10, 10, kHorizontalFrame);

   TGButtonGroup* bgroup = new TGButtonGroup(hf, "Filter type");
   TGRadioButton* radiob = new TGRadioButton(bgroup, "Geometric");
   radiob = new TGRadioButton(bgroup, "Geometry+Thresholds");
   radiob = new TGRadioButton(bgroup, "Full");
   bgroup->Connect("Clicked(Int_t)", "KVSimDirGUI", this, "FilterType(Int_t)");
   radiob->SetState(kButtonDown);
   fFilterType = kFTFull;
   hf->AddFrame(bgroup, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 2, 2, 2, 2));
   bgroup = new TGButtonGroup(hf, "Geometry");
   radiob = new TGRadioButton(bgroup, "KaliVeda");
   radiob = new TGRadioButton(bgroup, "ROOT");
   bgroup->Connect("Clicked(Int_t)", "KVSimDirGUI", this, "GeoType(Int_t)");
   radiob->SetState(kButtonDown);
   fGeoType = kGTROOT;
   hf->AddFrame(bgroup, new TGLayoutHints(kLHintsTop | kLHintsLeft, 20, 2, 2, 2));
   bgroup = new TGButtonGroup(hf, "Kinematics");
   radiob = new TGRadioButton(bgroup, "CM");
   radiob->SetState(kButtonDown);
   radiob = new TGRadioButton(bgroup, "Lab");
   bgroup->Connect("Clicked(Int_t)", "KVSimDirGUI", this, "Kinematics(Int_t)");
   fKine = kKCM;
   hf->AddFrame(bgroup, new TGLayoutHints(kLHintsTop | kLHintsLeft, 20, 2, 2, 2));
//   proof_analysis_filt = new TGPictureButton(hf, gClient->GetPicture("proof_base.xpm"));
//   proof_analysis_filt->Connect("Pressed()", "KVSimDirGUI", this, "EnableProof()");
//   proof_analysis_filt->Connect("Released()", "KVSimDirGUI", this, "DisableProof()");
//   proof_analysis_filt->SetToolTipText("Enable PROOF");
//   proof_analysis_filt->Resize(40, 40);
//   proof_analysis_filt->AllowStayDown(kTRUE);
//   hf->AddFrame(proof_analysis_filt, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 250, 2, 2, 2));
//   launch_analysis = new TGPictureButton(hf, gClient->GetPicture("query_submit.xpm"));
////   launch_analysis->Connect("Clicked()", "KVSimDirGUI", this, "RunFilter()");
//   launch_analysis->Connect("Clicked()", "KVSimDirGUI", this, "Run()");
//   launch_analysis->SetToolTipText("Run filter");
//   launch_analysis->Resize(40, 40);
//   hf->AddFrame(launch_analysis, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 2, 2));
   vf->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 10, 2));

//    hf = new TGHorizontalFrame(vf, 10, 10, kHorizontalFrame);
//    lab= new TGLabel(hf, "Output directory : ");
//    hf->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 2,2,2,2));
//    fTEOutputDir = new TGTextEntry(hf, new TGTextBuffer(256));
//    fTEOutputDir->SetText(gSystem->pwd());
//    fTEOutputDir->Resize(650, fTEOutputDir->GetDefaultHeight());
//    hf->AddFrame(fTEOutputDir, new TGLayoutHints(kLHintsCenterY|kLHintsLeft, 2,2,2,2));
//    change_class = new TGPictureButton(hf,gClient->GetPicture("bld_open.png"));
//    change_class->Resize(fTEOutputDir->GetDefaultHeight(),fTEOutputDir->GetDefaultHeight());
//    change_class->SetToolTipText("Change directory");
//    change_class->Connect("Clicked()", "KVSimDirGUI", this, "ChangeOutputDirectory()");
//    hf->AddFrame(change_class,new TGLayoutHints(kLHintsCenterY|kLHintsLeft, 2,2,2,2));
//    vf->AddFrame(hf, new TGLayoutHints(kLHintsTop|kLHintsExpandY,2,2,2,2));

   fFiltTab->AddFrame(vf, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

   fAnalysisTabs->MapSubwindows();
   fAnalysisTabs->Resize(fAnalysisTabs->GetDefaultWidth(), 300);
   fAnalysisTabs->GetLayoutManager()->Layout();

   MainFrame->AddFrame(fAnalysisTabs, new TGLayoutHints(kLHintsExpandX | kLHintsTop, 10, 10, 15, 10));



   vf = new TGVerticalFrame(MainFrame, 600, 200, kVerticalFrame);
   hf = new TGHorizontalFrame(vf, 600, 200, kHorizontalFrame);

   lab = new TGLabel(hf, "Events : ");
   hf->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   fCBAllEvents = new TGCheckButton(hf, "all");
   hf->AddFrame(fCBAllEvents, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   fNENumberEvents = new TGNumberEntry(hf, 1, 10, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMin, 1);
   hf->AddFrame(fNENumberEvents, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
   fCBAllEvents->Connect("Toggled(Bool_t)", "KVSimDirGUI", this, "EnableEventNumberEntry(Bool_t)");
   fCBAllEvents->SetState(kButtonDown, kTRUE);


   proof_analysis = new TGPictureButton(hf, gClient->GetPicture("proof_base.xpm"));
   proof_analysis->Connect("Pressed()", "KVSimDirGUI", this, "EnableProof()");
   proof_analysis->Connect("Released()", "KVSimDirGUI", this, "DisableProof()");
   proof_analysis->SetToolTipText("Enable PROOF");
   proof_analysis->Resize(40, 40);
   proof_analysis->AllowStayDown(kTRUE);

   TGPictureButton* launch_analysis = new TGPictureButton(hf, gClient->GetPicture("query_submit.xpm"));
   launch_analysis->Connect("Clicked()", "KVSimDirGUI", this, "Run()");
   launch_analysis->SetToolTipText("Run filter");
   launch_analysis->Resize(40, 40);

   hf->AddFrame(launch_analysis, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 2, 2, 2));
   hf->AddFrame(proof_analysis, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 2, 2, 2));

   vf->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0));
   MainFrame->AddFrame(vf, new TGLayoutHints(kLHintsExpandX | kLHintsTop, 10, 10, 15, 10));


   MainFrame->SetMWMHints(kMWMDecorAll,
                          kMWMFuncAll,
                          kMWMInputModeless);
   MainFrame->MapSubwindows();

   MainFrame->Resize(MainFrame->GetDefaultSize());
   MainFrame->MapWindow();
   // MainFrame->Resize(200,500);
   // use .guirc to fill the list of sim dirs
   KVString simdirs = fGuirc.GetValue("SimDirs", "");
   if (simdirs != "") {
      simdirs.Begin(" ");
      while (!simdirs.End()) {
         KVString simdir = simdirs.Next();
         KVString simdirectory = fGuirc.GetValue(Form("%s.Directory", simdir.Data()), "");
         KVSimDir* sd = new KVSimDir(simdir, simdirectory);
         sd->AnalyseDirectory();
         fListOfDirs.Add(sd);
      }
      FillTreeList();
   }
}

//________________________________________________________________

KVSimDirGUI::KVSimDirGUI(const KVSimDirGUI& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVSimDirGUI::~KVSimDirGUI()
{
   // Destructor
}

//________________________________________________________________

void KVSimDirGUI::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSimDirGUI::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVSimDirGUI& CastedObj = (KVSimDirGUI&)obj;
}

void KVSimDirGUI::FillTreeList()
{
   EmptyTreeList();
   TIter next(&fListOfDirs);
   TObject* o;
   while ((o = next())) {
      TGListTreeItem* item0 = fDirListTree->AddItem(NULL, o->GetName());
      item0->SetPictures(popen, pclose);
      fDirListTree->CloseItem(item0);
   }
}

void KVSimDirGUI::EmptyTreeList()
{
   TGListTreeItem* first = fDirListTree->GetFirstItem();
   while (first) {
      fDirListTree->DeleteItem(first);
      first = fDirListTree->GetFirstItem();
   }
}

void KVSimDirGUI::AddSimDir()
{
   static TString dir(".");
   TGFileInfo fi;
   fi.fIniDir = StrDup(dir);
   new KVFileDialog(gClient->GetDefaultRoot(), MainFrame, kKVFDDirectory, &fi);
   if (fi.fFilename) {
      TString simdirname;
      Bool_t ok;
      TString quest;
      quest.Form("Enter name for dataset %s", fi.fIniDir);
      new KVInputDialog(MainFrame, quest, &simdirname, &ok);
      if (ok) {
         KVSimDir* sd = new KVSimDir(simdirname, fi.fIniDir);
         sd->AnalyseDirectory();
         fListOfDirs.Add(sd);
         FillTreeList();
         fLVsimData->Display(sd->GetSimDataList());
         fLVfiltData->Display(sd->GetFiltDataList());
         // add to guirc file
         TString simdirs = fGuirc.GetValue("SimDirs", "");
         if (simdirs != "") simdirs += " ";
         simdirs += simdirname;
         fGuirc.SetValue("SimDirs", simdirs);
         fGuirc.SetValue(Form("%s.Directory", simdirname.Data()), fi.fIniDir);
         fGuirc.SaveLevel(kEnvUser);
      }
   }
   dir = fi.fIniDir;
}

void KVSimDirGUI::RefreshSimDir()
{
   TGListTreeItem* selection = fDirListTree->GetSelected();
   if (!selection) return;
   TString simdirname = selection->GetText();
   KVSimDir* togo = (KVSimDir*)fListOfDirs.FindObject(simdirname);
   if (!togo) return;
   togo->AnalyseDirectory();
   fLVsimData->Display(togo->GetSimDataList());
   fLVfiltData->Display(togo->GetFiltDataList());
}

void KVSimDirGUI::RemSimDir()
{
   TGListTreeItem* selection = fDirListTree->GetSelected();
   if (!selection) return;
   TString simdirname = selection->GetText();
   KVSimDir* togo = (KVSimDir*)fListOfDirs.FindObject(simdirname);
   if (!togo) return;
   fListOfDirs.Remove(togo);
   // there is no way of removing a name-value pair from a TEnv!!
   fGuirc.SetValue(Form("%s.Directory", simdirname.Data()), "");
   delete togo;
   FillTreeList();
   fLVsimData->RemoveAll();
   fLVfiltData->RemoveAll();
   // update guirc file
   TString simdirs = "";
   TIter next(&fListOfDirs);
   while ((togo = (KVSimDir*)next())) {
      if (simdirs != "") simdirs += " ";
      simdirs += togo->GetName();
      fGuirc.SetValue(Form("%s.Directory", togo->GetName()), togo->GetDirectory());
   }
   fGuirc.SetValue("SimDirs", simdirs);
   fGuirc.SaveLevel(kEnvUser);
}

void KVSimDirGUI::SelectSimDir(TGListTreeItem* simdir, Int_t)
{
   KVSimDir* sd = (KVSimDir*)fListOfDirs.FindObject(simdir->GetText());
   fLVsimData->Display(sd->GetSimDataList());
   fLVfiltData->Display(sd->GetFiltDataList());
}

void KVSimDirGUI::SelectAnalysisClass()
{
   static TString dir(".");
   const char* filetypes[] = {
      "Class header files", "*.h",
      0, 0
   };
   TGFileInfo fi;
   fi.fIniDir = StrDup(dir);
   fi.fFileTypes = filetypes;
   new KVFileDialog(gClient->GetDefaultRoot(), MainFrame, kKVFDOpen, &fi);
   if (fi.fFilename) {
      fAnalClassName = fi.fFilename;
      Int_t idot = fAnalClassName.Index(".");
      if (idot < 0) return;
      fAnalClassName.Remove(idot, 2);
      if (FindClassSourceFiles(gSystem->BaseName(fAnalClassName), fAnalClassImp, fAnalClassHeader, fi.fIniDir)) {
         fTEAnalysisClassFileName->SetText(fAnalClassName);
         fAnalClassName = gSystem->BaseName(fAnalClassName);
         fAnalClassDir = fi.fIniDir;
         cout << "Found class header " << fAnalClassHeader << " and implementation " << fAnalClassImp << " in " << fAnalClassDir << endl;
      }
   }
   dir = fi.fIniDir;
}

void KVSimDirGUI::EnableEventNumberEntry(Bool_t on)
{
   fNENumberEvents->SetState(!on);
}

void KVSimDirGUI::RunAnalysis()
{
   TList* selected_sim_runs = fLVsimData->GetSelectedObjects();
   TList* selected_filt_runs = fLVfiltData->GetSelectedObjects();
   if (!selected_sim_runs->GetEntries() && !selected_filt_runs->GetEntries()) {
      new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Choose one or more simulated or filtered data files!", kMBIconExclamation);
      delete selected_sim_runs;
      delete selected_filt_runs;
      return;
   }
   if (selected_sim_runs->GetEntries() && selected_filt_runs->GetEntries()) {
      new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Choose EITHER simulated or filtered data files!", kMBIconExclamation);
      delete selected_sim_runs;
      delete selected_filt_runs;
      return;
   }
   if (fAnalClassHeader == "" || fAnalClassImp == "") {
      new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Choose a valid analysis class!", kMBIconExclamation);
      delete selected_sim_runs;
      delete selected_filt_runs;
      return;
   }
   TList* runs_to_analyse = (selected_sim_runs->GetEntries() ? selected_sim_runs : selected_filt_runs);
   Bool_t filtered_analysis = (selected_filt_runs->GetEntries() > 0) ;
   runs_to_analyse->ls();
   TChain* analysis_chain = BuildChain(runs_to_analyse);

   TString fullclasspath;
   AssignAndDelete(fullclasspath, gSystem->ConcatFileName(fAnalClassDir, fAnalClassImp));
   fullclasspath += "+g";

   Long64_t nevents = analysis_chain->GetEntries();
   Bool_t all_events = fCBAllEvents->IsDown();

   if (fWithPROOF) {
      TProof* p = TProof::Open("");
      analysis_chain->SetProof();
      p->ClearCache();//to avoid problems with compilation of KVParticleCondition
      // enable KaliVeda on PROOF cluster
      if (p->EnablePackage("KaliVeda") != 0) {
         // first time, need to 'upload' package
         TString fullpath = KVBase::GetETCDIRFilePath("KaliVeda.par");
         p->UploadPackage(fullpath);
         p->EnablePackage("KaliVeda");
      }
   }

   TString results_file_name;
   KVSimFile* first_file = (KVSimFile*)runs_to_analyse->First();
   results_file_name.Form("%s_%s", fAnalClassName.Data(), first_file->GetName());

   if (!all_events) {
      nevents = (Long64_t)fNENumberEvents->GetNumber();
   }

   TString options;
   if (filtered_analysis) {
      options.Form("EventsReadInterval=%lld,BranchName=%s,CombinedOutputFile=%s,DataSet=%s,System=%s,Run=%d",
                   (nevents > 10 ? nevents / 10 : 1),
                   first_file->GetBranchName(),
                   results_file_name.Data(),
                   first_file->GetDataSet(),
                   first_file->GetSystem(),
                   first_file->GetRun()
                  );
   } else {
      options.Form("EventsReadInterval=%lld,BranchName=%s,CombinedOutputFile=%s",
                   (nevents > 10 ? nevents / 10 : 1),
                   first_file->GetBranchName(),
                   results_file_name.Data());
   }

   if (!all_events) {
      cout << "Processing " << nevents << " events" << endl;
      analysis_chain->Process(fullclasspath, options, nevents);
   } else
      analysis_chain->Process(fullclasspath, options);

   delete analysis_chain;
   delete selected_sim_runs;
   delete selected_filt_runs;
}

TChain* KVSimDirGUI::BuildChain(TList* runs)
{
   TIter next(runs);
   KVSimFile* file;
   TChain* theChain = 0;
   while ((file = (KVSimFile*)next())) {
      if (!theChain) {
         theChain = new TChain(file->GetTreeName());
      }
      TString fullpath;
      AssignAndDelete(fullpath, gSystem->ConcatFileName(file->GetSimDir()->GetDirectory(), file->GetName()));
      theChain->Add(fullpath);
   }
   return theChain;
}

//________________________________________________________________

void KVSimDirGUI::SelectDataSet(const char* name)
{
   KVDataSet* ds = gDataSetManager->GetDataSet(name);
   if (ds) {
      fDataset = name;
      fSystem = "";
      fRun = "";
      ds->cd();
      KVSeqCollection* systems = 0;
      if (gDataBase && gDataBase->GetTable("Systems")) systems = gDataBase->GetTable("Systems")->GetRecords();
      fCBsystem->RemoveAll();
      fCBsystem->AddEntry("Choose system...", 0);
      if (systems) {
         TIter next(systems);
         KVDBSystem* sys;
         int i = 1;
         while ((sys = (KVDBSystem*)next())) {
            fCBsystem->AddEntry(sys->GetName(), i++);
         }
      }
      fCBsystem->Layout();
      fCBsystem->Select(0, kFALSE);
      fCBrun->RemoveAll();
      fCBrun->AddEntry("Choose run...", 0);
      fCBrun->Layout();
      fCBrun->Select(0, kFALSE);
   }
}

void KVSimDirGUI::SelectSystem(const char* sysname)
{
   KVDBSystem* sys = (KVDBSystem*)gDataBase->GetRecord("Systems", sysname);
   if (sys) {
      fSystem = sysname;
      fRun = "";
      KVList* runs = sys->GetRuns();
      KVDBRun* dbr;
      TIter next(runs);
      int i = 1;
      fCBrun->RemoveAll();
      fCBrun->AddEntry("Choose run...", 0);
      while ((dbr = (KVDBRun*)next())) {
         fCBrun->AddEntry(Form("%4d", dbr->GetNumber()), i++);
      }
      fCBrun->Layout();
      fCBrun->Select(0, kFALSE);
      fTESystem->SetText(sysname);
   }
}

void KVSimDirGUI::SelectRun(const char* run)
{
   fRun = run;
   fRun.Remove(TString::kBoth, ' ');
   if (!fRun.IsDigit()) fRun = "";
}

void KVSimDirGUI::Run()
{
//    Info("Run","current tab : '%d'",fAnalysisTabs->GetCurrent());
   if (fAnalysisTabs->GetCurrent() == 0)      RunAnalysis();
   else if (fAnalysisTabs->GetCurrent() == 1) RunFilter();
}

// void KVSimDirGUI::ChangeOutputDirectory()
// {
//    static TString dir(".");
//    TGFileInfo fi;
//    fi.fIniDir = StrDup(dir);
//    new KVFileDialog(gClient->GetDefaultRoot(), MainFrame, kKVFDDirectory, &fi);
//    if (fi.fFilename) {
//       fTEOutputDir->SetText(fi.fIniDir);
//    }
//    dir = fi.fIniDir;
// }


void KVSimDirGUI::RunFilter()
{
   TList* selected_sim_runs = fLVsimData->GetSelectedObjects();
   TList* selected_filt_runs = fLVfiltData->GetSelectedObjects();
   if (selected_filt_runs->GetEntries()) {
      new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunFilter", "Only simulated events can be filtered!", kMBIconExclamation);
      delete selected_sim_runs;
      delete selected_filt_runs;
      return;
   }
   if (!selected_sim_runs->GetEntries()) {
      new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunFilter", "Choose a simulated data file to filter!", kMBIconExclamation);
      delete selected_sim_runs;
      delete selected_filt_runs;
      return;
   }
   if (selected_sim_runs->GetEntries() > 1) {
      new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunFilter", "Choose ONE simulated data file to filter!", kMBIconExclamation);
      delete selected_sim_runs;
      delete selected_filt_runs;
      return;
   }
   TList* runs_to_analyse = selected_sim_runs;
   runs_to_analyse->ls();
   TChain* analysis_chain = BuildChain(runs_to_analyse);
   analysis_chain->ls();

   TString geometry;
   if (fGeoType == kGTROOT) geometry = "ROOT";
   else geometry = "KV";
   TString kinema;
   if (fKine == kKCM) kinema = "cm";
   else kinema = "lab";
   TString filter;
   switch (fFilterType) {
      case kFTGeo:
         filter = "Geo";
         break;
      case kFTSeuils:
         filter = "GeoThresh";
         break;
      case kFTFull:
         filter = "Full";
   }

   // check system
   if (fSystem == "") {
      fSystem = fTESystem->GetText();
      KV2Body cd(fSystem.Data());
      cd.CalculateKinematics();
      cd.Print();
   }

   TString options;
   Long64_t nevents = analysis_chain->GetEntries();
   Bool_t all_events = fCBAllEvents->IsDown();
   if (!all_events) {
      nevents = (Long64_t)fNENumberEvents->GetNumber();
   }

   options.Form("EventsReadInterval=%lld,SimFileName=%s,SimTitle=%s,BranchName=%s,Dataset=%s,System=%s,Geometry=%s,Filter=%s,OutputDir=%s,Kinematics=%s",
                (nevents > 10 ? nevents / 10 : 1),
                ((KVSimFile*)runs_to_analyse->First())->GetName(),
                analysis_chain->GetTitle(),
                ((KVSimFile*)runs_to_analyse->First())->GetBranchName(),
                fDataset.Data(), fSystem.Data(), geometry.Data(), filter.Data(),
                ((KVSimFile*)runs_to_analyse->First())->GetSimDir()->GetDirectory(), kinema.Data());

   Info("RunFilter", "old : %s", options.Data());

   options  = Form("EventsReadInterval=%lld,", (nevents > 10 ? nevents / 10 : 1));
   options += Form("SimFileName=%s,", ((KVSimFile*)runs_to_analyse->First())->GetName());
   options += Form("SimTitle=%s,", analysis_chain->GetTitle());
   options += Form("BranchName=%s,", ((KVSimFile*)runs_to_analyse->First())->GetBranchName());
   options += Form("Dataset=%s,", fDataset.Data());
   options += Form("System=%s,", fSystem.Data());
   options += Form("Geometry=%s,", geometry.Data());
   options += Form("Filter=%s,", filter.Data());
   options += Form("OutputDir=%s,", ((KVSimFile*)runs_to_analyse->First())->GetSimDir()->GetDirectory());
   options += Form("Kinematics=%s", kinema.Data());


   if (fRun != "") {
      TString r;
      r.Form(",Run=%s", fRun.Data());
      options += r;
   }
   Info("RunFilter", "%s", options.Data());


   if (fWithPROOF) {
      TProof* p = TProof::Open("");
      analysis_chain->SetProof();
      p->ClearCache();//to avoid problems with compilation of KVParticleCondition
      // enable KaliVeda on PROOF cluster
      if (p->EnablePackage("KaliVeda") != 0) {
         // first time, need to 'upload' package
         TString fullpath = KVBase::GetETCDIRFilePath("KaliVeda.par");
         p->UploadPackage(fullpath);
         p->EnablePackage("KaliVeda");
      }
   }
   analysis_chain->Process("KVEventFiltering", options, nevents);
   RefreshSimDir();

   delete analysis_chain;
   delete selected_sim_runs;
   delete selected_filt_runs;
}
