#define KVINDRAReconIdent_cxx

#include "KVINDRAReconIdent.h"
#include "TFile.h"
#include "TTree.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "KVDataRepositoryManager.h"
#include "KVDataAnalyser.h"
#include "KVINDRADB.h"

ClassImp(KVINDRAReconIdent)

///////////////////////////////////////////////////////////
// Analysis class used to identify previously
// reconstructed events and write the new data files in the data repository
//

void KVINDRAReconIdent::InitAnalysis(void)
{
}

//_____________________________________
void KVINDRAReconIdent::InitRun(void)
{
   //When each run is opened, we create a new ROOT file for the identified events we
   //are going to generate from the reconstructed events we are reading.
   // By default this file will be written in the same data repository as the recon data file we are reading.
   // This can be changed by setting the environment variable(s):
   //
   //     ReconIdent.DataAnalysisTask.OutputRepository:     [name of repository]
   //     [name of dataset].ReconIdent.DataAnalysisTask.OutputRepository:         [name of repository]
   //
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used. If neither is defined, the new file will be written in the same repository as
   // the recon file (if possible, i.e. if repository is not remote).

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(
         gDataSet->GetDataSetEnv("ReconIdent.DataAnalysisTask.OutputRepository", gDataRepository->GetName()),
         gDataSet->GetName());

   //create new ROOT file for identified events
   fRunNumber = gIndra->GetCurrentRunNumber();
   fIdentFile = OutputDataset->NewRunfile("ident", fRunNumber);


   fIdentTree = new TTree("ReconstructedEvents", Form("%s : %s : ident events created from recon data",
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
}

//_____________________________________
Bool_t KVINDRAReconIdent::Analysis(void)
{
   //For each event we:
   //     perform primary event identification and calibration and fill tree

   fEventNumber = GetEvent()->GetNumber();
   if (GetEvent()->GetMult() > 0) {
      GetEvent()->IdentifyEvent();
      GetEvent()->CalibrateEvent();
   }
   fIdentTree->Fill();
   return kTRUE;
}

//_____________________________________
void KVINDRAReconIdent::EndRun(void)
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
         gDataSet->GetDataSetEnv("ReconIdent.DataAnalysisTask.OutputRepository", gDataRepository->GetName()),
         gDataSet->GetName());

   OutputDataset->CommitRunfile("ident", gIndra->GetCurrentRunNumber(),
                                fIdentFile);
   fIdentFile = 0;
   fIdentTree = 0;
}

//_____________________________________
void KVINDRAReconIdent::EndAnalysis(void)
{
}
