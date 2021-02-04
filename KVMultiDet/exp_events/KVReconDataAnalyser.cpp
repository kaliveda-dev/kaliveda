//Created by KVClassFactory on Thu Jul 19 2018
//Author: John Frankland

#include "KVBase.h"
#include "KVReconDataAnalyser.h"
#include "KVDBRun.h"
#include "KVMultiDetArray.h"
#include "KVDataAnalysisTask.h"
#include "KVDataSet.h"
#include "TChain.h"
#include "TObjString.h"
#include "TChain.h"
#include "KVAvailableRunsFile.h"
#include "TProof.h"

#include <KVClassFactory.h>
#include <KVTriggerConditions.h>

using namespace std;

ClassImp(KVReconDataAnalyser)

KVReconDataAnalyser::KVReconDataAnalyser()
   : fSelector(nullptr), theChain(nullptr)
{
}

void KVReconDataAnalyser::Reset()
{
   //Reset task variables
   KVDataSetAnalyser::Reset();
   theChain = nullptr;
   fSelector = nullptr;
   TotalEntriesToRead = 0;
}

KVReconDataAnalyser::~KVReconDataAnalyser()
{
   //Destructor
   SafeDelete(fSelector);
}

//_________________________________________________________________

Bool_t KVReconDataAnalyser::CheckTaskVariables()
{
   // Checks the task variables

   if (!KVDataSetAnalyser::CheckTaskVariables()) return kFALSE;

   cout << "============> Analysis summary <=============" << endl;
   cout << "Analysis of runs " << GetRunList().GetList() << " with the class ";
   cout << "\"" << GetUserClass() << "\"." << endl;
   if (GetNbEventToRead()) {
      cout << GetNbEventToRead() << " events will be processed." << endl;
   }
   else {
      cout << "All events will be processed." << endl;
   }
   cout << "=============================================" << endl;

   return kTRUE;
}

//_________________________________________________________________

void KVReconDataAnalyser::SubmitTask()
{
   //Run the interactive analysis

   //make the chosen dataset the active dataset ( = gDataSet; note this also opens database
   //and positions gDataBase & gExpDB).
   GetDataSet()->cd();
   fSelector = nullptr;

   theChain = new TChain("ReconEvents");
   theChain->SetDirectory(0); // we handle delete

   GetRunList().Begin();
   Int_t run;

   // open and add to TChain all required files
   // we force the opening of the files to avoid problems with xrootd which sometimes
   // seems to have a little difficulty
   while (!GetRunList().End()) {
      run = GetRunList().Next();
      TString fullPathToRunfile = gDataSet->GetFullPathToRunfile(GetDataType(), run);
      cout << "Opening file " << fullPathToRunfile << endl;
      TFile* f = gDataSet->OpenRunfile<TFile>(GetDataType(), run);
      cout << "Adding file " << fullPathToRunfile;
      cout << " to the TChain." << endl;
      dynamic_cast<TChain*>(theChain)->Add(fullPathToRunfile);
      if (f && !f->IsZombie()) {
         // update run infos in available runs file if necessary
         KVAvailableRunsFile* ARF = gDataSet->GetAvailableRunsFile(GetDataType());
         if (ARF->InfosNeedUpdate(run, gSystem->BaseName(fullPathToRunfile))) {
            if (!((TTree*)f->Get("ReconEvents"))) {
               Error("SubmitTask", "No tree named ReconEvents is present in the current file");
               delete theChain;
               return;
            }
            TEnv* treeInfos = (TEnv*)((TTree*)f->Get("ReconEvents"))->GetUserInfo()->FindObject("TEnv");
            if (treeInfos) {
               TString kvversion = treeInfos->GetValue("KVBase::GetKVVersion()", "");
               TString username = treeInfos->GetValue("gSystem->GetUserInfo()->fUser", "");
               if (kvversion != "") ARF->UpdateInfos(run, gSystem->BaseName(fullPathToRunfile), kvversion, username);
            }
            else {
               Info("SubmitTask", "No TEnv object associated to the tree");
            }
         }
      }
   }
   TotalEntriesToRead = theChain->GetEntries();
   TString option = Form("EventsReadInterval=%lld,", GetAnalysisTask()->GetStatusUpdateInterval());
   option += Form("FullRunList=%s", GetFullRunList().GetList());

   // Add any user-defined options
   if (GetUserClassOptions() != "") {
      option += ",";
      option += GetUserClassOptions();
   }

   TObject* new_selector = GetInstanceOfUserClass();

   if (!new_selector || !new_selector->InheritsFrom("TSelector")) {
      cout << "The selector \"" << GetUserClass() << "\" is not valid." << endl;
      cout << "Process aborted." << endl;
      SafeDelete(new_selector);
   }
   else {
      SafeDelete(new_selector);
      Info("SubmitTask", "Beginning TChain::Process...");
#ifdef WITH_CPP11
      if (GetProofMode() != KVDataAnalyser::EProofMode::None) dynamic_cast<TChain*>(theChain)->SetProof(kTRUE);
#else
      if (GetProofMode() != KVDataAnalyser::None) dynamic_cast<TChain*>(theChain)->SetProof(kTRUE);
#endif
      TString analysis_class;
      if (GetAnalysisTask()->WithUserClass()) analysis_class.Form("%s%s", GetUserClassImp().Data(), GetACliCMode());
      else analysis_class = GetUserClass();

      if (GetNbEventToRead()) {
         theChain->Process(analysis_class, option.Data(), GetNbEventToRead());
      }
      else {
         theChain->Process(analysis_class, option.Data());
      }
   }
   delete theChain;
   fSelector = nullptr; //deleted by TChain/TTreePlayer
}

//__________________________________________________________________________________//

void KVReconDataAnalyser::WriteBatchEnvFile(const Char_t* jobname, Bool_t save)
{
   //Save (in the TEnv fBatchEnv) all necessary information on analysis task which can be used to execute it later
   //(i.e. when batch processing system executes the job).
   //If save=kTRUE (default), write the information in a file whose name is given by ".jobname"
   //where 'jobname' is the name of the job as given to the batch system.

   KVDataSetAnalyser::WriteBatchEnvFile(jobname, kFALSE);
   if (save) GetBatchInfoFile()->SaveLevel(kEnvUser);
}

void KVReconDataAnalyser::preInitAnalysis()
{
   // Called by currently-processed KVSelector before user's InitAnalysis() method.
   // We build the multidetector for the current dataset in case informations on
   // detector are needed e.g. to define histograms in InitAnalysis().
   // Note that at this stage we are not analysing a given run, so the parameters
   // of the array are not set (they will be set in preInitRun()).
   //
   // Note for PROOF: as this will be called both on master and on slave workers,
   // in order to reduce memory footprint we only build multidetector on the slaves

   if (!gProof || !gProof->IsMaster()) {
      if (!gMultiDetArray) KVMultiDetArray::MakeMultiDetector(GetDataSet()->GetName());
   }
}

void KVReconDataAnalyser::preInitRun()
{
   // Called by currently-processed TSelector when a new file in the TChain is opened.
   // We call gMultiDetArray->SetParameters for the current run.
   // Infos on currently read file/tree are printed.

   Int_t run = GetRunNumberFromFileName(theChain->GetCurrentFile()->GetName());
   gMultiDetArray->SetParameters(run);
   KVDBRun* CurrentRun = gExpDB->GetDBRun(run);
   fSelector->SetCurrentRun(CurrentRun);

   cout << endl << " ===================  New Run  =================== " <<
        endl << endl;

   CurrentRun->Print();
   if (CurrentRun->GetSystem()) {
      if (CurrentRun->GetSystem()->GetKinematics())
         CurrentRun->GetSystem()->GetKinematics()->Print();
   }

   cout << endl << " ================================================= " <<
        endl << endl;

   PrintTreeInfos();
   Info("preInitRun", "Data written with series %s, release %d", GetDataSeries().Data(),
        GetDataReleaseNumber());
}

void KVReconDataAnalyser::preAnalysis()
{
   // Set minimum (trigger) multiplicity for array

   gMultiDetArray->SetMinimumOKMultiplicity(fSelector->GetEvent());
}

TEnv* KVReconDataAnalyser::GetReconDataTreeInfos() const
{
   return (TEnv*)theChain->GetTree()->GetUserInfo()->FindObject("TEnv");
}

#ifdef USING_ROOT6
void KVReconDataAnalyser::SetTriggerConditionsForRun(int run)
{
   // When called from the InitRun() method of a user's analysis class, this method will ensure that only data
   // compatible with the experimental trigger will be provided for analysis in the user's Analysis() method.
   //
   // This will be done by searching for a KVTriggerConditions plugin class defined for the currently-analysed
   // dataset, defined like so:
   //
   //~~~~
   //+Plugin.KVTriggerConditions:   [dataset]   [classname]  [libname]   "[default constructor]()"
   //~~~~

   TPluginHandler* ph = KVBase::LoadPlugin("KVTriggerConditions", GetDataSet()->GetName());
   if (!ph) {
      Info("SetTriggerConditionsForRun",
           "No definition of trigger conditions available for dataset %s",
           GetDataSet()->GetName());
      return;
   }
   std::unique_ptr<KVTriggerConditions> trig((KVTriggerConditions*) ph->ExecPlugin(0));
   trig->SetTriggerConditionsForRun(fSelector, run);
}
#endif

void KVReconDataAnalyser::PrintTreeInfos()
{
   // Print informations on currently analysed TTree
   TEnv* treeInfos = GetReconDataTreeInfos();
   if (!treeInfos) return;
   cout << endl << "----------------------------------------------------------------------------------------------------" << endl;
   cout << "INFORMATIONS ON VERSION OF KALIVEDA USED TO GENERATE FILE:" << endl << endl;
   fDataVersion = treeInfos->GetValue("KVBase::GetKVVersion()", "(unknown)");
   cout << "version = " << fDataVersion << endl ;
   cout << "build date = " << treeInfos->GetValue("KVBase::GetKVBuildDate()", "(unknown)") << endl ;
   cout << "source directory = " << treeInfos->GetValue("KVBase::GetKVSourceDir()", "(unknown)") << endl ;
   cout << "KVROOT = " << treeInfos->GetValue("KVBase::GetKVRoot()", "(unknown)") << endl ;
   if (strcmp(treeInfos->GetValue("KVBase::bzrBranchNick()", "(unknown)"), "(unknown)")) {
      cout << "BZR branch name = " << treeInfos->GetValue("KVBase::bzrBranchNick()", "(unknown)") << endl ;
      cout << "BZR revision #" << treeInfos->GetValue("KVBase::bzrRevisionNumber()", "(unknown)") << endl ;
      cout << "BZR revision ID = " << treeInfos->GetValue("KVBase::bzrRevisionId()", "(unknown)") << endl ;
      cout << "BZR revision date = " << treeInfos->GetValue("KVBase::bzrRevisionDate()", "(unknown)") << endl ;
   }
   else if (strcmp(treeInfos->GetValue("KVBase::gitBranch()", "(unknown)"), "(unknown)")) {
      cout << "git branch = " << treeInfos->GetValue("KVBase::gitBranch()", "(unknown)") << endl;
      cout << "git commit = " << treeInfos->GetValue("KVBase::gitCommit()", "(unknown)") << endl;
   }
   cout << endl << "INFORMATIONS ON GENERATION OF FILE:" << endl << endl;
   cout << "Generated by : " << treeInfos->GetValue("gSystem->GetUserInfo()->fUser", "(unknown)") << endl ;
   cout << "Analysis task : " << treeInfos->GetValue("AnalysisTask", "(unknown)") << endl ;
   cout << "Job name : " << treeInfos->GetValue("BatchSystem.JobName", "(unknown)") << endl ;
   cout << "Job submitted from : " << treeInfos->GetValue("LaunchDirectory", "(unknown)") << endl ;
   cout << "Runs : " << treeInfos->GetValue("Runs", "(unknown)") << endl ;
   cout << "Number of events requested : " << treeInfos->GetValue("NbToRead", "(unknown)") << endl ;
   cout << endl << "----------------------------------------------------------------------------------------------------" << endl;

   // if possible, parse fDataVersion into series and release number
   // e.g. if fDataVersion = "1.8.10":
   //     => fDataSeries = "1.8"   fDataReleaseNum = 10
   // e.g. if fDataVersion = "1.11/00":
   //     => fDataSeries = "1.11"   fDataReleaseNum = 0
   Int_t a, b, c;
   if (fDataVersion != "(unknown)") {
      if (fDataVersion.Contains("/")) {
         if (sscanf(fDataVersion.Data(), "%d.%d/%d", &a, &b, &c) == 3) {
            fDataSeries.Form("%d.%d", a, b);
            fDataReleaseNum = c;
         }
      }
      else {
         if (sscanf(fDataVersion.Data(), "%d.%d.%d", &a, &b, &c) == 3) {
            fDataSeries.Form("%d.%d", a, b);
            fDataReleaseNum = c;
         }
      }
   }
   else {
      fDataSeries = "";
      fDataReleaseNum = -1;
   }
}
