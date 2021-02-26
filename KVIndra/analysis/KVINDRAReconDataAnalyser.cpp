/*
$Id: KVINDRAReconDataAnalyser.cpp,v 1.4 2007/11/15 14:59:45 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/15 14:59:45 $
*/

//Created by KVClassFactory on Wed Apr  5 23:50:04 2006
//Author: John Frankland

#include "KVBase.h"
#include "KVINDRAReconDataAnalyser.h"
#include "KVINDRADBRun.h"
#include "KVINDRADB.h"
#include "KVDataAnalysisTask.h"
#include "KVDataSet.h"
#include "TChain.h"
#include "TObjString.h"
#include "TChain.h"
#include "KVAvailableRunsFile.h"
#include "KVINDRA.h"
#include "TProof.h"
#include "KVINDRATriggerConditions.h"

#include <KVReconEventSelector.h>

using namespace std;

ClassImp(KVINDRAReconDataAnalyser)

KVINDRAReconDataAnalyser::KVINDRAReconDataAnalyser()
   : fSelector(nullptr), fOldSelector(nullptr), theChain(nullptr),
     theRawData(nullptr), theGeneData(nullptr),
     ParVal(nullptr), ParNum(nullptr), parList(nullptr)
{
}

void KVINDRAReconDataAnalyser::Reset()
{
   //Reset task variables
   KVDataAnalyser::Reset();
   theChain = nullptr;
   theRawData = nullptr;
   theGeneData = nullptr;
   ParVal = nullptr;
   ParNum = nullptr;
   fSelector = nullptr;
   fOldSelector = nullptr;
   TotalEntriesToRead = 0;
}

KVINDRAReconDataAnalyser::~KVINDRAReconDataAnalyser()
{
   //Destructor
   if (ParVal) delete [] ParVal;
   if (ParNum) delete [] ParNum;
   SafeDelete(fSelector);
   SafeDelete(fOldSelector);
}

//_________________________________________________________________

Bool_t KVINDRAReconDataAnalyser::CheckTaskVariables()
{
   // Checks the task variables

   if (!KVDataAnalyser::CheckTaskVariables()) return kFALSE;

   cout << "============> Analysis summary <=============" << endl;
   cout << "Analysis of runs " << GetRunList().
        GetList() << " with the class ";
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

void KVINDRAReconDataAnalyser::SubmitTask()
{
   //Run the interactive analysis

   //make the chosen dataset the active dataset ( = gDataSet; note this also opens database
   //and positions gDataBase & gIndraDB).
   GetDataSet()->cd();
   fSelector = nullptr;
   fOldSelector = nullptr;

   theChain = new TChain("ReconstructedEvents");
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
            if (!((TTree*)f->Get("ReconstructedEvents"))) {
               Error("SubmitTask", "No tree named ReconstructedEvents is present in the current file");
               delete theChain;
               return;
            }
            TEnv* treeInfos = (TEnv*)((TTree*)f->Get("ReconstructedEvents"))->GetUserInfo()->FindObject("TEnv");
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
   TString option("");

   // Add the total run list in option
   if (!(option.IsNull())) option += ",";
   option += Form("FullRunList=%s", GetFullRunList().GetList());

   // Add any user-defined options
   if (GetUserClassOptions() != "") {
      option += ",";
      option += GetUserClassOptions();
   }

   set_up_analyser_for_task(this);

   // for backwards compatibility, we allow user class to inherit from
   // KVOldINDRASelector instead of KVINDRAEventSelector
   // for forwards compatibility, we allow it to inherit from KVReconEventSelector too!
   TObject* new_selector = GetInstanceOfUserClass("KVOldINDRASelector,KVReconEventSelector");

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
   fOldSelector = nullptr; //deleted by TChain/TTreePlayer
}

//__________________________________________________________________________________//

void KVINDRAReconDataAnalyser::WriteBatchEnvFile(const Char_t* jobname, Bool_t save)
{
   //Save (in the TEnv fBatchEnv) all necessary information on analysis task which can be used to execute it later
   //(i.e. when batch processing system executes the job).
   //If save=kTRUE (default), write the information in a file whose name is given by ".jobname"
   //where 'jobname' is the name of the job as given to the batch system.

   KVDataSetAnalyser::WriteBatchEnvFile(jobname, kFALSE);
   // backwards-compatible fix for old KVSelector analysis classes
   GetBatchInfoFile()->SetValue("UserClassAlternativeBaseClass", "KVOldINDRASelector");
   if (save) GetBatchInfoFile()->SaveLevel(kEnvUser);
}

KVNumberList KVINDRAReconDataAnalyser::PrintAvailableRuns(KVString& datatype)
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

void KVINDRAReconDataAnalyser::preInitAnalysis()
{
   // Called by currently-processed KVEventSelector before user's InitAnalysis() method.
   // We build the multidetector for the current dataset in case informations on
   // detector are needed e.g. to define histograms in InitAnalysis().
   // Note that at this stage we are not analysing a given run, so the parameters
   // of the array are not set (they will be set in preInitRun()).
   //
   // Note for PROOF: as this will be called both on master and on slave workers,
   // in order to reduce memory footprint we only build INDRA on the slaves

   if (!gProof || !gProof->IsMaster()) {
      if (!gIndra) KVMultiDetArray::MakeMultiDetector(GetDataSet()->GetName());
   }
   // in case data is being analysed with a "vanilla" KVReconEventSelector, we need
   // to set the correct branch name for reconstructed INDRA data which is "INDRAReconEvent"
   // instead of the default "ReconEvent"
   if (fSelector) fSelector->SetBranchName("INDRAReconEvent");
   else Warning("preInitAnalysis", "could not set branch name correctly");
}

void KVINDRAReconDataAnalyser::SetSelectorCurrentRun(KVINDRADBRun* CurrentRun)
{
   if (fSelector) {
      if (fSelector->InheritsFrom("KVINDRAEventSelector"))
         dynamic_cast<KVINDRAEventSelector*>(fSelector)->SetCurrentRun(CurrentRun);
      // WARNING - horrible kludge...
      else if (fSelector->InheritsFrom("KVReconEventSelector"))
         dynamic_cast<KVReconEventSelector*>(fSelector)->SetCurrentRun(CurrentRun);
   }
   else {
      fOldSelector->SetCurrentRun(CurrentRun);
   }
}

void KVINDRAReconDataAnalyser::preInitRun()
{
   // Called by currently-processed TSelector when a new file in the TChain is opened.
   // We call gIndra->SetParameters for the current run.
   // We connect the acquisition parameter objects to the branches of the raw data tree.
   // Infos on currently read file/tree are printed.
   // Any required data patches ("rustines") are initialized.

   Int_t run = GetRunNumberFromFileName(theChain->GetCurrentFile()->GetName());
   gIndra->SetParameters(run);
   KVINDRADBRun* CurrentRun = gIndraDB->GetRun(run);
   SetCurrentRun(CurrentRun);
   SetSelectorCurrentRun(CurrentRun);
   cout << endl << " ===================  New Run  =================== " <<
        endl << endl;

   CurrentRun->Print();
   if (CurrentRun->GetSystem()) {
      SetSystem(CurrentRun->GetSystem());
      if (CurrentRun->GetSystem()->GetKinematics())
         CurrentRun->GetSystem()->GetKinematics()->Print();
   }

   cout << endl << " ================================================= " <<
        endl << endl;

   ConnectRawDataTree();
   ConnectGeneDataTree();
   PrintTreeInfos();
   Info("preInitRun", "Data written with series %s, release %d", GetDataSeries().Data(),
        GetDataReleaseNumber());
   fRustines.InitializePatchList(GetDataSet()->GetName(), GetDataType(), run, GetDataSeries(),
                                 GetDataReleaseNumber(), theChain->GetCurrentFile()->GetStreamerInfoCache());
   fRustines.Print();
}

Long64_t KVINDRAReconDataAnalyser::GetRawEntryNumber()
{
   Long64_t rawEntry = (fSelector ? fSelector->GetEventNumber() - 1
                        : fOldSelector->GetEventNumber() - 1);

   return rawEntry;
}

KVReconstructedEvent* KVINDRAReconDataAnalyser::GetReconstructedEvent()
{
   return (fSelector ? dynamic_cast<KVReconstructedEvent*>(fSelector->GetEvent()) : fOldSelector->GetEvent());
}

#ifdef USING_ROOT6
void KVINDRAReconDataAnalyser::SetTriggerConditionsForRun(int run)
{
   KVINDRATriggerConditions trig;
   trig.SetTriggerConditionsForRun(fSelector, run);
}
#endif

void KVINDRAReconDataAnalyser::preAnalysis()
{
   // Read and set raw data for the current reconstructed event
   // Any required data patches ("rustines") are applied.

   if (!theRawData) return;
   // all recon events are numbered 1, 2, ... : therefore entry number is N-1
   Long64_t rawEntry = GetRawEntryNumber();

//   TIter it(gIndra->GetACQParams());
//   KVACQParam* a;
//   while ((a = (KVACQParam*)it())) a->Clear();

//   theRawData->GetEntry(rawEntry);
//   for (int i = 0; i < NbParFired; i++) {
//      KVACQParam* par = gIndra->GetACQParam((*parList)[ParNum[i]]->GetName());
//      if (par) {
//         par->SetData(ParVal[i]);
//      }
//   }
   gIndra->SetMinimumOKMultiplicity(GetReconstructedEvent());

   // as rustines often depend on a knowledge of the original raw data,
   // we apply them after it has been read in
   KVINDRAReconEvent* event = (KVINDRAReconEvent*)GetReconstructedEvent();
   if (fRustines.HasActivePatches()) fRustines.Apply(event);
}

void KVINDRAReconDataAnalyser::ConnectRawDataTree()
{
   // Called by preInitRun().
   // When starting to read a new run (=new file), we look for the TTree "RawData" in the
   // current file (it should have been created by KVINDRARawDataReconstructor).
   // If found, it will be used by ReadRawData() to set the values of all acquisition parameters
   // for each event.

   theRawData = (TTree*)theChain->GetCurrentFile()->Get("RawData");
   if (!theRawData) {
      Warning("ConnectRawDataTree", "RawData tree not found in file; raw data parameters of detectors will not be available in analysis");
      return;
   }
   else
      Info("ConnectRawDataTree", "Found RawData tree in file");
   Int_t maxNopar = theRawData->GetMaximum("NbParFired");
   if (ParVal) delete [] ParVal;
   if (ParNum) delete [] ParNum;
   ParVal = new UShort_t[maxNopar];
   ParNum = new UInt_t[maxNopar];
   parList = (TObjArray*)theRawData->GetUserInfo()->FindObject("ParameterList");
   theRawData->SetBranchAddress("NbParFired", &NbParFired);
   theRawData->SetBranchAddress("ParNum", ParNum);
   theRawData->SetBranchAddress("ParVal", ParVal);
   Info("ConnectRawDataTree", "Connected raw data parameters");
   Entry = 0;
}

void KVINDRAReconDataAnalyser::ConnectGeneDataTree()
{
   // Called by preInitRun().
   // When starting to read a new run (=new file), we look for the TTree "GeneData" in the
   // current file (it should have been created by KVINDRARawDataReconstructor).

   theGeneData = (TTree*)theChain->GetCurrentFile()->Get("GeneData");
   if (!theGeneData) {
      cout << "  --> No pulser & laser data for this run !!!" << endl << endl;
   }
   else {
      cout << "  --> Pulser & laser data tree contains " << theGeneData->GetEntries()
           << " events" << endl << endl;
   }
}

TEnv* KVINDRAReconDataAnalyser::GetReconDataTreeInfos() const
{
   return (TEnv*)theChain->GetTree()->GetUserInfo()->FindObject("TEnv");
}

void KVINDRAReconDataAnalyser::PrintTreeInfos()
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
   cout << "BZR branch name = " << treeInfos->GetValue("KVBase::bzrBranchNick()", "(unknown)") << endl ;
   cout << "BZR revision #" << treeInfos->GetValue("KVBase::bzrRevisionNumber()", "(unknown)") << endl ;
   cout << "BZR revision ID = " << treeInfos->GetValue("KVBase::bzrRevisionId()", "(unknown)") << endl ;
   cout << "BZR revision date = " << treeInfos->GetValue("KVBase::bzrRevisionDate()", "(unknown)") << endl ;
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
   Int_t a, b, c;
   if (fDataVersion != "(unknown)") {
      if (sscanf(fDataVersion.Data(), "%d.%d.%d", &a, &b, &c) == 3) {
         fDataSeries.Form("%d.%d", a, b);
         fDataReleaseNum = c;
      }
   }
   else {
      fDataSeries = "";
      fDataReleaseNum = -1;
   }
}

void KVINDRAReconDataAnalyser::CloneRawAndGeneTrees()
{
   if (theRawData) theRawData->CloneTree(-1, "fast"); //copy raw data tree to file
   if (theGeneData) theGeneData->CloneTree(-1, "fast"); //copy pulser & laser (gene) tree to file
}

Bool_t KVINDRAReconDataAnalyser::CheckIfUserClassIsValid(const KVString&)
{
   // Overrides KVDataAnalyser method, for backwards & forwards compatibility.
   //
   // Old user analysis classes may derive from KVOldINDRASelector (aka KVSelector) instead of KVINDRAEventSelector.
   //
   // It is also possible to analyse data with a "vanilla" KVReconEventSelector analysis class.
   return KVDataAnalyser::CheckIfUserClassIsValid("KVOldINDRASelector,KVReconEventSelector");
}
