#include "KVINDRAIdentRoot.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVINDRAReconNuc.h"
#include "KVINDRAReconDataAnalyser.h"
#include "KVINDRADB.h"
using namespace std;

ClassImp(KVINDRAIdentRoot)


////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAIdentRoot</h2>
<h4>Generation of fully-identified and calibrated INDRA data files</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//_____________________________________
void KVINDRAIdentRoot::InitAnalysis(void)
{
}

//_____________________________________
void KVINDRAIdentRoot::InitRun(void)
{
   //When each run is opened, we create a new ROOT file for the identified events we
   //are going to generate from the reconstructed events we are reading.
   // By default this file will be written in the same data repository as the recon data file we are reading.
   // This can be changed by setting the environment variable(s):
   //
   //     IdentRoot.DataAnalysisTask.OutputRepository:     [name of repository]
   //     [name of dataset].IdentRoot.DataAnalysisTask.OutputRepository:         [name of repository]
   //
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used. If neither is defined, the new file will be written in the same repository as
   // the recon file (if possible, i.e. if repository is not remote).

   /*
   CHECK that ident file was not generated with v1.8.0, 1.8.1 or 1.8.2 (1.8.1a or >=1.8.3 is OK)
     if bzr branch = 1.8 must have revision >= 489
     if bzr branch = 1.8.1-bugfixes must have revision >= 444
   */
   TEnv* infos = ((KVINDRAReconDataAnalyser*)gDataAnalyser)->GetReconDataTreeInfos();
   if (infos) {
      TString branchname = infos->GetValue("KVBase::bzrBranchNick()", "");
      if (branchname == "") {
         Warning("InitRun", "IDENT file generated with UNKNOWN version of KaliVeda (no bzr branchname)");
      } else {
         Int_t revno = infos->GetValue("KVBase::bzrRevisionNumber()", 0);
         if (!revno) {
            Warning("InitRun", "IDENT file generated with UNKNOWN version of KaliVeda (no bzr revno)");
         } else {
            if ((branchname == "1.8" && revno < 489) || (branchname == "1.8.1-bugfixes" && revno < 444)) {
               Fatal("InitRun", "**** IDENT FILE GENERATED WITH WRONG VERSION!!! **** IDENT FILE GENERATED WITH WRONG VERSION!!!\n\n\t\t\t====>> Regenerate with KaliVeda v1.8.3+ <====");
               exit(1);
            }
         }
      }
   }
   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(
         gDataSet->GetDataSetEnv("IdentRoot.DataAnalysisTask.OutputRepository", gDataRepository->GetName()),
         gDataSet->GetName());

   //create new ROOT file for identified events
   fRunNumber = gIndra->GetCurrentRunNumber();
   fIdentFile = OutputDataset->NewRunfile("root", fRunNumber);


   fIdentTree = new TTree("ReconstructedEvents", Form("%s : %s : fully-identified & calibrated events created from ident data",
                          gIndraDB->GetRun(fRunNumber)->GetName(),
                          gIndraDB->GetRun(fRunNumber)->GetTitle())
                         );
#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
#if ROOT_VERSION_CODE < ROOT_VERSION(5,26,1)
   // The TTree::OptimizeBaskets mechanism is disabled, as for ROOT versions < 5.26/00b
   // this lead to a memory leak
   fIdentTree->SetAutoFlush(0);
#endif
#endif
   //leaves for reconstructed events
   KVEvent::MakeEventBranch(fIdentTree, "INDRAReconEvent", "KVINDRAReconEvent", GetEventReference());

   Info("InitRun", "Created identified/calibrated data tree %s : %s", fIdentTree->GetName(), fIdentTree->GetTitle());

   // initialise identifications
   gIndra->InitializeIDTelescopes();

   // print status of identifications
   gIndra->PrintStatusOfIDTelescopes();
   // print status of calibrations
   gIndra->PrintCalibStatusOfDetectors();

   for (register int i = 0; i < 15; i++) Acodes[i] = 0;
   for (register int i = 0; i < 4; i++) Astatus[i] = 0;
}

//_____________________________________
Bool_t KVINDRAIdentRoot::Analysis(void)
{
   //For each event we:
   //     perform secondary event identification and calibration and fill tree
   fEventNumber = GetEvent()->GetNumber();
   CountStatus();
   if (GetEvent()->GetMult() > 0) {
      GetEvent()->SecondaryIdentCalib();
      CountCodes();
   }
   fIdentTree->Fill();
   return kTRUE;
}

//_____________________________________
void KVINDRAIdentRoot::EndRun(void)
{
   //At the end of each run we:
   //      write the tree into the new file
   //      close the file
   //      copy the file into the required repository (see InitRun)
   //      update the available runlist

   fIdentFile->cd();

   gDataAnalyser->WriteBatchInfo(fIdentTree);

   GetRawData()->CloneTree(-1, "fast"); //copy raw data tree to file
   GetGeneData()->CloneTree(-1, "fast"); //copy pulser & laser (gene) tree to file

   fIdentFile->Write();

   //add file to repository
   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(
         gDataSet->GetDataSetEnv("IndraRoot.DataAnalysisTask.OutputRepository", gDataRepository->GetName()),
         gDataSet->GetName());

   OutputDataset->CommitRunfile("root", gIndra->GetCurrentRunNumber(),
                                fIdentFile);
   fIdentFile = 0;
   fIdentTree = 0;

   cout << endl << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
   cout << endl << "         BILAN  DES  COURSES " << endl << endl;;
   int ntot = 0;
   for (register int i = 0; i < 4; i++)ntot += Astatus[i];

   for (register int i = 0; i < 4; i++) {
      cout << "   Status" << i << "\t" << Astatus[i] << "\t" << setprecision(2) << 100.*Astatus[i] / (1.*ntot) << " %" << endl;
   }
   cout << endl << "  Total all status : " << ntot << endl << endl;
   ntot = 0;
   for (register int i = 0; i < 15; i++)ntot += Acodes[i];
   for (register int i = 0; i < 15; i++) {
      cout << "   Code" << i << "\t" << Acodes[i] << "\t" << setprecision(2) << 100.*Acodes[i] / (1.*ntot) << " %" << endl;
   }
   cout << endl << "  Total all codes : " << ntot << endl;
   cout << endl << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
}

//_____________________________________
void KVINDRAIdentRoot::EndAnalysis(void)
{
}
void KVINDRAIdentRoot::CountCodes()
{
   KVINDRAReconNuc* particle;
   for (register int i = 0; i < 15; i++) codes[i] = 0;
   while ((particle = GetEvent()->GetNextParticle())) {
      int code = particle->GetCodes().GetVedaIDCode();
      if ((code == 0 && particle->IsIdentified()) || (code > 0 && code < 15)) codes[code] += 1;
   }
   int ntot = 0;
   for (register int i = 0; i < 15; i++) {
      ntot += codes[i];
      Acodes[i] += codes[i];
   }
}
void KVINDRAIdentRoot::CountStatus()
{
   KVINDRAReconNuc* particle;
   for (register int i = 0; i < 4; i++) status[i] = 0;
   while ((particle = GetEvent()->GetNextParticle())) {
      status[particle->GetStatus()] += 1;
   }
   int ntot = 0;
   for (register int i = 0; i < 4; i++) {
      ntot += status[i];
      Astatus[i] += status[i];
   }
}

