/*
$Id: KVINDRARawDataReconstructor.cpp,v 1.7 2008/10/17 10:58:07 franklan Exp $
$Revision: 1.7 $
$Date: 2008/10/17 10:58:07 $
*/

//Created by KVClassFactory on Fri May  4 17:24:08 2007
//Author: franklan

#include "KVINDRARawDataReconstructor.h"
#include "KVDataAnalyser.h"
#include "KVINDRA.h"
#include "KVDataSet.h"
#include "KVINDRADB.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "RVersion.h"

using namespace std;

ClassImp(KVINDRARawDataReconstructor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRARawDataReconstructor</h2>
<h4>Reconstruction of events from raw data acquisition files</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVINDRARawDataReconstructor::KVINDRARawDataReconstructor()
   : taskname("Reconstruction"), datatype("recon")
{
   //Default constructor
   file = 0;
   tree = genetree = 0;
   recev = 0;
   nb_recon = 0;
}

KVINDRARawDataReconstructor::~KVINDRARawDataReconstructor()
{
   //Destructor
   SafeDelete(recev);
}

void KVINDRARawDataReconstructor::InitAnalysis()
{

}

//______________________________________________________________________________________//

void KVINDRARawDataReconstructor::InitRun()
{
   // Creates new ROOT file with TTree for reconstructed/calibrated events.
   // By default this file will be written in the same data repository as the raw data file we are reading.
   // This can be changed by setting the environment variable(s):
   //
   //     Reconstruction.DataAnalysisTask.OutputRepository:     [name of repository]
   //     [name of dataset].Reconstruction.DataAnalysisTask.OutputRepository:         [name of repository]
   //
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used. If neither is defined, the new file will be written in the same repository as
   // the raw file (if possible, i.e. if repository is not remote).

   // Create new KVINDRAReconEvent used to reconstruct & store events
   // The condition used to seed new reconstructed particles (see KVReconstructedEvent::AnalyseTelescopes)
   // is set by reading the value of the environment variables:
   //     Reconstruction.DataAnalysisTask.ParticleSeedCond:        [all/any]
   //     [name of dataset].Reconstruction.DataAnalysisTask.ParticleSeedCond:     [all/any]
   // If no value is set for the current dataset (second variable), the value of the
   // first variable will be used.

   if (!recev) recev = new KVINDRAReconEvent;
   recev->SetPartSeedCond(gDataSet->GetDataSetEnv("Reconstruction.DataAnalysisTask.ParticleSeedCond"));

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(
         gDataSet->GetDataSetEnv(Form("%s.DataAnalysisTask.OutputRepository", taskname.Data()),
                                 gDataRepository->GetName()),
         gDataSet->GetName());

   file = OutputDataset->NewRunfile(datatype.Data(), fRunNumber);

   cout << "Writing \"" << datatype.Data() << "\" events in ROOT file " << file->GetName() << endl;

   //tree for raw data
   rawtree = new TTree("RawData", Form("%s : %s : raw data",
                                       gIndraDB->GetRun(fRunNumber)->GetName(), gIndraDB->GetRun(fRunNumber)->GetTitle()));
   rawtree->Branch("RunNumber", &fRunNumber, "RunNumber/I");
   rawtree->Branch("EventNumber", &fEventNumber, "EventNumber/I");

   // the format of the raw data tree must be "arrays" : we depend on it in KVINDRAReconDataAnalyser
   // in order to read the raw data and set the detector acquisition parameters
   TString raw_opt = "arrays";
   GetRawDataReader()->SetUserTree(rawtree, raw_opt.Data());
   Info("InitRun", "Created raw data tree (%s : %s). Format: %s",
        rawtree->GetName(), rawtree->GetTitle(), raw_opt.Data());

#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
#if ROOT_VERSION_CODE < ROOT_VERSION(5,26,1)
   // The TTree::OptimizeBaskets mechanism is disabled, as for ROOT versions < 5.26/00b
   // this lead to a memory leak
   rawtree->SetAutoFlush(0);
#endif
#endif


   //tree for reconstructed events
   tree = new TTree("ReconstructedEvents", Form("%s : %s : %s events created from raw data",
                    gIndraDB->GetRun(fRunNumber)->GetName(),
                    gIndraDB->GetRun(fRunNumber)->GetTitle(),
                    datatype.Data())
                   );
#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
#if ROOT_VERSION_CODE < ROOT_VERSION(5,26,1)
   // The TTree::OptimizeBaskets mechanism is disabled, as for ROOT versions < 5.26/00b
   // this lead to a memory leak
   tree->SetAutoFlush(0);
#endif
#endif


   //leaves for reconstructed events
   KVEvent::MakeEventBranch(tree, "INDRAReconEvent", "KVINDRAReconEvent", &recev);

   Info("InitRun", "Created reconstructed data tree %s : %s", tree->GetName(), tree->GetTitle());

   //tree for gene data
   genetree = new TTree("GeneData", Form("%s : %s : gene data",
                                         gIndraDB->GetRun(fRunNumber)->GetName(), gIndraDB->GetRun(fRunNumber)->GetTitle()));

   //we add to the 'gene tree' a branch for every acquisition parameter of the detector
   genetree->Branch("RunNumber", &fRunNumber, "RunNumber/I");
   genetree->Branch("EventNumber", &fEventNumber, "EventNumber/I");
   KVACQParam* acqpar;
   TIter next_acqpar(gIndra->GetACQParams());
   while ((acqpar = (KVACQParam*)next_acqpar())) {
      genetree->Branch(acqpar->GetName(), *(acqpar->ConnectData()), Form("%s/S", acqpar->GetName()));
   }
#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
#if ROOT_VERSION_CODE < ROOT_VERSION(5,26,1)
   // The TTree::OptimizeBaskets mechanism is disabled, as for ROOT versions < 5.26/00b
   // this lead to a memory leak
   genetree->SetAutoFlush(0);
#endif
#endif


   Info("InitRun", "Created pulser/laser data tree (%s : %s) for %d parameters",
        genetree->GetName(), genetree->GetTitle(), genetree->GetNbranches());
   //initialise number of reconstructed events
   nb_recon = 0;

}

//______________________________________________________________________________________//

Bool_t KVINDRARawDataReconstructor::Analysis()
{
   // Analysis of event.
   // RawData TTree is filled with values of parameters for event.
   // If event has INDRA trigger information (IsIndraEvent()==kTRUE)
   // then
   //    *) event reconstruction is performed for 'Physics' events
   //    *) or the GeneTree is filled with pulser/laser data for 'Gene' events

   recev->SetNumber(GetEventNumber());

   if (gIndra->GetTriggerInfo()->IsINDRAEvent()) {
      if (gIndra->GetTriggerInfo()->IsPhysics()) {
         gIndra->ReconstructEvent(recev, (KVDetectorEvent*)GetDetectorEvent());
         nb_recon++;
         ExtraProcessing();
      } else {
         genetree->Fill();
      }
   }
   // ReconstructedEvents tree must be filled for every event, even ones where
   // no event has been reconstructed. This is so that when reading back we can make
   // RawData a 'friend' TTree and keep everything in synch.
   tree->Fill();
   recev->Clear();

   return kTRUE;
}

//______________________________________________________________________________________//

void KVINDRARawDataReconstructor::EndRun()
{
   SafeDelete(recev);

   cout << endl << " *** Number of reconstructed INDRA events : "
        << nb_recon << " ***" << endl << endl;
   file->cd();
   gDataAnalyser->WriteBatchInfo(tree);
   tree->Write();//write tree to file
   rawtree->Write();
   genetree->Write();

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(
         gDataSet->GetDataSetEnv(Form("%s.DataAnalysisTask.OutputRepository", taskname.Data()),
                                 gDataRepository->GetName()),
         gDataSet->GetName());
   //add new file to repository
   OutputDataset->CommitRunfile(datatype.Data(), fRunNumber, file);
}
