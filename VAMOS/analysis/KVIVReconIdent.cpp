#include "KVIVReconIdent.h"
#include "KVIVReconEvent.h"
#include "KVVAMOS.h"
#include "TFile.h"
#include "TTree.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "KVDataRepositoryManager.h"
#include "KVDataAnalyser.h"
#include "KVINDRADB.h"
#include "Riostream.h"

using namespace std;

ClassImp(KVIVReconIdent)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIVReconIdent</h2>
<h4>INDRA identification and calibration, VAMOS trajectory reconstruction,
calibration and identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVIVReconIdent::InitAnalysis(void)
{

   TString dt;
   dt = gDataSet->GetDataSetEnv("KVIVReconIdent.DataToAnalyse", "INDRA VAMOS");
   dt.ToLower();
   Info("InitAnalysis", "Analysing data of %s", dt.Data());
   if (dt.Contains("indra")) SetBit(kINDRAdata);
   if (dt.Contains("vamos")) SetBit(kVAMOSdata);

   if (TestBit(kINDRAdata)) cout << "INDRA" << endl;
   if (TestBit(kVAMOSdata)) cout << "VAMOS" << endl;
}
//_____________________________________

void KVIVReconIdent::InitRun(void)
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
      gDataRepositoryManager->GetDataSet(gDataSet->GetOutputRepository("ReconIdent"), gDataSet->GetName());

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
   TBranch* recon_br = (TBranch*)fChain->GetListOfBranches()->First();
   KVEvent::MakeEventBranch(fIdentTree, recon_br->GetName(), recon_br->GetClassName(), GetEventReference());

   // set flag if this branch contains a KVIVReconEvent object
   fIsIVevent = TClass::GetClass(recon_br->GetClassName())->InheritsFrom("KVIVReconEvent");

   Info("InitRun", "Created identified/calibrated data tree %s : %s", fIdentTree->GetName(), fIdentTree->GetTitle());

   if (TestBit(kINDRAdata)) {
      // initialise INDRA identifications
      gIndra->InitializeIDTelescopes();

      cout << endl << setw(20) << "" << "----------------------" << endl;
      cout         << setw(20) << "" << "|  STATUS FOR INDRA  |" << endl;
      cout         << setw(20) << "" << "----------------------" << endl << endl;
      // print status of identifications
      gIndra->PrintStatusOfIDTelescopes();
      // print status of calibrations
      gIndra->PrintCalibStatusOfDetectors();
   }


   if (TestBit(kVAMOSdata)) {
      // initialise VAMOS identifications
      gVamos->InitializeIDTelescopes();

      cout << endl << setw(20) << "" << "----------------------" << endl;
      cout         << setw(20) << "" << "|  STATUS FOR VAMOS  |" << endl;
      cout         << setw(20) << "" << "----------------------" << endl << endl;
      // print status of identifications
      gVamos->PrintStatusOfIDTelescopes();
      // print status of calibrations
      gVamos->PrintCalibStatusOfDetectors();

      // look for VAMOS data correction class if defined in env
      fDataCorr = NULL;
      fDataCorr = GetDataCorrection();
   }
}
//_____________________________________

Bool_t KVIVReconIdent::Analysis(void)
{
   //For each event we:
   //     perform primary event identification and calibration and fill tree

   fEventNumber = GetEvent()->GetNumber();

   //Analyse INDRA event
   if (TestBit(kINDRAdata)) {
      if (GetEvent()->GetMult() > 0) {
         GetEvent()->IdentifyEvent();
         GetEvent()->CalibrateEvent();
      }
   }

   //Analyse VAMOS event
   if (TestBit(kVAMOSdata)) {
      if (fIsIVevent) { // condition set for backwards compatibility with
         // old recon ROOT files

         KVIVReconEvent* IVevent = (KVIVReconEvent*)GetEvent();
         // Z-identification, calibration, Q-A identification and corrections

         //debug
         Info("Analysis", "identification of a new event (mult_vamos=%d)", IVevent->GetVAMOSMult());

         IVevent->IdentAndCalibVAMOSEvent(fDataCorr);

         Info("Analysis", "... IdentAndCalibVAMOSEvent done ...");
      }
   }

   //Fill Ident tree
   fIdentTree->Fill();
   Info("Analysis", "... Fill tree done ...");

   return kTRUE;
}
//_____________________________________

KVVAMOSDataCorrection* KVIVReconIdent::GetDataCorrection()
{
   //Returns the KVVAMOSDataCorrection object used to apply corrections on VAMOS

   if (!fDataCorr) {
      fDataCorr = KVVAMOSDataCorrection::MakeDataCorrection(gDataSet->GetName());

      KVDBRun* db_run_entry(gIndraDB->GetRun(fRunNumber));
      assert(db_run_entry);

      Info("GetDataCorrection", "Setting VAMOS ID correction parameters...");

      const TString keyname("VAMOSIDCorrectionParameters");

      Info("GetDataCorrection", "Retrieving records using key \'%s\'...", keyname.Data());

      const KVRList* const records(db_run_entry->GetLinks(keyname));
      assert(records);

      if (!records) {
         Error("GetDataCorrection", "Failed to retrieve records using key \"%s\", "
               "has the key name changed?", keyname.Data());
      }

      else {
         fDataCorr->SetIDCorrectionParameters(records);
         fDataCorr->Init();
      }
   }

   return fDataCorr;
}
//_____________________________________

void KVIVReconIdent::EndAnalysis(void)
{
}
