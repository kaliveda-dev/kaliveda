//Author: Eric Bonnet

#include "KVINDRADstToRootTransfert.h"
#include "KVClassFactory.h"
#include "KVBatchSystem.h"

#include "TROOT.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "Riostream.h"
#include "TFile.h"
#include "TTree.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "KVINDRA.h"
#include "KVINDRACodes.h"
#include "KVINDRAReconNuc.h"
#include "KVINDRAReconEvent.h"
#include "KVChIo.h"
#include "KVSilicon.h"
#include "KVCsI.h"
#include "KVPhoswich.h"
#include "KVDetector.h"
#include "KVIDTelescope.h"
#include "KVDBRun.h"
#include "KVDataRepositoryManager.h"
#include "KVDataSetManager.h"
#include "KVDataRepository.h"
#include "KVBatchSystem.h"
#include "KVGANILDataReader.h"
#include "KVINDRADB.h"

typedef KVDetector* (KVINDRADstToRootTransfert::*FNMETHOD)(int, int);

using namespace std;

ClassImp(KVINDRADstToRootTransfert)


KVINDRADstToRootTransfert::KVINDRADstToRootTransfert()
{
   //Default constructor
}

KVINDRADstToRootTransfert::~KVINDRADstToRootTransfert()
{
   //Destructor
}

void KVINDRADstToRootTransfert::InitRun()
{
   Info("InitRun", "ds InitRun");

   KVString dst_file = gDataSet->GetFullPathToRunfile("dst", fRunNumber);
   Info("InitRun", "dst file %s", dst_file.Data());

   Info("InitRun", "%s %s", gDataSet->GetDataPathSubdir(), gDataSet->GetRunfileName("dst", fRunNumber));
   gDataRepository->CopyFileFromRepository(gDataSet, "dst", gDataSet->GetRunfileName("dst", fRunNumber), ".");
   gROOT->ProcessLine(".! ls -lhrt *");

   Info("InitRun", "Starting analysis of run %d", fRunNumber);

   TDatime now1;
   Info("InitRun", "Debut lecture DST %s", now1.AsString());
   DefineSHELLVariables();


   ReadDST();

   Info("InitRun", "Bilan ressource apres ReadDST");
   ProcInfo_t pid;
   if (gSystem->GetProcInfo(&pid) == 0) {
      TString du = gSystem->GetFromPipe("du -hs");
      TObjArray* toks = du.Tokenize(" .");
      TString disk = ((TObjString*)toks->At(0))->String();
      delete toks;
      cout << "     ------------- Process infos -------------" << endl;
      printf(" CpuUser = %f s.     VirtMem = %f MB      DiskUsed = %s\n",
             pid.fCpuUser, pid.fMemVirtual / 1024., disk.Data());
   }
   else {
      Warning("InitRun", "pas d info disponible sur le bilan ressource");
   }


   TDatime now2;
   Info("InitRun", "Fin lecture DST %s", now2.AsString());

   KVString inst;
   inst.Form(".! rm %s", gDataSet->GetRunfileName("dst", fRunNumber));
   gROOT->ProcessLine(inst.Data());

   inst.Form(".! rm xrun%d", fRunNumber);
   gROOT->ProcessLine(inst.Data());

}

void KVINDRADstToRootTransfert::ReadDST()
{

   KVString inst;
   inst.Form(".! %s", KVBase::GetBINDIRFilePath("veda2root_kali"));
   gROOT->ProcessLine(inst.Data());

}

void KVINDRADstToRootTransfert::ProcessRun()
{

   // Convert DST file for 1 run to KaliVeda ROOT format.
   // By default the new ROOT file will be written in the same data repository as the DST file we are reading.
   // This can be changed by setting the environment variable(s):
   //
   //     DSTtoROOT.DataAnalysisTask.OutputRepository:     [name of repository]
   //     [name of dataset].DSTtoROOT.DataAnalysisTask.OutputRepository:         [name of repository]
   //
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used. If neither is defined, the new file will be written in the same repository as
   // the DST file (if possible, i.e. if repository is not remote).

   ifstream f_in("list_of_files");
   KVString line = "";

   Int_t nfiles, necrit, rn_verif;
   nfiles = necrit = rn_verif = 0;
   TObjArray* toks = 0;
   while (f_in.good()) {
      line.ReadLine(f_in);
      if (line != "") {
         toks = line.Tokenize("=");
         Int_t val = ((TObjString*)toks->At(1))->GetString().Atoi();

         if (line.BeginsWith("file_number="))         nfiles = val;
         else if (line.BeginsWith("write_evts="))     necrit = val;
         else if (line.BeginsWith("run_number="))     rn_verif = val;
         else {}
         delete toks;
      }
      else {}
   }
   f_in.close();

   if (rn_verif != fRunNumber) {
      Error("ProcessRun", "Le numero de run inscrit dans le fichier list_of_files (%d) est different du run courant (%d)", rn_verif, fRunNumber);
      Error("ProcessRun", "\t->analysis is going to stop");

      TString inst;
      inst.Form(".! rm list_of_files");
      gROOT->ProcessLine(inst.Data());

      inst.Form(".! rm arbre_root_*.txt");
      gROOT->ProcessLine(inst.Data());

      return;
   }
   Info("ProcessRun", "Lecture de list_of_files :\n - nbre de fichiers a lire %d\n - nbre d evts a lire %d\n", nfiles, necrit);

   TDatime now1;
   Info("ProcessRun", "Debut Conversion au format du run %d ROOT %s", fRunNumber, now1.AsString());

   KVMultiDetArray::MakeMultiDetector(gDataSet->GetName(), fRunNumber);

   KVINDRAReconEvent* evt = new KVINDRAReconEvent();
   evt->SetTitle(gIndraDB->GetRun(gIndra->GetCurrentRunNumber())->GetSystem()->GetName());
   evt->SetName(gIndraDB->GetRun(gIndra->GetCurrentRunNumber())->GetName());

   // Create new ROOT file with TTree for converted events.

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(gDataSet->GetOutputRepository("DSTtoROOT"), gDataSet->GetName());

   TFile* fi = OutputDataset->NewRunfile("root", fRunNumber);

   KVString stit;
   stit.Form("%s : %s : root events converted from DST",
             gIndraDB->GetRun(fRunNumber)->GetName(), gIndraDB->GetRun(fRunNumber)->GetTitle());
   //tree for reconstructed events
   data_tree = new TTree("ReconstructedEvents", stit.Data());
#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
#if ROOT_VERSION_CODE < ROOT_VERSION(5,26,1)
   // The TTree::OptimizeBaskets mechanism is disabled, as for ROOT versions < 5.26/00b
   // this lead to a memory leak
   data_tree->SetAutoFlush(0);
#endif
#endif
   //leaves for reconstructed events
   KVEvent::MakeEventBranch(data_tree, "INDRAReconEvent", "KVINDRAReconEvent", evt);

   //tree for raw data
   rawtree = new TTree("RawData", Form("%s : %s : raw data",
                                       gIndraDB->GetRun(fRunNumber)->GetName(), gIndraDB->GetRun(fRunNumber)->GetTitle()));
   rawtree->Branch("RunNumber", &fRunNumber, "RunNumber/I");
   rawtree->Branch("EventNumber", &fEventNumber, "EventNumber/I");

   // the format of the raw data tree must be the same as that generated by KVGanilDataReader::SetUserTree
   // with the option "arrays".
   // we depend on it in KVINDRAReconDataAnalyser in order to read the raw data and set the detector acquisition parameters.
   // the event numbers of the two trees must be synchronised
   TString raw_opt = "arrays";

   KVGANILDataReader* raw_data = gDataSet->OpenRunfile<KVGANILDataReader>("raw", fRunNumber);
   if (!raw_data) {
      Error("ProcessRun", "No RAW data file available for this run. ABORT!!");
      exit(1);
   }
   raw_data->SetUserTree(rawtree, raw_opt.Data());
   Info("InitRun", "Created raw data tree (%s : %s). Format: %s",
        rawtree->GetName(), rawtree->GetTitle(), raw_opt.Data());
   // we use the KVGANILDataReader just in order to create the required TTree branches
   // and initialize the acquisition parameter objects with the index given by the acquisition
   // we can now delete it
   delete raw_data;
   // get list of all acquisition parameters
   //params = gIndra->GetACQParams();
   // internal data member pointers are pointing to internal array of GTGanilData object
   // used by now deleted KVGANILDataReader. need to set them to internal fData member.
   TIter nextparam(params);
   KVACQParam* acqpar;
   while ((acqpar = (KVACQParam*)nextparam())) acqpar->UseInternalDataMember();
   // reset addresses of branches which were pointing to internal members of deleted KVGANILDataReader.
   rawtree->SetBranchAddress("NbParFired", &NbParFired);
   rawtree->SetBranchAddress("ParVal", ParVal);
   rawtree->SetBranchAddress("ParNum", ParNum);

   events_good = events_read = 0;

   if (camp2) {
      Info("ProcessRun", "Special treatment INDRA 2nd campaign data:");
      Info("ProcessRun", "Particles detected in phoswich ring 1 have general ID code=4");
      Info("ProcessRun", "After translation, they will have Veda ID code=2 (like 1st campaign)");
   }

   Info("ProcessRun", "Bilan ressource avant lecture des %d fichiers ascii", nfiles);
   ProcInfo_t pid;
   if (gSystem->GetProcInfo(&pid) == 0) {
      TString du = gSystem->GetFromPipe("du -hs");
      TObjArray* toks = du.Tokenize(" .");
      TString disk = ((TObjString*)toks->At(0))->String();
      delete toks;
      cout << "     ------------- Process infos -------------" << endl;
      printf(" CpuUser = %f s.     VirtMem = %f MB      DiskUsed = %s\n",
             pid.fCpuUser, pid.fMemVirtual / 1024., disk.Data());
   }


   KVString inst;
   fEventNumber = 1;
   for (Int_t nf = 1; nf <= nfiles; nf += 1) {

      stit.Form("arbre_root_%d.txt", nf);
      ifstream f_data(stit.Data());
      events_in_file = 0;
      while (f_data.good()) {
         lire_evt(f_data, evt);
         evt->Clear();

         if (events_read % 10000 == 0 && events_read > 0) {
            cout << " +++ " << events_read << " events processed +++ " << endl;
            ProcInfo_t pid;
            if (gSystem->GetProcInfo(&pid) == 0) {
               TString du = gSystem->GetFromPipe("du -hs");
               TObjArray* toks = du.Tokenize(" .");
               TString disk = ((TObjString*)toks->At(0))->String();
               delete toks;
               cout << "     ------------- Process infos -------------" << endl;
               printf(" CpuUser = %f s.     VirtMem = %f MB      DiskUsed = %s\n",
                      pid.fCpuUser, pid.fMemVirtual / 1024., disk.Data());
            }
         }
      }
      f_data.close();

      inst.Form(".! rm arbre_root_%d.txt", nf);

      Info("ProcessRun", "Bilan ressource apres lecture du fichier numero %d/%d", nf, nfiles);
      ProcInfo_t pid;
      if (gSystem->GetProcInfo(&pid) == 0) {
         TString du = gSystem->GetFromPipe("du -hs");
         TObjArray* toks = du.Tokenize(" .");
         TString disk = ((TObjString*)toks->At(0))->String();
         delete toks;
         cout << "     ------------- Process infos -------------" << endl;
         printf(" CpuUser = %f s.     VirtMem = %f MB      DiskUsed = %s\n",
                pid.fCpuUser, pid.fMemVirtual / 1024., disk.Data());
      }

      gROOT->ProcessLine(inst.Data());
   }

   if (gIndraDB->GetRun(gIndra->GetCurrentRunNumber())->GetEvents()) {
      //check number of events against scaler info for this run
      Double_t check_events = (1.*events_read) / ((Double_t)necrit);
      cout << "Nb. evts read from files: " << events_read << endl;
      cout << "Nb. evts written to TTree: " << events_good << endl;
      cout << "Nb. evts in FORTRAN file: " << necrit << endl;
      if (check_events < 0.99) cout << "ERROR: number of events read <99% number in FORTRAN file" << endl;
   }

   //At the end of each run we:
   // write the tree into the new file
   // add the file to the repository

   fi->cd();

   gDataAnalyser->WriteBatchInfo(data_tree);
   data_tree->Write();     //write tree to file
   rawtree->Write();

   //add new file to repository
   OutputDataset->CommitRunfile("root", fRunNumber, fi);

   TDatime now2;
   Info("ProcessRun", "Fin Conversion format ROOT %s", now2.AsString());


   inst.Form(".! rm list_of_files");
   gROOT->ProcessLine(inst.Data());

}

void KVINDRADstToRootTransfert::EndRun()
{

   Info("EndRun", "ds EndRun");


}

void KVINDRADstToRootTransfert::SubmitTask()
{

   if (gDataSet != GetDataSet()) GetDataSet()->cd();
   SetCampagneNumber();

   if (fCampNumber == -1) return;

   //InitAnalysis();
   Info("SubmitTask", "Liste de runs : %s", GetRunList().AsString());
   //loop over runs
   Info("SubmitTask", "RunningInLaunchDirectory : %d", Int_t(RunningInLaunchDirectory()));

   GetRunList().Begin();
   while (!GetRunList().End()) {
      Info("SubmitTask", "%s", GetBatchName());
      fRunNumber = GetRunList().Next();
      Info("SubmitTask", "Traitement du Run %d", fRunNumber);
      InitRun();
      ProcessRun();
      EndRun();
   }

   //EndAnalysis();

}

//_____________________________________________________________________________________________//
void KVINDRADstToRootTransfert::DefineSHELLVariables()
{


   Info("DefineSHELLVariables", "campagne %d", fCampNumber);

   KVString shell_var;

   shell_var.Form("%d", fCampNumber);
   gSystem->Setenv("CAMPAGNE", shell_var.Data());

// shell_var.Form("%s",gSystem->ExpandPathName("$PWD"));
// gSystem->Setenv("DIR_PERSO",shell_var.Data());

   shell_var.Form("%s/lib", gSystem->ExpandPathName("$KVROOT"));
   gSystem->Setenv("REP_LIBS", shell_var.Data());

   shell_var.Form("%s/src/faire_arbre_c%d.f", gSystem->ExpandPathName("$KVROOT"), fCampNumber);
   gSystem->Setenv("FOR_PERSO", shell_var.Data());

   shell_var.Form("run%d", fRunNumber);
   gSystem->Setenv("RUN_PREFIX", shell_var.Data());

   //gSystem->Setenv("DST_FILE",gDataSet->GetRunfileName("dst",fRunNumber));

   camp1 = camp2 = camp4 = kFALSE;

   if (fCampNumber == 1) {

      shell_var.Form("%s/veda%d/for", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_FOR", shell_var.Data());

      shell_var.Form("%s/veda%d/files/", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_DATA", shell_var.Data());

      camp1 = kTRUE;

   }
   else if (fCampNumber == 2) {

      shell_var.Form("%s/veda%d/for%d", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber, fCampNumber);
      gSystem->Setenv("VEDA_FOR", shell_var.Data());

      shell_var.Form("%s/veda%d/data/", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_DATA", shell_var.Data());

      camp2 = kTRUE;

   }
   else if (fCampNumber == 4) {

      shell_var.Form("%s/veda%d/for/include", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_FOR", shell_var.Data());

      shell_var.Form("%s/veda%d/data/", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_DATA", shell_var.Data());

      camp4 = kTRUE;

   }
   else {}



}
//_____________________________________________________________________________________________//
KVDetector* KVINDRADstToRootTransfert::Code2and9and10(Int_t ring, Int_t mod)
{
//code = 2 : part. ident. dans CsI
   //for 1st campaign: code=2 for phoswich (ring 1) also
//code = 9 : ident. incomplete dans CsI ou Phoswich (Z.min)
//code =10 : ident. "entre les lignes" dans CsI
//set coder values of detectors concerned by a particle with ID code 2
//Les codes 9 et 10 sont des codes qui ont ete ajoutes pour donner
//au code 2 la notion d'identification sans ambiguites.

   //returns pointer of the detector for which
   //KVINDRAReconNuc::Reconstruct must be called.
   //cout << "Code2and9and10" << endl;
   if (ring == 1) {
      if (!camp4) {
         //phoswich
         KVPhoswich* phos =
            (KVPhoswich*)gIndra->GetDetectorByType(ring, mod, Phos_R);
         if (!phos) return 0;
         phos->SetEnergy(de1);
         phos->GetACQParam("R")->SetData((UShort_t)canal[Phos_R]);
         phos->GetACQParam("L")->SetData((UShort_t)canal[Phos_L]);
         phos->GetACQParam("T")->SetData((UShort_t)mt);
         identifying_telescope = gIndra->GetIDTelescope(Form("PHOS_R_L_%02d", mod));
         return phos;
      }
      else {   // campagne 4: GSI Campaign
         KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
         if (!csi) return 0;
         KVSilicon* si = (KVSilicon*) gIndra->GetDetectorByType(ring, mod, Si_GG);
         if (si)si->SetEnergy(de2);
         csi->SetEnergy(de3);
         if (si) {
            si->GetACQParam("GG")->SetData((UShort_t)canal[Si_GG]);
            si->GetACQParam("PG")->SetData((UShort_t)canal[Si_PG]);
            si->GetACQParam("T")->SetData((UShort_t)canal[Si_T]);
         }
         csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
         csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
         csi->GetACQParam("T")->SetData((UShort_t)mt);
         identifying_telescope = gIndra->GetIDTelescope(Form("CSI_R_L_%02d%02d", ring, mod));
         return csi;
      }
   }
   else if (ring >= 2 && ring <= 9) {
      //chio-si-csi
      KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (!csi) return 0;
      KVSilicon* si = (KVSilicon*) gIndra->GetDetectorByType(ring, mod, Si_GG);
      KVChIo* chio = (KVChIo*)csi->GetChIo();
      if (!chio) return 0;
      chio->SetEnergy(de1);
      if (si)si->SetEnergy(de2);
      csi->SetEnergy(de3);
      chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
      chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
      chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
      if (si) {
         si->GetACQParam("GG")->SetData((UShort_t)canal[Si_GG]);
         si->GetACQParam("PG")->SetData((UShort_t)canal[Si_PG]);
         si->GetACQParam("T")->SetData((UShort_t)canal[Si_T]);
      }
      csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
      csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
      csi->GetACQParam("T")->SetData((UShort_t)mt);
      identifying_telescope = gIndra->GetIDTelescope(Form("CSI_R_L_%02d%02d", ring, mod));
      return csi;
   }
   else if (ring > 9) {
      //chio-csi or chio-si75-sili-csi
      KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (!csi) return 0;
      KVChIo* chio = (KVChIo*)csi->GetChIo();
      if (!chio) return 0;
      KVSi75* si75 = 0;
      KVSiLi* sili = 0;
      if (canal[Si75_GG] > 0) {
         si75 = (KVSi75*)gIndra->GetDetectorByType(ring, mod, Si75_GG);
      }
      if (canal[SiLi_GG] > 0) {
         sili = (KVSiLi*)gIndra->GetDetectorByType(ring, mod, SiLi_GG);
      }
      chio->SetEnergy(de1);
      if (si75)si75->SetEnergy(de4);
      if (sili)sili->SetEnergy(de5);
      csi->SetEnergy(de3);
      chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
      chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
      chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
      if (si75) {
         si75->GetACQParam("GG")->SetData((UShort_t)canal[Si75_GG]);
         si75->GetACQParam("PG")->SetData((UShort_t)canal[Si75_PG]);
         si75->GetACQParam("T")->SetData((UShort_t)canal[Si75_T]);
      }
      if (sili) {
         sili->GetACQParam("GG")->SetData((UShort_t)canal[SiLi_GG]);
         sili->GetACQParam("PG")->SetData((UShort_t)canal[SiLi_PG]);
         sili->GetACQParam("T")->SetData((UShort_t)canal[SiLi_T]);
      }
      csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
      csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
      csi->GetACQParam("T")->SetData((UShort_t)mt);
      identifying_telescope = gIndra->GetIDTelescope(Form("CSI_R_L_%02d%02d", ring, mod));
      return csi;
   }
   return 0;
}
//_____________________________________________________________________________________________//
KVDetector* KVINDRADstToRootTransfert::Code3(Int_t ring, Int_t mod)
{
// cout << "Code3" << endl;
//code = 3 : fragment identifie dans Si-CsI
//       ou fragment ident. dans Si75-SiLi ou SiLi-CsI
   //set coder values of detectors concerned by a particle with ID code 3
   //returns pointer of the detector for which
   //KVINDRAReconNuc::Reconstruct must be called.

   // First Ring if GSI Campaign
   if (ring == 1 && camp4) {
      KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (!csi) return 0;
      KVSilicon* si = (KVSilicon*) gIndra->GetDetectorByType(ring, mod, Si_GG);
      if (!si) return 0;
      si->SetEnergy(de2);
      csi->SetEnergy(de3);
      si->GetACQParam("GG")->SetData((UShort_t)canal[Si_GG]);
      si->GetACQParam("PG")->SetData((UShort_t)canal[Si_PG]);
      si->GetACQParam("T")->SetData((UShort_t)canal[Si_T]);
      csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
      csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
      csi->GetACQParam("T")->SetData((UShort_t)mt);
      identifying_telescope = gIndra->GetIDTelescope(Form("SI_CSI_%02d%02d", ring, mod));
      return csi;
   }
   else if (ring >= 2 && ring <= 9) {
      //chio-si-csi
      KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (!csi) return 0;
      KVSilicon* si = (KVSilicon*) gIndra->GetDetectorByType(ring, mod, Si_GG);
      if (!si) return 0;
      KVChIo* chio = (KVChIo*)csi->GetChIo();
      if (!chio) return 0;
      chio->SetEnergy(de1);
      chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
      chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
      chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
      si->SetEnergy(de2);
      csi->SetEnergy(de3);
      si->GetACQParam("GG")->SetData((UShort_t)canal[Si_GG]);
      si->GetACQParam("PG")->SetData((UShort_t)canal[Si_PG]);
      si->GetACQParam("T")->SetData((UShort_t)canal[Si_T]);
      csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
      csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
      csi->GetACQParam("T")->SetData((UShort_t)mt);
      identifying_telescope = gIndra->GetIDTelescope(Form("SI_CSI_%02d%02d", ring, mod));
      return csi;
   }
   else if (ring >= 10) {
      KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (!csi) return 0;
      KVChIo* chio = (KVChIo*)csi->GetChIo();
      if (!chio) return 0;
      KVSi75* si75 = (KVSi75*)gIndra->GetDetectorByType(ring, mod, Si75_GG);
      if (!si75) return 0;
      KVSiLi* sili = (KVSiLi*)gIndra->GetDetectorByType(ring, mod, SiLi_GG);
      if (!sili) return 0;
      chio->SetEnergy(de1);
      si75->SetEnergy(de4);
      sili->SetEnergy(de5);
      chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
      chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
      chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
      si75->GetACQParam("GG")->SetData((UShort_t)canal[Si75_GG]);
      si75->GetACQParam("PG")->SetData((UShort_t)canal[Si75_PG]);
      si75->GetACQParam("T")->SetData((UShort_t)canal[Si75_T]);
      sili->GetACQParam("GG")->SetData((UShort_t)canal[SiLi_GG]);
      sili->GetACQParam("PG")->SetData((UShort_t)canal[SiLi_PG]);
      sili->GetACQParam("T")->SetData((UShort_t)canal[SiLi_T]);
      if (canal[CsI_R] > 0.) {
         csi->SetEnergy(de3);
         csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
         csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
         csi->GetACQParam("T")->SetData((UShort_t)mt);
         identifying_telescope = gIndra->GetIDTelescope(Form("SILI_CSI_%02d%02d", ring, mod));
         return csi;
      }
      else {
         identifying_telescope = gIndra->GetIDTelescope(Form("SI75_SILI_%02d%02d", ring, mod));
         return sili;
      }
   }
   return 0;
}
//_____________________________________________________________________________________________//
KVDetector* KVINDRADstToRootTransfert::Code0(Int_t ring, Int_t mod)
{
// cout << "Code0" << endl;
//code = 0 : gamma
//  ** Mention speciale pour les Gammas :
//
//    Pour ceux-ci, on ne stocke que certaines infos disponibles :
//
//     - Signal codeur CsI/Phoswich rapide  --> de3(i)  canaux en negatif
//     - Marqueur de temps --> Mt(i)

   if (ring == 1 && !camp4) {
      //phoswich
      KVPhoswich* phos = (KVPhoswich*)gIndra->GetDetectorByType(ring, mod, Phos_R);
      if (!phos) return 0;
      phos->GetACQParam("R")->SetData((UShort_t)TMath::Abs(de3));
      phos->GetACQParam("T")->SetData((UShort_t)mt);
      identifying_telescope = gIndra->GetIDTelescope(Form("PHOS_R_L_%02d", mod));
      return phos;
   }
   KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
   if (csi) {
      csi->GetACQParam("T")->SetData((UShort_t)mt);
      csi->GetACQParam("R")->SetData((UShort_t)TMath::Abs(de3));
   }
   identifying_telescope = gIndra->GetIDTelescope(Form("CSI_R_L_%02d%02d", ring, mod));
   return csi;
}
//_____________________________________________________________________________________________//
KVDetector* KVINDRADstToRootTransfert::Code1(Int_t ring, Int_t mod)
{
// cout << "Code1" << endl;
//code = 1 : neutron (seulement couronnes 2 a 9)

   if (ring < 2 || ring > 9) return 0;

   KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
   if (!csi) return 0;
   csi->SetEnergy(de3);
   csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
   csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
   csi->GetACQParam("T")->SetData((UShort_t)mt);
   identifying_telescope = gIndra->GetIDTelescope(Form("CSI_R_L_%02d%02d", ring, mod));
   return csi;
}
//_____________________________________________________________________________________________//
KVDetector* KVINDRADstToRootTransfert::Code4and5and6and8(Int_t ring, Int_t mod)
{
//  code = 4 : fragment identifie dans ChIo-Si
//                ou fragment ou part. identifie dans ChIo-Si75
   //for 2nd campaign: code=4 => phoswich (ring 1) also
//  code = 5 : fragment non-ident. (montee de Bragg)(Z min.)
//                      ou stoppe dans Chio (Z min)
//code = 6 : frag. cree par la coherence CsI  (stoppe ds Si.)
// En general, lorsqu'il n'y a pas "multiple hit" dans un module
// INDRA, canal contient les valeurs bruts de codeurs. Dans le
// cas d'un "multiple hit" avec creation de particule , canal
// contient les valeurs estimees de la contribution CHIO ou SI
// obtenues par la coherence.
// code = 8 : multiple comptage dans ChIo avec arret
//                - dans les siliciums (cour. 2-9)
//                - dans les CsI (cour. 10-17)
// Nota: Pour le code 8 la contribution ChIo est partagee egalement entre
//      toutes les particules qui s'arretent dans les modules suivants.
//      Le Z individuel est donc surement faux mais cela permet
//      d'avoir une assez bonne estimation du Z.total.
//
// 09/2012 pour la 2e campagne presence de code14 qui semble etre
// des codes 4 donc on les traite comme tels

   if (ring == 1 && camp2) {
      //phoswich
      KVPhoswich* phos =
         (KVPhoswich*)gIndra->GetDetectorByType(ring, mod, Phos_R);
      if (!phos) return 0;
      phos->SetEnergy(de1);
      phos->GetACQParam("R")->SetData((UShort_t)canal[Phos_R]);
      phos->GetACQParam("L")->SetData((UShort_t)canal[Phos_L]);
      phos->GetACQParam("T")->SetData((UShort_t)mt);
      identifying_telescope = gIndra->GetIDTelescope(Form("PHOS_R_L_%02d", mod));
      return phos;
   }
   else if (ring >= 2 && ring <= 9) {
      //chio-si
      KVSilicon* si = (KVSilicon*) gIndra->GetDetectorByType(ring, mod, Si_GG);
      if (!si) return 0;
      KVChIo* chio = (KVChIo*)si->GetChIo();
      if (!chio) return 0;
      chio->SetEnergy(de1);
      chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
      chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
      chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
      if (de2 > 0.0) {
         si->SetEnergy(de2);
         si->GetACQParam("GG")->SetData((UShort_t)canal[Si_GG]);
         si->GetACQParam("PG")->SetData((UShort_t)canal[Si_PG]);
         si->GetACQParam("T")->SetData((UShort_t)canal[Si_T]);
         identifying_telescope = gIndra->GetIDTelescope(Form("CI_SI_%02d%02d", ring, mod));
         return si;
      }
      else
         return chio;
   }
   else if (ring >= 10) {
      //chio-csi or chio-si75
      KVCsI* csi = (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (!csi) return 0;
      KVChIo* chio = (KVChIo*)csi->GetChIo();
      if (!chio) return 0;
      KVSi75* si75 = (KVSi75*)gIndra->GetDetectorByType(ring, mod, Si75_GG);
      if (!si75) return 0;
      chio->SetEnergy(de1);
      chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
      chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
      chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
      if (canal[CsI_R] > 0.) {
         csi->SetEnergy(de3);
         csi->GetACQParam("R")->SetData((UShort_t)canal[CsI_R]);
         csi->GetACQParam("L")->SetData((UShort_t)canal[CsI_L]);
         csi->GetACQParam("T")->SetData((UShort_t)mt);
         identifying_telescope = gIndra->GetIDTelescope(Form("CI_CSI_%02d%02d", ring, mod));
         return csi;
      }
      else if (de4 > 0.0) {
         si75->SetEnergy(de4);
         si75->GetACQParam("GG")->SetData((UShort_t)canal[Si75_GG]);
         si75->GetACQParam("PG")->SetData((UShort_t)canal[Si75_PG]);
         si75->GetACQParam("T")->SetData((UShort_t)canal[Si75_T]);
         identifying_telescope = gIndra->GetIDTelescope(Form("CI_SI75_%02d%02d", ring, mod));
         return si75;
      }
      else
         return chio;
   }
   return 0;
}
//_____________________________________________________________________________________________//
KVDetector* KVINDRADstToRootTransfert::Code7(Int_t ring, Int_t mod)
{
//    cout << "Code7" << endl;

//code = 7 : frag. cree par la coherence ChIo (stoppe ds ChIo)

   KVChIo* chio = (KVChIo*)gIndra->GetDetectorByType(ring, mod, ChIo_GG);
   if (!chio) {
      //if chio not found, may be because particule has (ring,mod) corresponding
      //to a Si/CsI, and not those of a ChIo module
      //try CsI with given ring number and module
      KVCsI* csi = (KVCsI*)gIndra->GetDetectorByType(ring, mod, CsI_R);
      if (csi) chio = (KVChIo*)csi->GetChIo();
      if (!chio) return 0;
   }
   chio->SetEnergy(de1);
   chio->GetACQParam("GG")->SetData((UShort_t)canal[ChIo_GG]);
   chio->GetACQParam("PG")->SetData((UShort_t)canal[ChIo_PG]);
   chio->GetACQParam("T")->SetData((UShort_t)canal[ChIo_T]);
   return chio;
}
//_____________________________________________________________________________________________//
void KVINDRADstToRootTransfert::lire_evt(ifstream& f_in, KVINDRAReconEvent* evt)
{
   //
   //Lire un evenement
   //

   FNMETHOD CodeFunc[] = {
      &KVINDRADstToRootTransfert::Code0,            //code0
      &KVINDRADstToRootTransfert::Code1,            //code1
      &KVINDRADstToRootTransfert::Code2and9and10,    //code2
      &KVINDRADstToRootTransfert::Code3,            //code3
      &KVINDRADstToRootTransfert::Code4and5and6and8,//code4
      &KVINDRADstToRootTransfert::Code4and5and6and8,//code5
      &KVINDRADstToRootTransfert::Code4and5and6and8,//code6
      &KVINDRADstToRootTransfert::Code7,            //code7
      &KVINDRADstToRootTransfert::Code4and5and6and8,//code8
      &KVINDRADstToRootTransfert::Code2and9and10,    //code9
      &KVINDRADstToRootTransfert::Code2and9and10    //code10
   };

   TString buff;
   Int_t mul, num_ev;

   buff.ReadLine(f_in);
   num_ev = buff.Atoi();

   if (f_in.good()) {
      events_in_file++; //total number events in each file
      events_read++;    //total events read
      events_good++;

      // Instead of using the DST event number (which may not have a continuous sequence, e.g. 1,3,4,7,12,...)
      // we use the sequential event counter fEventNumber (=1,2,3,...) which is also used as the event number
      // of the raw data tree which we are filling with one entry for each DST event using the informations
      // on raw data (canal[..]) written in the DST files. This is to ensure correct synchronisation
      //evt->SetNumber(num_ev);
      evt->SetNumber(fEventNumber);

      f_in >> mul;
      for (Int_t i = 0; i < mul; i++) {

         Int_t z, a, code, icou, imod, ecode, ncans;
         Double_t z_indra, a_indra, ener;
         for (Int_t gj = 0; gj < 16; gj++) canal[gj] = 0.0;
         f_in >> a;
         f_in >> z;
         f_in >> z_indra;
         f_in >> a_indra;
//Check energy is not NAN
//       f_in >> ener;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) {
            ener = buff.Atof();
         }
         else {
            ener = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for ener. Set E=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }
         f_in >> icou;
         f_in >> imod;
         f_in >> code;
         f_in >> ecode;

//       f_in >> de_mylar;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) de_mylar = buff.Atof();
         else {
            de_mylar = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for de_mylar. Set de_mylar=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }
         de_mylar *= 1.0; //just to avoid "variable set but not used" warning

//       f_in >> de1;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) de1 = buff.Atof();
         else {
            de1 = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for de1. Set de1=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }

//       f_in >> de2;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) de2 = buff.Atof();
         else {
            de2 = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for de2. Set de2=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }

//       f_in >> de3;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) de3 = buff.Atof();
         else {
            de3 = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for de3. Set de3=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }

//       f_in >> de4;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) de4 = buff.Atof();
         else {
            de4 = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for de4. Set de4=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }

//       f_in >> de5;
         buff.ReadLine(f_in);
         if (buff.IsFloat()) de5 = buff.Atof();
         else {
            de5 = 0.;
            cout << "BAD:  event#" << num_ev << " z=" << z << " a=" << a << " / Non-numeric value read for de5. Set de5=0" << endl;
            cout << "(read: " << buff.Data() << ")" << endl;
         }

         f_in >> mt;

         if (!icou) {
            cout << "BAD:  event#" << num_ev << "  particle#" << i << "  Z=" << z << "  code=" << code << endl;
         }

         Int_t idf;
         for (Int_t mk = 1; mk <= 4; mk++) {
            f_in >> idf;
            code_idf[mk] = idf;
         }

         f_in >> ncans;
         if (ncans) {
            Int_t kk;
            Double_t chan;
            for (Int_t mk = 1; mk <= ncans; mk++) {
               f_in >> kk >> chan;
               canal[kk] = chan;
            }
         }

         if ((code < 11 || (camp2 && code == 14)) && icou > 0 && imod > 0 && icou < 18 && imod < 25) {
            //work out where particle stopped
            //set up corresponding detectors with energy losses and raw data (canal)
            //set stopping detector's marqueur de temps to the MT of the particle
            //(the other Mt are not written on the DST's).
            //then use KVINDRAReconNuc::Reconstruct to set up list of hit detectors,
            //hit group and telescope for this particle
            //
            //pour la 2e campagne presence de code14 qui semble etre
            //des codes 4 donc on les traite comme tels

            KVDetector* det_stop = 0;
            identifying_telescope = 0;
            if (code == 14)  det_stop = (this->*CodeFunc[4])(icou, imod);
            else           det_stop = (this->*CodeFunc[code])(icou, imod);

            if (!det_stop) {
               cout << "det_stop=0: event#" << num_ev << " particle#" << i << "  icou=" << icou << " imod=" << imod <<
                    " z=" << z << " a=" << a << " code=" << code << " ecode=" << ecode << endl;
            }
            else {
               KVINDRAReconNuc* tmp = evt->AddParticle();
               if (!tmp) {
                  Fatal("lire_evt", "KVINDRAReconEvent::AddParticle() returns NULL pointer.");
                  gSystem->Exit(1);
               }
               det_stop->GetACQParam("T")->SetData((UShort_t)mt);
               tmp->Reconstruct(det_stop);
               if (identifying_telescope) {
                  tmp->SetIdentifyingTelescope(identifying_telescope);
                  tmp->SetIsIdentified();
                  tmp->SetZMeasured(kTRUE);
               }
               if (ecode > 0)
                  tmp->SetIsCalibrated();
               //modify ID code for phoswich particles 2nd campaign
               //we put VEDA ID code=2 just like for 1st campaign
               //if(camp2 && icou==1 && code==4) code=2;
               tmp->SetIDCode(KVINDRACodes::VedaIDCodeToBitmask(code));
               tmp->SetECode(KVINDRACodes::VedaECodeToBitmask(ecode));
               tmp->SetZ(z);
               tmp->SetRealZ(z_indra);
               tmp->SetA(TMath::Abs(a));
               if (a > 0 && (code == 2 || code == 3)) {
                  tmp->SetAMeasured(kTRUE);
                  tmp->SetRealA(a_indra);
               }
               else {
                  tmp->SetAMeasured(kFALSE);
                  // reset Z in order to force calculation of mass according to default mass formula of KVINDRAReconNuc
                  tmp->SetZ(z);
               }
               if (code == 1) {
                  //set Z and realZ = 0 in case of 'neutron'
                  tmp->SetZ(0);
                  tmp->SetRealZ(0);
                  tmp->SetA(1);
                  tmp->SetRealA(0);
               }
               if (code == 5) {
                  tmp->SetZMeasured(kFALSE);
               }
               tmp->SetEnergy(ener);

               tmp->SetEnergyChIo(de1 + de_mylar);
               tmp->SetEnergySi(de2);
               tmp->SetEnergyCsI(de3);
               tmp->SetEnergySi75(de4);
               tmp->SetEnergySiLi(de5);


            }//if(det_stop)
         }//if code<11
      }//for(int i=0;

      //write event to tree
      FillRawTree();
      data_tree->Fill();
      fEventNumber++;
   }//if(f_in.good

}

KVNumberList KVINDRADstToRootTransfert::PrintAvailableRuns(KVString& datatype)
{
   //Prints list of available runs, sorted according to multiplicity
   //trigger, for selected dataset, data type/analysis task, and system
   //Returns list containing all run numbers

   KVNumberList all_runs =
      GetDataSet()->GetRunList(datatype.Data(), GetSystem());
   KVINDRADBRun* dbrun;

   //first read list and find what triggers are available
   vector<int> triggers;
   all_runs.Begin();
   while (!all_runs.End()) {
      dbrun = (KVINDRADBRun*)GetDataSet()->GetDataBase()->GetDBRun(all_runs.Next());
      if (triggers.size() == 0
            || std::find(triggers.begin(), triggers.end(), dbrun->GetTrigger()) != triggers.end()) {
         triggers.push_back(dbrun->GetTrigger());
      }
   }
   //sort triggers in ascending order
   std::sort(triggers.begin(), triggers.end());

   for (std::vector<int>::iterator it = triggers.begin(); it != triggers.end(); ++it) {
      cout << " ---> Trigger M>" << *it << endl;
      all_runs.Begin();
      while (!all_runs.End()) {
         dbrun = (KVINDRADBRun*)GetDataSet()->GetDataBase()->GetDBRun(all_runs.Next());
         if (dbrun->GetTrigger() == *it) {
            cout << "    " << Form("%4d", dbrun->GetNumber());
            cout << Form("\t(%9llu events)", dbrun->GetEvents());
            cout << "\t[File written: " << dbrun->GetDatime().
                 AsString() << "]";
            if (dbrun->GetComments())
               cout << "\t" << dbrun->GetComments();
            cout << endl;
         }
      }
      cout << endl;
   }
   return all_runs;
}

void KVINDRADstToRootTransfert::FillRawTree()
{
   // Put values of all "fired" acquisition parameters into the arrays ParNum/ParVal
   // and store them in the raw data tree

   NbParFired = 0;
   TIter next(params);
   KVACQParam* par = 0;
   while ((par = (KVACQParam*)next())) {
      if (par->Fired()) {
         ParVal[NbParFired] = par->GetCoderData();
         ParNum[NbParFired++] = par->GetNumber();
      }
   }
   rawtree->Fill();
}
