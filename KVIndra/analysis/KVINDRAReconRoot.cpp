#define KVINDRAReconRoot_cxx

#include "KVINDRAReconRoot.h"
#include "TFile.h"
#include "TTree.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "KVDataRepositoryManager.h"
#include "KVDataAnalyser.h"
#include "KVINDRAReconNuc.h"
#include "KVINDRADB.h"

using namespace std;

ClassImp(KVINDRAReconRoot)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAReconRoot</h2>
<h4>Generation of fully-identified and calibrated INDRA data files</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVINDRAReconRoot::InitAnalysis(void)
{
}

//_____________________________________
void KVINDRAReconRoot::InitRun(void)
{
   //When each run is opened, we create a new ROOT file for the identified events we
   //are going to generate from the reconstructed events we are reading.
   // By default this file will be written in the same data repository as the recon data file we are reading.
   // This can be changed by setting the environment variable(s):
   //
   //     ReconRoot.DataAnalysisTask.OutputRepository:     [name of repository]
   //     [name of dataset].ReconRoot.DataAnalysisTask.OutputRepository:         [name of repository]
   //
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used. If neither is defined, the new file will be written in the same repository as
   // the recon file (if possible, i.e. if repository is not remote).

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(
         gDataSet->GetDataSetEnv("ReconRoot.DataAnalysisTask.OutputRepository", gDataRepository->GetName()),
         gDataSet->GetName());

   //create new ROOT file for identified events
   fRunNumber = gIndra->GetCurrentRunNumber();
   fIdentFile = OutputDataset->NewRunfile("root", fRunNumber);


   fIdentTree = new TTree("ReconstructedEvents", Form("%s : %s : fully-identified & calibrated events created from recon data",
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
Bool_t KVINDRAReconRoot::Analysis(void)
{
   //For each event we:
   //     perform primary & secondary event identification and calibration and fill tree
   fEventNumber = GetEvent()->GetNumber();
   CountStatus();
   if (GetEvent()->GetMult() > 0) {
      GetEvent()->IdentifyEvent();
      GetEvent()->CalibrateEvent();
      GetEvent()->SecondaryIdentCalib();
      CountCodes();
   }
   fIdentTree->Fill();
   return kTRUE;
}
void KVINDRAReconRoot::CountCodes()
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
void KVINDRAReconRoot::CountStatus()
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

//_____________________________________
void KVINDRAReconRoot::EndRun(void)
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
         gDataSet->GetDataSetEnv("ReconRoot.DataAnalysisTask.OutputRepository", gDataRepository->GetName()),
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
void KVINDRAReconRoot::EndAnalysis(void)
{
}
