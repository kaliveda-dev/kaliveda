//Created by KVClassFactory on Mon Jul 16 15:20:21 2018
//Author: eindra

#include "KVRawDataReconstructor.h"
#include "KVDataSet.h"
#include "KVDataRepositoryManager.h"

ClassImp(KVRawDataReconstructor)

KVRawDataReconstructor::KVRawDataReconstructor()
   : KVRawDataAnalyser()
{
   // Default constructor
   Info("KVRawDataReconstructor", "Constructed");
}

//____________________________________________________________________________//

KVRawDataReconstructor::~KVRawDataReconstructor()
{
   // Destructor
}

void KVRawDataReconstructor::InitAnalysis()
{
   TClass* recev_cl = TClass::GetClass(GetDataSet()->GetReconstructedEventClassName());
   fRecev = ((KVReconstructedEvent*)recev_cl->New());

   Info("InitAnalysis", "Reconstructed event container class: %s", recev_cl->GetName());
}

void KVRawDataReconstructor::InitRun()
{
   fEvRecon.reset(new KVEventReconstructor(gMultiDetArray, fRecev));

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(GetDataSet()->GetOutputRepository("Reconstruction"), GetDataSet()->GetName());

   fRecFile = OutputDataset->NewRunfile("recon", fRunNumber);

   std::cout << "Writing \"recon\" events in ROOT file " << fRecFile->GetName() << std::endl;

   //tree for reconstructed events
   fRecTree = new TTree("ReconEvents", Form("%s : %s : %s",
                        gMultiDetArray->GetName(),
                        gExpDB->GetDBRun(fRunNumber)->GetTitle(),
                        gExpDB->GetDBRun(fRunNumber)->GetName())
                       );

   //leaves for reconstructed events
   KVEvent::MakeEventBranch(fRecTree, "ReconEvent", fRecev);

   Info("InitRun", "Created reconstructed data tree %s : %s", fRecTree->GetName(), fRecTree->GetTitle());
}

Bool_t KVRawDataReconstructor::Analysis()
{
   if (gMultiDetArray->HandledRawData()) {
      fEvRecon->ReconstructEvent(gMultiDetArray->GetFiredDetectors());
      fEvRecon->GetEvent()->SetNumber(GetEventNumber());
      fRecTree->Fill();
      //if(fEvRecon->GetEvent()->GetMult()) fEvRecon->GetEvent()->ls();
      fEvRecon->GetEvent()->Clear();
   }

   return kTRUE;
}

void KVRawDataReconstructor::EndRun()
{
   Info("KVRawDataReconstructor", "EndRun");
   fRecev->Clear();
   fRecFile->cd();
   WriteBatchInfo(fRecTree);
   fRecTree->Write();

   // get dataset to which we must associate new run
   KVDataSet* OutputDataset =
      gDataRepositoryManager->GetDataSet(GetDataSet()->GetOutputRepository("Reconstruction"), GetDataSet()->GetName());
   //add new file to repository
   OutputDataset->CommitRunfile("recon", fRunNumber, fRecFile);
}

void KVRawDataReconstructor::EndAnalysis()
{
   Info("KVRawDataReconstructor", "EndAnalysis");
}

//____________________________________________________________________________//

