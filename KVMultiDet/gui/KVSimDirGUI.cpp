//Created by KVClassFactory on Wed Jul 18 11:55:11 2012
//Author: John Frankland,,,

#include "KVSimDirGUI.h"
#include "KVDataSetManager.h"
#include "KVBatchSystemManager.h"
#include "KVDataSet.h"
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
#include <KVDataAnalyser.h>
#include <KVSimDirFilterAnalyser.h>
#include <KVSimReader.h>
#include <iostream>
using namespace std;

#include "KVBatchSystemParametersGUI.h"
#include "KVDropDownDialog.h"

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

#ifdef WITH_RSQLITE
TString KVSimDirGUI::AddTableForDataSet(KVSimDir* sd, int ds_table_number)
{
   // Add a new dataset table for the given KVSimDir object

   TString ds_table_name = Form("Table_%d", ds_table_number);
   KVSQLite::table DStable(ds_table_name);
   DStable.add_primary_key("Id");
   DStable.add_column("FileName", KVSQLite::column_type::TEXT);
   DStable.add_column("TreeInfo", KVSQLite::column_type::TEXT);
   DStable.add_column("Filtered", KVSQLite::column_type::INTEGER);
   DStable.add_column("Events", KVSQLite::column_type::INTEGER);
   DStable.add_column("DataSet", KVSQLite::column_type::TEXT);
   DStable.add_column("System", KVSQLite::column_type::TEXT);
   DStable.add_column("Run", KVSQLite::column_type::INTEGER);
   DStable.add_column("GeoType", KVSQLite::column_type::TEXT);
   DStable.add_column("TreeName", KVSQLite::column_type::TEXT);
   DStable.add_column("BranchName", KVSQLite::column_type::TEXT);
   DStable.add_column("OrigFile", KVSQLite::column_type::TEXT);
   DStable.add_column("FiltType", KVSQLite::column_type::TEXT);
   DStable.add_column("Gemini", KVSQLite::column_type::INTEGER);
   DStable.add_column("GemDecayPerEvent", KVSQLite::column_type::INTEGER);
   fConfigDB.add_table(DStable);
   // update name of dataset table in main table
   fConfigDB["Datasets"]["Table"] = ds_table_name;
   fConfigDB.update("Datasets", "Table", Form("Name=\"%s\"", sd->GetName()));
   return ds_table_name;
}

void KVSimDirGUI::FillDataSetTableWithInfos(TString ds_table_name, KVSimDir* sd)
{
   fConfigDB.prepare_data_insertion(ds_table_name);
   KVSQLite::table& dstable = fConfigDB[ds_table_name];
   if (sd->GetSimDataList()->GetEntries()) {
      TIter it1(sd->GetSimDataList());
      KVSimFile* sf = nullptr;
      while ((sf = (KVSimFile*)it1())) {
         dstable["Id"].set_null();
         dstable["FileName"] = sf->GetName();
         dstable["TreeInfo"] = sf->GetTitle();
         dstable["Filtered"] = 0;
         dstable["Events"] = (Int_t)sf->GetEvents();
         dstable["TreeName"] = sf->GetTreeName();
         dstable["BranchName"] = sf->GetBranchName();
         fConfigDB.insert_data_row();
      }
   }
   if (sd->GetFiltDataList()->GetEntries()) {
      TIter it1(sd->GetFiltDataList());
      KVSimFile* sf = nullptr;
      while ((sf = (KVSimFile*)it1())) {
         dstable["Id"].set_null();
         dstable["FileName"] = sf->GetName();
         dstable["TreeInfo"] = sf->GetTitle();
         dstable["Filtered"] = 1;
         dstable["Events"] = (Int_t)sf->GetEvents();
         dstable["TreeName"] = sf->GetTreeName();
         dstable["BranchName"] = sf->GetBranchName();
         dstable["DataSet"] = sf->GetDataSet();
         dstable["System"] = sf->GetSystem();
         dstable["Run"] = sf->GetRun();
         dstable["GeoType"] = sf->GetGeometry();
         dstable["FiltType"] = sf->GetFilterType();
         dstable["OrigFile"] = sf->GetOriginalFile();
         dstable["Gemini"] = sf->IsGemini();
         dstable["GemDecayPerEvent"] = sf->GetGemDecayPerEvent();
         fConfigDB.insert_data_row();
      }
   }
   fConfigDB.end_data_insertion();
}
#endif

KVSimDirGUI::KVSimDirGUI()
   : fGuirc(".kvsimdirguirc")
#ifdef WITH_RSQLITE
   , fConfigDB(gSystem->ExpandPathName("$(HOME)/.kvsimdirguidb"))
#endif
{
   // Default constructor
   // main frame

   fSelectedSimDir = nullptr;
   fWithPROOF = kFALSE;

   // make Aclic create all *.so *.d files in separate temporary directories
   //this is to avoid problems with KVParticleCondition when using PROOF
   gSystem->SetBuildDir(gSystem->TempDirectory());

   MainFrame = new TGMainFrame(gClient->GetRoot(), 10, 10, kMainFrame | kVerticalFrame);
   MainFrame->SetName("KaliVedaSim GUI");
   MainFrame->SetWindowName("KaliVedaSim GUI");
   MainFrame->Connect("CloseWindow()", "KVSimDirGUI", this, "Exit()");

   TGHorizontalFrame* hftop = new TGHorizontalFrame(MainFrame, 10, 10, kHorizontalFrame);

   TGCompositeFrame* vf = new TGVerticalFrame(hftop, 200, 500, kVerticalFrame);
   // sim dir buttons
   TGCompositeFrame* hf = new TGHorizontalFrame(vf, 10, 10, kHorizontalFrame);
   BaddDir = new TGPictureButton(hf, gClient->GetPicture("package_add.xpm"));
   hf->AddFrame(BaddDir, new TGLayoutHints(kLHintsLeft | kLHintsTop, 3, 2, 2, 2));
   BaddDir->Resize(40, 40);
   BaddDir->SetToolTipText("Add directory");
   BaddDir->Connect("Clicked()", "KVSimDirGUI", this, "AddSimDir()");
   BimpSim = new TGPictureButton(hf, gClient->GetPicture("bld_text.xpm"));
   hf->AddFrame(BimpSim, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 2, 2));
   BimpSim->Resize(40, 40);
   BimpSim->SetToolTipText("Import simulation");
   BimpSim->Connect("Clicked()", "KVSimDirGUI", this, "ImportSimulation()");
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
   BimpSim->SetEnabled(kFALSE);
   BremDir->SetEnabled(kFALSE);
   BrefreshDir->SetEnabled(kFALSE);

   vf->AddFrame(hf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

   // canvas widget
   fDirListCanvas = new TGCanvas(vf, 172, 400);
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
   fLVsimData->AllowContextMenu(kFALSE);
   group->AddFrame(fLVsimData, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 5, 5, 10, 10));
   vf->AddFrame(group, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
   group = new TGGroupFrame(vf, "Filtered Simulations");
   fLVfiltData = new KVListView(KVSimFile::Class(), group, 550, 200);
   fLVfiltData->SetDataColumns(8);
   fLVfiltData->SetDataColumn(0, "Simulation", "GetOriginalFile");
   fLVfiltData->SetDataColumn(1, "DataSet");
   fLVfiltData->SetDataColumn(2, "System");
   fLVfiltData->SetDataColumn(3, "Run");
   fLVfiltData->SetDataColumn(4, "Gemini", "IsGemini");
   fLVfiltData->SetDataColumn(5, "Decay/event", "GetGemDecayPerEvent");
   fLVfiltData->SetDataColumn(6, "FilterType");
   fLVfiltData->SetDataColumn(7, "Events");
   fLVfiltData->ActivateSortButtons();
   fLVfiltData->AllowContextMenu(kFALSE);
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

   TGTextButton* new_anal_class = new TGTextButton(hf, "New simulated analysis class");
   new_anal_class->Connect("Clicked()", "KVSimDirGUI", this, "NewSimulatedAnalysisClass()");
   hf->AddFrame(new_anal_class, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10));
   TGTextButton* new_filt_class = new TGTextButton(hf, "New filtered analysis class");
   new_filt_class->Connect("Clicked()", "KVSimDirGUI", this, "NewFilteredAnalysisClass()");
   hf->AddFrame(new_filt_class, new TGLayoutHints(kLHintsLeft | kLHintsCenterY));

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
   fTESystem->Connect("TextChanged(const char*)", "KVSimDirGUI", this, "UpdateSystemText(const char*)");

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
//   radiob->SetState(kButtonDown);
   radiob = new TGRadioButton(bgroup, "Full");
   radiob->SetState(kButtonDown);
   bgroup->Connect("Clicked(Int_t)", "KVSimDirGUI", this, "FilterType(Int_t)");
   fFilterType = kFTFull;
//   fFilterType = kFTSeuils;
   hf->AddFrame(bgroup, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 2, 2, 2, 2));

   TGVerticalFrame* vf1 = new TGVerticalFrame(hf, 30, 30);
   bgroup = new TGButtonGroup(vf1, "Options");
   phi_rotation_check = new TGCheckButton(bgroup, "Random phi");
   phi_rotation_check->Connect("Toggled(Bool_t)", "KVSimDirGUI", this, "SetRandomPhi(Bool_t)");
   phi_rotation_check->SetToolTipText("Random rotation around beam axis before detection");
   fRandomPhi = kTRUE;
   phi_rotation_check->SetState(kButtonDown);
#ifdef WITH_GEMINI
   gemini_decay = new TGCheckButton(bgroup, "Gemini++ decay");
   gemini_decay->Connect("Toggled(Bool_t)", "KVSimDirGUI", this, "SetGeminiDecay(Bool_t)");
   gemini_decay->SetToolTipText("Use Gemini++ to calculate statistical decay before detection");
   gemini_decay->SetState(kButtonUp);
   //
   TGHorizontalFrame* hhf = new TGHorizontalFrame(vf1, 20);
   TGLabel* gemini_label = new TGLabel(hhf, "Decays/evt.:");
   hhf->AddFrame(gemini_label, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
   gemini_decays = new TGNumberEntry(hhf, 1, 5, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMin, 1);
   gemini_decays->GetNumberEntry()->SetToolTipText("Number of times to decay each event with Gemini++");
   hhf->AddFrame(gemini_decays, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 10, 2, 2, 2));
   gemini_decays->SetState(kFALSE);
   //
   TGHorizontalFrame* hhhf = new TGHorizontalFrame(vf1, 20);
   gemini_add_rot_energy = new TGCheckButton(hhhf, "Add rotational energy");
   gemini_add_rot_energy->Connect("Toggled(Bool_t)", "KVSimDirGUI", this, "SetGeminiDecayAddRotEnergy(Bool_t)");
   gemini_add_rot_energy->SetToolTipText("Add the rotational energy to the excitation energy for each nucleus");
   //gemini_add_rot_energy->SetState(kButtonDown);
   hhhf->AddFrame(gemini_add_rot_energy, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 6, 2, 2, 3));
   gemini_add_rot_energy->SetDisabledAndSelected(kTRUE);
   //
#endif
   fGemDecay = kFALSE;
   fGemAddRotEner = kTRUE;
   fGeoType = kGTROOT;
   vf1->AddFrame(bgroup, new TGLayoutHints(kLHintsTop, 1, 1, 1, 5));
#ifdef WITH_GEMINI
   vf1->AddFrame(hhf, new TGLayoutHints(kLHintsBottom, 2, 2, 2, 2));
   vf1->AddFrame(hhhf, new TGLayoutHints(kLHintsBottom, 3, 3, 2, 4));
#endif
   hf->AddFrame(vf1, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   bgroup = new TGButtonGroup(hf, "Kinematics");
   radiob = new TGRadioButton(bgroup, "CM");
   radiob->SetState(kButtonDown);
   radiob = new TGRadioButton(bgroup, "Lab");
   radiob = new TGRadioButton(bgroup, "Projectile");
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
   if (!gBatchSystemManager) new KVBatchSystemManager;
   gBatchSystemManager->GetDefaultBatchSystem()->cd();
   proof_analysis->SetToolTipText(gBatchSystem->GetTitle());
   proof_analysis->Resize(40, 40);
   proof_analysis->AllowStayDown(kTRUE);

   TGPictureButton* launch_analysis = new TGPictureButton(hf, gClient->GetPicture("query_submit.xpm"));
   launch_analysis->Connect("Clicked()", "KVSimDirGUI", this, "Run()");
   launch_analysis->SetToolTipText("Run analysis/filter");
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
#ifdef WITH_RSQLITE
   // define tables in DB if not already done
   bool sqlite_conversion = false;
   if (!fConfigDB.get_number_of_tables()) {
      sqlite_conversion = true;
      KVSQLite::table datasets("Datasets");
      datasets.add_primary_key("Id");
      datasets.add_column("Name", "TEXT");
      datasets.add_column("Directory", "TEXT");
      datasets.add_column("Table", "TEXT");
      fConfigDB.add_table(datasets);
   }
   if (!sqlite_conversion) {
      // use existing sqlite database to reconstruct simdirs
      ReconstructSimDirsFromDB();
      FillTreeList();
      return;
   }
#endif
   // use .guirc to fill the list of sim dirs
   KVString simdirs = fGuirc.GetValue("SimDirs", "");
   if (simdirs != "") {
#ifdef WITH_RSQLITE
      if (sqlite_conversion) {
         // we are converting from the old TEnv back end to sqlite
         // need to fill table of Datasets
         fConfigDB.prepare_data_insertion("Datasets");
      }
      int ds_table = 0;
#endif
      simdirs.Begin(" ");
      while (!simdirs.End()) {
         KVString simdir = simdirs.Next();
         KVString simdirectory = fGuirc.GetValue(Form("%s.Directory", simdir.Data()), "");
         KVSimDir* sd = new KVSimDir(simdir, simdirectory);
#ifdef WITH_RSQLITE
         if (sqlite_conversion) {
            fConfigDB["Datasets"]["Name"] = simdir;
            fConfigDB["Datasets"]["Directory"] = simdirectory;
            fConfigDB.insert_data_row();
         }
#endif
         sd->AnalyseDirectory();
         fListOfDirs.Add(sd);
      }
#ifdef WITH_RSQLITE
      if (sqlite_conversion) {
         fConfigDB.end_data_insertion();
         TIter itdir(&fListOfDirs);
         KVSimDir* sd = nullptr;
         while ((sd = (KVSimDir*)itdir())) {
            // add table for dataset
            TString ds_table_name = AddTableForDataSet(sd, ds_table);
            // fill table with infos
            FillDataSetTableWithInfos(ds_table_name, sd);
            ++ds_table;
         }
      }
#endif
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
   BremDir->SetEnabled(kFALSE);
   BimpSim->SetEnabled(kFALSE);
   BrefreshDir->SetEnabled(kFALSE);
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
      simdirname.ReplaceAll(" ", "_"); //no spaces in name
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
         fGuirc.SetValue(Form("%s.Directory", simdirname.Data()), sd->GetDirectory());
         fGuirc.SaveLevel(kEnvUser);
#ifdef WITH_RSQLITE
         //update database
         int ds_table_num = 0;
         if (fConfigDB.count("Datasets")) {
            // get name/number of last dataset table
            KVNameValueList table_names = fConfigDB.get_name_value_list("Datasets", "Name", "Table");
            sscanf(table_names.GetParameter(table_names.GetNpar() - 1)->GetString(),
                   "Table_%d", &ds_table_num);
            ++ds_table_num;
         }
         fConfigDB.prepare_data_insertion("Datasets");
         fConfigDB["Datasets"]["Id"].set_null();
         fConfigDB["Datasets"]["Name"] = simdirname;
         fConfigDB["Datasets"]["Directory"] = sd->GetDirectory();
         fConfigDB.insert_data_row();
         fConfigDB.end_data_insertion();
         TString table_name = AddTableForDataSet(sd, ds_table_num);
         FillDataSetTableWithInfos(table_name, sd);
#endif
      }
   }
   dir = fi.fIniDir;
}

void KVSimDirGUI::RefreshSimDir()
{
   if (!fSelectedSimDir) return;
   fSelectedSimDir->AnalyseDirectory();
   fLVsimData->Display(fSelectedSimDir->GetSimDataList());
   fLVfiltData->Display(fSelectedSimDir->GetFiltDataList());
#ifdef WITH_RSQLITE
   fConfigDB.select_data("Datasets", "Table", Form("Name=\"%s\"", fSelectedSimDir->GetName()));
   TString ds_table_name;
   while (fConfigDB.get_next_result()) ds_table_name = fConfigDB["Datasets"]["Table"].get_data<TString>();
   fConfigDB.clear_table(ds_table_name);
   FillDataSetTableWithInfos(ds_table_name, fSelectedSimDir);
#endif
}

void KVSimDirGUI::RemSimDir()
{
   if (!fSelectedSimDir) return;
   fListOfDirs.Remove(fSelectedSimDir);
   // there is no way of removing a name-value pair from a TEnv!!
   fGuirc.SetValue(Form("%s.Directory", fSelectedSimDir->GetName()), "");
#ifdef WITH_RSQLITE
   // remove from datasets table
   fConfigDB.select_data("Datasets", "Table", Form("Name=\"%s\"", fSelectedSimDir->GetName()));
   TString ds_table_name;
   while (fConfigDB.get_next_result()) ds_table_name = fConfigDB["Datasets"]["Table"].get_data<TString>();
   fConfigDB.delete_data("Datasets", Form("Name=\"%s\"", fSelectedSimDir->GetName()));
   // and delete table with dataset data
#endif
   delete fSelectedSimDir;
   fSelectedSimDir = nullptr;
   FillTreeList();
   fLVsimData->RemoveAll();
   fLVfiltData->RemoveAll();
   // update guirc file
   TString simdirs = "";
   TIter next(&fListOfDirs);
   KVSimDir* togo;
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
   fSelectedSimDir = (KVSimDir*)fListOfDirs.FindObject(simdir->GetText());
   if (fSelectedSimDir) {
      fLVsimData->Display(fSelectedSimDir->GetSimDataList());
      fLVfiltData->Display(fSelectedSimDir->GetFiltDataList());
      BremDir->SetEnabled();
      BimpSim->SetEnabled();
      BrefreshDir->SetEnabled();
   }
   else {
      BremDir->SetEnabled(kFALSE);
      BimpSim->SetEnabled(kFALSE);
      BrefreshDir->SetEnabled(kFALSE);
   }
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
      fAnalClassName = gSystem->BaseName(fi.fFilename);
      Int_t idot = fAnalClassName.Index(".");
      if (idot < 0) return;
      fAnalClassName.Remove(idot, 2);
      if (FindClassSourceFiles(fAnalClassName, fAnalClassImp, fAnalClassHeader, fi.fIniDir)) {
         fTEAnalysisClassFileName->SetText(fAnalClassName);
         fAnalClassDir = fi.fIniDir;
         cout << "Found class header " << gSystem->BaseName(fAnalClassHeader) << " and implementation "
              << gSystem->BaseName(fAnalClassImp) << " in " << fAnalClassDir << endl;
      }
   }
   dir = fi.fIniDir;
}

void KVSimDirGUI::EnableEventNumberEntry(Bool_t on)
{
   fNENumberEvents->SetState(!on);
}

void KVSimDirGUI::SelectDataSet(const char* name)
{
   KVDataSet* ds = gDataSetManager->GetDataSet(name);
   if (ds) {
      fDataset = name;
      fSystem = "";
      fRun = "";
      ds->cd();
      // use ROOT geometry if available
      if (ds->GetDataSetEnv("KVMultiDetArray.ROOTGeometry", kTRUE)) {
         GeoType(kGTROOT);
      }
      else {
         GeoType(kGTKaliVeda);
      }
      KVSeqCollection* systems = 0;
      if (gExpDB) systems = gExpDB->GetSystems();
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
   KVDBSystem* sys = gExpDB->GetSystem(sysname);
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

void KVSimDirGUI::UpdateSystemText(const char* s)
{
   // called every time the text changes
   fSystem = s;
}

void KVSimDirGUI::SelectRun(const char* run)
{
   fRun = run;
   fRun.Remove(TString::kBoth, ' ');
   if (!fRun.IsDigit()) fRun = "";
}

void KVSimDirGUI::Run()
{
   // Called when "Process" button is pressed to launch the analysis

   if (fAnalysisTabs->GetCurrent() == 0)      RunAnalysis("tree");
   else if (fAnalysisTabs->GetCurrent() == 1) {
      if (fDataset == "") { // Make sure a dataset has been chosen
         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::Run", "First choose the dataset which describes the geometry!", kMBIconExclamation);
         return;
      }
      RunAnalysis("filter");
   }
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

void KVSimDirGUI::RunAnalysis(const TString& type)
{
   unique_ptr<TList> selected_sim_runs(fLVsimData->GetSelectedObjects());
   unique_ptr<TList> selected_filt_runs(fLVfiltData->GetSelectedObjects());
   TList* runs_to_analyse(nullptr);
   TList* aux_runs_to_analyse(nullptr);
   TString analysis_task;
   if (type == "tree") { // analysis of simulated or filtered events
      if (!selected_sim_runs->GetEntries() && !selected_filt_runs->GetEntries()) {
         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Choose one or more simulated or filtered data files!", kMBIconExclamation);
         return;
      }
      //      if (selected_sim_runs->GetEntries() && selected_filt_runs->GetEntries()) {
      //         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Choose EITHER simulated or filtered data files!", kMBIconExclamation);
      //         return;
      //      }
      if (fAnalClassHeader == "" || fAnalClassImp == "") {
         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Choose a valid analysis class!", kMBIconExclamation);
         return;
      }
      if (selected_sim_runs->GetEntries()) {
         if (selected_filt_runs->GetEntries()) {
            // if both simulated & filtered events are selected, we assume the user wants to analyse filtered
            // data while having access to the original non-filtered data as an auxiliary/"friend" tree
            runs_to_analyse = selected_filt_runs.get();
            analysis_task = "analysis filtered";
            aux_runs_to_analyse = selected_sim_runs.get();
         }
         else {
            runs_to_analyse = selected_sim_runs.get();
            analysis_task = "analysis simulated";
         }
      }
      else {
         runs_to_analyse = selected_filt_runs.get();
         analysis_task = "analysis filtered";
      }
   }
   else {
      // filtering
      if (selected_filt_runs->GetEntries()) {
         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunFilter", "Only simulated events can be filtered!", kMBIconExclamation);
         return;
      }
      if (!selected_sim_runs->GetEntries()) {
         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunFilter", "Choose a simulated data file to filter!", kMBIconExclamation);
         return;
      }
      runs_to_analyse = selected_sim_runs.get();
      analysis_task = "filter simulated";
   }
   KVDataAnalysisTask* anTask = gDataSetManager->GetAnalysisTaskAny(analysis_task);
   gDataAnalyser = KVDataAnalyser::GetAnalyser(anTask->GetDataAnalyser());
   gDataAnalyser->SetAnalysisTask(anTask);
   gDataAnalyser->SetFileList(runs_to_analyse);
   if (aux_runs_to_analyse) gDataAnalyser->SetAuxFileList(aux_runs_to_analyse);
   if (anTask->WithUserClass()) {
      gDataAnalyser->SetUserClass(fAnalClassName);
      if (!gDataAnalyser->IsUserClassValid()) {
         new TGMsgBox(gClient->GetRoot(), MainFrame, "KVSimDirGUI::RunAnalysis", "Compilation failed. Correct mistakes and try again!", kMBIconExclamation);
      }
   }
   else if (strcmp(anTask->GetUserBaseClass(), ""))
      gDataAnalyser->SetUserClass(anTask->GetUserBaseClass(), kFALSE);//task with default "user" class
   Bool_t all_events = fCBAllEvents->IsDown();
   if (!all_events)
      gDataAnalyser->SetNbEventToRead((Long64_t)fNENumberEvents->GetNumber());
   else
      gDataAnalyser->SetNbEventToRead(0);
   Bool_t cancel_batch_job = kFALSE;
   if (fWithPROOF) {
      gBatchSystem->Clear();
      KVNameValueList batchParams;
      gBatchSystem->GetBatchSystemParameterList(batchParams);
      new KVBatchSystemParametersGUI(MainFrame, &batchParams, gDataAnalyser, &cancel_batch_job);
      if (!cancel_batch_job) {
         gBatchSystem->SetBatchSystemParameters(batchParams);
         gDataAnalyser->SetBatchSystem(gBatchSystem);
      }
   }
   fCancelJob = kFALSE;
   if (type == "filter") SetFilterOptions();
   if (!cancel_batch_job && !fCancelJob) gDataAnalyser->Run();
   selected_filt_runs.reset(nullptr);
   selected_sim_runs.reset(nullptr);
   RefreshSimDir();
}

void KVSimDirGUI::SetFilterOptions()
{
   TString geometry;
   if (fGeoType == kGTROOT) geometry = "ROOT";
   else geometry = "KV";
   TString kinema;
   if (fKine == kKCM) kinema = "cm";
   else if (fKine == kKProj) kinema = "proj";
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

   // check system - only for ad hoc systems
   KVDBSystem* sys = (gExpDB ? gExpDB->GetSystem(fSystem) : nullptr);
   if (!sys) {
      KV2Body cd(fSystem.Data());
      // check valid ad hoc system given by user
      if (cd.IsZombie()) {
         Error("SetFilterOptions", "Please check the format of the ad hoc system name you gave");
         WarningBox("Bad System", "Please check the format of the ad hoc system name you gave", kFALSE);
         fCancelJob = kTRUE;
         return;
      }
      cd.CalculateKinematics();
      cd.Print();
   }

   TString options;
   options = Form("Dataset=%s,", fDataset.Data());
   options += Form("System=%s,", fSystem.Data());
   options += Form("Geometry=%s,", geometry.Data());
   options += Form("Filter=%s,", filter.Data());
   options += Form("Kinematics=%s", kinema.Data());
   if (fRun != "") {
      TString r;
      r.Form(",Run=%s", fRun.Data());
      options += r;
   }
   if (!fRandomPhi) options += ",PhiRot=no";
   if (fGemDecay) {
      options += ",Gemini=yes";
      options += Form(",GemDecayPerEvent=%d", (Int_t)gemini_decays->GetNumber());

      if (fGemAddRotEner) options += Form(",GemAddRotEner=yes");
      else  options += Form(",GemAddRotEner=no");
   }

   gDataAnalyser->SetUserClassOptions(options);
}

void KVSimDirGUI::ImportSimulation()
{
   // Import simulation data from currently selected directory

   const char* filetypes[] = {
      "All files", "*.*",
      0, 0
   };
   TGFileInfo fi;
   fi.fIniDir = StrDup(fSelectedSimDir->GetDirectory());
   fi.fFileTypes = filetypes;
   fi.SetMultipleSelection(true);

   new KVFileDialog(gClient->GetDefaultRoot(), MainFrame, kKVFDOpen, &fi);

   bool one_file = (!fi.fMultipleSelection && fi.fFilename);
   bool many_files = (fi.fMultipleSelection && fi.fFileNamesList);//may only be one
   if (one_file || many_files) {
      // set up list of KVSimReader plugins
      KVString plugins = KVBase::GetListOfPlugins("KVSimReader");
      KVString choices;
      plugins.Begin(" ");
      while (!plugins.End()) {
         // get URI corresponding to plugin
         KVString uri = KVBase::GetPluginURI("KVSimReader", plugins.Next());
         if (choices.Length()) choices += " ";
         choices += uri;
      }
      // get model from user
      TString model;
      Bool_t ok;
      new KVDropDownDialog(MainFrame, "Choose the simulation model and type of events", choices, 0, &model, &ok);
      if (ok) {

         if (one_file) {
            unique_ptr<KVSimReader> SR(KVSimReader::MakeSimReader(model));
            SR->SetOutputDirectory(fSelectedSimDir->GetDirectory());
            SR->ConvertAndSaveEventsInFile(fi.fFilename);
         }
         else {
            if (fi.fFileNamesList->GetEntries() == 1) {
               unique_ptr<KVSimReader> SR(KVSimReader::MakeSimReader(model));
               SR->SetOutputDirectory(fSelectedSimDir->GetDirectory());
               SR->ConvertAndSaveEventsInFile(fi.fFileNamesList->First()->GetName());
            }
            else {
               TIter it(fi.fFileNamesList);
               TObject* o;
               int i = 1;
               while ((o = it())) {
                  unique_ptr<KVSimReader> SR(KVSimReader::MakeSimReader(model));
                  SR->SetMultiFiles();
                  SR->SetFileIndex(i++);
                  SR->SetOutputDirectory(fSelectedSimDir->GetDirectory());
                  SR->ConvertAndSaveEventsInFile(o->GetName());
               }
            }
         }

         RefreshSimDir();
      }
   }
   else if (fi.fMultipleSelection && fi.fFileNamesList) {
      fi.fFileNamesList->ls();
   }
}

void KVSimDirGUI::NewFilteredAnalysisClass()
{
   // Get name of new class
   TString classname;
   Bool_t ok;
   new KVInputDialog(MainFrame, "Enter name of new analysis class", &classname, &ok, "Enter name of new analysis class");
   // check new classname is not name of existing class
   KVString impfile, decfile;
   if (KVBase::FindClassSourceFiles(classname, impfile, decfile)) {
      ok = ok && WarningBox("Replacing existing class",
                            Form("%s is the name of an existing class defined in [%s,%s].\nDo you want to overwrite this class?\n(All existing code will be lost)",
                                 classname.Data(), decfile.Data(), impfile.Data()),
                            kTRUE);
   }
   if (ok) {
      KVSimDirFilterAnalyser::Make(classname);
   }
}

void KVSimDirGUI::NewSimulatedAnalysisClass()
{
   // Get name of new class
   TString classname;
   Bool_t ok;
   new KVInputDialog(MainFrame, "Enter name of new analysis class", &classname, &ok, "Enter name of new analysis class");
   // check new classname is not name of existing class
   KVString impfile, decfile;
   if (KVBase::FindClassSourceFiles(classname, impfile, decfile)) {
      ok = ok && WarningBox("Replacing existing class",
                            Form("%s is the name of an existing class defined in [%s,%s].\nDo you want to overwrite this class?\n(All existing code will be lost)",
                                 classname.Data(), decfile.Data(), impfile.Data()),
                            kTRUE);
   }
   if (ok) {
      KVSimDirAnalyser::Make(classname);
   }
}

//__________________________________________
Bool_t KVSimDirGUI::WarningBox(const char* title, const char* msg, Bool_t confirm)
{
   // Warning box in case of problems
   // if confirm=kTRUE we ask for a yes/no answer from the user:
   //     if 'yes' is pressed, we return kTRUE, if 'no', kFALSE.
   // by default, only a 'dismiss' button is shown, and this method always returns kTRUE.

   Bool_t reply = kTRUE;
   if (!confirm)
      new TGMsgBox(gClient->GetRoot(), MainFrame, title, msg, kMBIconExclamation);
   else {
      Int_t ret_code = 0;
      new TGMsgBox(gClient->GetRoot(), MainFrame, title, msg, kMBIconExclamation, kMBYes | kMBNo, &ret_code);
      reply = (ret_code & kMBYes);
   }
   return reply;
}

#ifdef WITH_RSQLITE
void KVSimDirGUI::ReconstructSimDirsFromDB()
{
   // Rebuild all KVSimDir/KVSimFile objects from information in sqlite DB

   fConfigDB.select_data("Datasets");
   KVNameValueList tables;
   while (fConfigDB.get_next_result()) {
      fListOfDirs.Add(new KVSimDir(fConfigDB["Datasets"]["Name"].get_data<TString>(),
                                   fConfigDB["Datasets"]["Directory"].get_data<TString>()));
      tables.SetValue(fConfigDB["Datasets"]["Name"].get_data<TString>(),
                      fConfigDB["Datasets"]["Table"].get_data<TString>());
   }
   for (int i = 0; i < tables.GetNpar(); ++i) {
      KVSimDir* sd = (KVSimDir*)fListOfDirs.FindObject(tables.GetParameter(i)->GetName());
      fConfigDB.select_data(tables.GetValue<TString>(i));
      KVSQLite::table& dstable = fConfigDB[tables.GetValue<TString>(i)];
      while (fConfigDB.get_next_result()) {
         if (dstable["Filtered"].get_data<int>()) {
            // filtered/reconstructed data
            KVSimFile* f = new KVSimFile(
               sd,
               dstable["FileName"].get_data<TString>(), dstable["TreeInfo"].get_data<TString>(),
               dstable["Events"].get_data<int>(), dstable["TreeName"].get_data<TString>(),
               dstable["BranchName"].get_data<TString>(), dstable["DataSet"].get_data<TString>(),
               dstable["System"].get_data<TString>(), dstable["Run"].get_data<int>(),
               dstable["GeoType"].get_data<TString>(), dstable["OrigFile"].get_data<TString>(),
               dstable["FiltType"].get_data<TString>()
            );
            f->SetGemini(dstable["Gemini"].get_data<bool>());
            if (f->IsGemini()) f->SetGemDecayPerEvent(dstable["GemDecayPerEvent"].get_data<int>());
            sd->AddFiltData(f);
         }
         else {
            // simulation data
            KVSimFile* f = new KVSimFile(
               sd,
               dstable["FileName"].get_data<TString>(), dstable["TreeInfo"].get_data<TString>(),
               dstable["Events"].get_data<int>(), dstable["TreeName"].get_data<TString>(),
               dstable["BranchName"].get_data<TString>());
            sd->AddSimData(f);
         }
      }
   }
}
#endif
