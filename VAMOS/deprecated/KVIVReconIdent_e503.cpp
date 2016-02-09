#include "KVIVReconIdent_e503.h"

/**
   WARNING: This class has been deprecated and will eventually be removed. Do
   not use!

   This class is only compiled if the deprecated code is enabled in the build
   configuration (e.g. cmake -DUSE_DEPRECATED_VAMOS=yes). If you enable the
   deprecated code then a large number of warnings will be printed to the
   terminal. To disable these warnings (not advised) compile VAMOS with
   -Wno-deprecated-declarations. Despite these warnings the code should compile
   just fine. The warnings are there to prevent the unwitting use of the
   deprecated code (which should be strongly discouraged).

   BY DEFAULT THIS CLASS IS NOT COMPILED.

   Deprecated by: Peter Wigg (peter.wigg.314159@gmail.com)
   Date:          Thu 17 Dec 17:24:38 GMT 2015
*/

using namespace std;

ClassImp(KVIVReconIdent_e503)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIVReconIdent_e503</h2>
<h4>INDRA identification and calibration, VAMOS trajectory reconstruction,
calibration and identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIVReconIdent_e503::KVIVReconIdent_e503():
   fAnalyseV(NULL),
   fLogV(NULL)
{
   //Default constructor
}

KVIVReconIdent_e503::~KVIVReconIdent_e503()
{
   //Destructor
}

void KVIVReconIdent_e503::InitAnalysis(void)
{
   KVIVReconIdent::InitAnalysis();

   //Create VAMOS Analysisv object
   fLogV = new LogFile;
   if (gBatchSystem) {
      fLogV->Open(Form("%s_vamos.log", gBatchSystem->GetJobName()));
   } else {
      fLogV->Open("Calibration_vamos.log");
   }
   //get VAMOS calibrator for current dataset
   fAnalyseV = Analysisv::NewAnalyser(gDataSet->GetName(), fLogV);


   //Chargement des grilles de E503
   LoadGrids_e503();
}

//_____________________________________
void KVIVReconIdent_e503::InitRun(void)
{
   KVIVReconIdent::InitRun();

   //Connect VAMOS branches in input and output trees

   KVINDRAReconIdent::InitRun();
   fAnalyseV->OpenOutputTree(fIdentTree);
   fAnalyseV->outAttach();
   fAnalyseV->CreateHistograms();
   fAnalyseV->OpenInputTree(fChain->GetTree());
   fAnalyseV->inAttach();

   // set current run number
   fAnalyseV->SetCurrentRun(gMultiDetArray->GetCurrentRunNumber());

   //Set Brho_ref and angle of VAMOS
   fAnalyseV->SetBrhoRef(gVamos->GetBrhoRef());
   fAnalyseV->SetAngleVamos(gVamos->GetAngle());

   //Chargement des cuts graphiques, détecteurs
   Init_e503();

}

//_____________________________________
Bool_t KVIVReconIdent_e503::Analysis(void)
{
   //Identification of INDRA events and ident/reconstruction of VAMOS data

   //fLogV->Log<<"-----------"<<endl;
   //fLogV->Log<<"ev num vamos="<<event<<endl;
   //fLogV->Log<<"GetMult     "<<GetEvent()->GetMult()<<endl;
   //fLogV->Log<<"-----------"<<endl;

// Identification of INDRA events
   if (TestBit(kINDRAdata)) {
      fEventNumber = GetEvent()->GetNumber();
      if (GetEvent()->GetMult() > 0) {

         GetEvent()->IdentifyEvent();
         GetEvent()->CalibrateEvent();
      }
   }


// Ident/Reconstruction of VAMOS data
   fAnalyseV->Treat();
   fIdentTree->Fill();
   fAnalyseV->FillHistograms();

   //fLogV->Log<<"___________________________________________"<<endl;
   //fLogV->Log<<"___________________________________________"<<endl;

   return kTRUE;
}

//_____________________________________
void KVIVReconIdent_e503::EndAnalysis(void)
{

   fLogV->Close();
}

void KVIVReconIdent_e503::Init_e503()
{

   Int_t sys;

   if (gIndra->GetCurrentRunNumber() > 321 && gIndra->GetCurrentRunNumber() < 379) {
      sys = 4840;
   }
   if (gIndra->GetCurrentRunNumber() > 378 && gIndra->GetCurrentRunNumber() < 425) {
      sys = 4848;
   }
   if (gIndra->GetCurrentRunNumber() > 510 && gIndra->GetCurrentRunNumber() < 551) {
      sys = 4040;
   }
   if ((gIndra->GetCurrentRunNumber() > 454 && gIndra->GetCurrentRunNumber() < 510) || (gIndra->GetCurrentRunNumber() > 582 && gIndra->GetCurrentRunNumber() < 592)) {
      sys = 4048;
   }
   fLogV->Log << "sys : " << sys << endl;

   // VAMOS Part : Q Graphical Cuts

   //==========================================================================
   // Identification Si-CsI
   TFile* cfile = new TFile(Form("$KVROOT/KVFiles/INDRA_e503/cuts_brho_%d.root", sys));

   TList* list = new TList();
   if (cfile->IsZombie() == 0) {
      TObject* obj = NULL;
      for (Int_t i = 5; i <= 20; i++) {

         if ((obj = cfile->Get(Form("q%d", i))) && obj->InheritsFrom(TCutG::Class()))
            list->Add(obj);

      }

   } else {
      fLogV->Log << "Error : not reading cuts (Si-CsI)..." << endl;
   }

   fAnalyseV->SetFileCut(list);
   delete cfile;


   //==========================================================================
   cfile = new TFile(Form("$KVROOT/KVFiles/INDRA_e503/cuts_brho_chiosi_%d.root", sys));
   list->Clear();
   if (cfile->IsZombie() == 0) {
      TObject* obj = NULL;
      for (Int_t i = 7; i <= 20; i++) {

         if ((obj = cfile->Get(Form("q%d", i))) && obj->InheritsFrom(TCutG::Class()))
            list->Add(obj);

      }

   } else fLogV->Log << "Error : not reading cuts (ChIo-Si)..." << endl;

   fAnalyseV->SetFileCutChioSi(list);
   delete cfile;


   //==========================================================================
   cfile = new TFile(Form("$KVROOT/KVFiles/INDRA_e503/cuts_brho_sitof_%d.root", sys));
   list->Clear();
   if (cfile->IsZombie() == 0) {
      TObject* obj = NULL;
      for (Int_t i = 6; i <= 20; i++) {

         if ((obj = cfile->Get(Form("q%d", i))) && obj->InheritsFrom(TCutG::Class()))
            list->Add(obj);

      }

   } else fLogV->Log << "Error : not reading cuts (Si-ToF)..." << endl;

   fAnalyseV->SetFileCutChioSi(list);
   delete cfile;
   delete list;

   //==========================================================================

   //============================================
   //Layers definition used inside Identificationv class
   //Those layers will be used to add a correction to the total energy

   KVTarget* tgt = new KVTarget("C", 0.012); //mg/cm2

   //=======================
   //Drift Chamber #1
   KVDetector* dcv1 = new KVDetector;

   // Entrance window
   KVMaterial* win;
   win = new KVMaterial("Myl", 1.5 * KVUnits::um);

   // Gas Layer
   KVMaterial* gas;
   gas = new KVMaterial("C4H10", 130.*KVUnits::mm);
   gas->SetPressure(18.*KVUnits::mbar);

   // Exit window
   KVMaterial* wout;
   wout = new KVMaterial("Myl", 1.5 * KVUnits::um);

   dcv1->AddAbsorber(win);
   dcv1->AddAbsorber(gas);
   dcv1->AddAbsorber(wout);
   //=======================

   KVMaterial* sed = new KVMaterial("Myl", 0.9 * KVUnits::um);

   //=======================
   //Drift Chamber #2
   KVDetector* dcv2 = new KVDetector;

   // Entrance window
   KVMaterial* win2;
   win2 = new KVMaterial("Myl", 1.5 * KVUnits::um);

   // Gas Layer
   KVMaterial* gas2;
   gas2 = new KVMaterial("C4H10", 130.*KVUnits::mm);
   gas2->SetPressure(18.*KVUnits::mbar);

   // Exit window
   KVMaterial* wout2;
   wout2 = new KVMaterial("Myl", 1.5 * KVUnits::um);
   dcv2->AddAbsorber(win2);
   dcv2->AddAbsorber(gas2);
   dcv2->AddAbsorber(wout2);
   //=======================

   //=======================
   //Ionisation Chamber
   KVDetector* ic = new KVDetector;

   // Entrance window
   KVMaterial* winic;
   winic = new KVMaterial("Myl", 1.5 * KVUnits::um);

   // Gas Layer
   KVMaterial* dead;
   dead = new KVMaterial("C4H10", 20.68 * KVUnits::mm);
   dead->SetPressure(40.*KVUnits::mbar);

   KVMaterial* active;
   active = new KVMaterial("C4H10", 104.57 * KVUnits::mm);
   active->SetPressure(40.*KVUnits::mbar);

   ic->AddAbsorber(winic);
   ic->AddAbsorber(dead);
   ic->AddAbsorber(active);
   //=======================

   KVMaterial* isogap1 = new KVMaterial("C4H10", 12.78 * KVUnits::mm);

   isogap1->SetPressure(40.*KVUnits::mbar);

   KVMaterial* ssi = new KVMaterial("Si", 530.*KVUnits::um);

   KVMaterial* isogap2 = new KVMaterial("C4H10", 136.5 * KVUnits::mm);

   isogap2->SetPressure(40.*KVUnits::mbar);

   KVMaterial* ccsi = new KVMaterial("CsI", 1.0 * KVUnits::cm);
   //============================================
   // VAMOS Part : Total Energy Correction


   fAnalyseV->SetTarget(tgt);
   fAnalyseV->SetDC1(dcv1);
   fAnalyseV->SetSed(sed);
   fAnalyseV->SetDC2(dcv2);
   fAnalyseV->SetIC(ic);
   fAnalyseV->SetGap1(isogap1);
   fAnalyseV->SetSi(ssi);
   fAnalyseV->SetGap2(isogap2);
   fAnalyseV->SetCsI(ccsi);

   //============================================


   //============================================
   KVDetector* kvd_si = new KVDetector("Si", 530.*KVUnits::um);  //define a Silicon detector of 500 microm thick
   KVDetector* kvd_csi = new KVDetector("CsI", 1.);              //define a CsI detector of 1 cm thick
   KVDetector* gap = new KVDetector("C4H10", 136.5 * KVUnits::mm);
   gap->SetPressure(40.*KVUnits::mbar);

   fAnalyseV->SetTel1(kvd_si);
   fAnalyseV->SetTel2(gap);
   fAnalyseV->SetTel3(kvd_csi);
   //============================================

   return;
}

Bool_t KVIVReconIdent_e503::LoadGrids_e503()
{

   const char* grid_map = "grid_vamos.dat";

   printf("Attempting to load grids from %s...\n", grid_map);

   if (gIDGridManager == 0) {
      printf("gIDGridManager not running\n");
      return 1;
   }

   char ds_path[256];
   sprintf(ds_path, "null");

   if (gDataSet->GetDataSetDir() == 0) {
      printf("Failed to retrieve data set dir string\n");
      return 1;
   }

   sprintf(ds_path, "%s/%s", gDataSet->GetDataSetDir(), grid_map);

   printf("Reading grid map: %s\n", grid_map);
   if (gIDGridManager->ReadAsciiFile(ds_path) != 0) {
      printf("Grids Si-CsI from Vamos loaded\n");
   } else {
      return 1;
   }
   //============================================================================
   const char* grid_map2 = "grid_chio_si_vamos.dat";

   printf("Attempting to load grids from %s...\n", grid_map2);

   if (gIDGridManager == 0) {
      printf("gIDGridManager not running\n");
      return 1;
   }

   char ds_path2[256];
   sprintf(ds_path2, "null");

   if (gDataSet->GetDataSetDir() == 0) {
      printf("Failed to retrieve data set dir string\n");
      return 1;
   }

   sprintf(ds_path2, "%s/%s", gDataSet->GetDataSetDir(), grid_map2);

   printf("Reading grid map: %s\n", grid_map2);
   if (gIDGridManager->ReadAsciiFile(ds_path2) != 0) {
      printf("Grids Chio-Si from Vamos loaded\n");
   } else {
      return 1;
   }
   //============================================================================
   const char* grid_map3 = "grid_si_tof_vamos.dat";

   printf("Attempting to load grids from %s...\n", grid_map3);

   if (gIDGridManager == 0) {
      printf("gIDGridManager not running\n");
      return 1;
   }

   char ds_path3[256];
   sprintf(ds_path3, "null");

   if (gDataSet->GetDataSetDir() == 0) {
      printf("Failed to retrieve data set dir string\n");
      return 1;
   }

   sprintf(ds_path3, "%s/%s", gDataSet->GetDataSetDir(), grid_map3);

   printf("Reading grid map: %s\n", grid_map3);
   if (gIDGridManager->ReadAsciiFile(ds_path3) != 0) {
      printf("Grids Si-Tof from Vamos loaded\n");
   } else {
      cout << "Can't find Si-Tof Grids" << endl;
      return 1;
   }

   return 0;

}

