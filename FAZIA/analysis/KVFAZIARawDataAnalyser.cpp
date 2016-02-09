/*
$Id: KVFAZIARawDataAnalyser.cpp,v 1.4 2007/11/15 14:59:45 ebonnet Exp $
$Revision: 1.4 $
$Date: 2007/11/15 14:59:45 $
*/

//Created by KVClassFactory on Wed Apr  5 23:50:04 2006
//Author: John Frankland

#include "KVBase.h"
#include "KVFAZIARawDataAnalyser.h"
#include "KVDataAnalysisTask.h"
#include "KVDataSet.h"
#include "TChain.h"
#include "TObjString.h"
#include "TChain.h"
#include "KVMultiDetArray.h"
#include "KVAvailableRunsFile.h"
#include "KVFAZIADBRun.h"

using namespace std;

ClassImp(KVFAZIARawDataAnalyser)
////////////////////////////////////////////////////////////////////////////////
// For analysing raw FAZIA data
//
////////////////////////////////////////////////////////////////////////////////
KVFAZIARawDataAnalyser::KVFAZIARawDataAnalyser()
{
   //Default constructor
   fDataSelector = "none";
   theChain = 0;
//   theRawData=0;
   fSelector = 0;
}

void KVFAZIARawDataAnalyser::Reset()
{
   //Reset task variables
   KVDataAnalyser::Reset();
   fDataSelector = "none";
   theChain = 0;
   //theRawData=0;
   fSelector = 0;

}

KVFAZIARawDataAnalyser::~KVFAZIARawDataAnalyser()
{
   //Destructor
   SafeDelete(fSelector);
}

//_________________________________________________________________

Bool_t KVFAZIARawDataAnalyser::CheckTaskVariables()
{
   // Checks the task variables

   if (!KVDataAnalyser::CheckTaskVariables()) return kFALSE;

   if (fDataSelector == "none") {
      //ChooseKVDataSelector();
   }

   cout << "============> Analysis summary <=============" << endl;
   cout << "Analysis of runs " << fRunList.
        GetList() << " with the TSelector ";
   cout << "\"" << GetUserClass() << "\"." << endl;
   if (nbEventToRead) {
      cout << nbEventToRead << " events will be processed." << endl;
   } else {
      cout << "All events will be processed." << endl;
   }
   cout << "=============================================" << endl;

   return kTRUE;
}

//_________________________________________________________________

void KVFAZIARawDataAnalyser::SubmitTask()
{
   //Run the interactive analysis

   //make the chosen dataset the active dataset ( = gDataSet; note this also opens database
   //and positions gDataBase & gIndraDB).
   fDataSet->cd();
   fSelector = 0;

   theChain = new TChain("FAZIA");
   theChain->SetDirectory(0); // we handle delete

   fRunList.Begin();
   Int_t run;

   // open and add to TChain all required files
   // we force the opening of the files to avoid problems with xrootd which sometimes
   // seems to have a little difficulty
   while (!fRunList.End()) {
      run = fRunList.Next();
      TString fullPathToRunfile = gDataSet->GetFullPathToRunfile(fDataType.Data(), run);
      cout << "Opening file " << fullPathToRunfile << endl;
      TFile* f = (TFile*)gDataSet->OpenRunfile(fDataType.Data(), run);
      cout << "Adding file " << fullPathToRunfile;
      cout << " to the TChain." << endl;
      theChain->Add(fullPathToRunfile);
      if (f && !f->IsZombie()) {
         // update run infos in available runs file if necessary

         /*
         KVAvailableRunsFile* ARF = gDataSet->GetAvailableRunsFile(fDataType.Data());
         if( ARF->InfosNeedUpdate(run, gSystem->BaseName( fullPathToRunfile )) ){
            if ( !((TTree*)f->Get("ReconstructedEvents")) ){
               Error("SubmitTask","No tree named ReconstructedEvents is present in the current file");
               delete theChain;
               return;
            }
            TEnv*treeInfos = (TEnv*)((TTree*)f->Get("ReconstructedEvents"))->GetUserInfo()->FindObject("TEnv");
            if (treeInfos){
               TString kvversion = treeInfos->GetValue("KVBase::GetKVVersion()","");
               TString username = treeInfos->GetValue("gSystem->GetUserInfo()->fUser","");
               if(kvversion!="") ARF->UpdateInfos(run, gSystem->BaseName( fullPathToRunfile ), kvversion, username);
            }
            else{
               Info("SubmitTask","No TEnv object associated to the tree");
            }
         }
         */
      }
   }

   TString option("");
   if (fDataSelector.Length()) {
      option.Form("DataSelector=%s", fDataSelector.Data());
      cout << "Data Selector : " << fDataSelector.Data() << endl;
   }

   fSelector = (KVFAZIAReader*)GetInstanceOfUserClass();

   if (!fSelector || !fSelector->InheritsFrom("TSelector")) {
      cout << "The selector \"" << GetUserClass() << "\" is not valid." << endl;
      cout << "Process aborted." << endl;
   } else {
      SafeDelete(fSelector);
      Info("SubmitTask", "Beginning TChain::Process...");
      preInitAnalysis();
      if (nbEventToRead) {
         theChain->Process(GetUserClass(), option.Data(), nbEventToRead);
      } else {
         theChain->Process(GetUserClass(), option.Data());
      }
   }
   theChain->ResetBranchAddresses();
   Info("SubmitTask", "call of TChain destructor");
   SafeDelete(theChain);
   Info("SubmitTask", "end of call");
   fSelector = 0; //deleted by TChain/TTreePlayer
}

//_________________________________________________________________

void KVFAZIARawDataAnalyser::ChooseKVDataSelector()
{
   // Choose the KVDataSelector. Look for source files in current working directory.
   SetKVDataSelector();
   cout << "Give the name of the Data Selector: [<RET>=none]" << endl;
   fDataSelector.ReadToDelim(cin);
   if (fDataSelector != "") KVBase::FindClassSourceFiles(fDataSelector.Data(), fDataSelectorImp, fDataSelectorDec);
}

//__________________________________________________________________________________//

void KVFAZIARawDataAnalyser::WriteBatchEnvFile(const Char_t* jobname, Bool_t save)
{
   //Save (in the TEnv fBatchEnv) all necessary information on analysis task which can be used to execute it later
   //(i.e. when batch processing system executes the job).
   //If save=kTRUE (default), write the information in a file whose name is given by ".jobname"
   //where 'jobname' is the name of the job as given to the batch system.


   KVDataAnalyser::WriteBatchEnvFile(jobname, kFALSE);

   if (fDataSelector != "none" && fDataSelector != "") {
      fBatchEnv->SetValue("KVDataSelector", fDataSelector.Data());
      if (fDataSelectorImp != "") fBatchEnv->SetValue("KVDataSelectorImp", fDataSelectorImp.Data());
      if (fDataSelectorDec != "") fBatchEnv->SetValue("KVDataSelectorDec", fDataSelectorDec.Data());
   }

   if (save) fBatchEnv->SaveLevel(kEnvUser);

}

//_________________________________________________________________

Bool_t KVFAZIARawDataAnalyser::ReadBatchEnvFile(const Char_t* filename)
{
   //Read the batch env file "filename" and initialise the analysis task using the
   //informations in the file
   //Returns kTRUE if all goes well

   Bool_t ok = kFALSE;


   if (!KVDataAnalyser::ReadBatchEnvFile(filename)) return ok;


   fDataSelector = fBatchEnv->GetValue("KVDataSelector", "");
   if (fDataSelector != "" && fDataSelector != "none") {
      //if names of source files for selector are known, and if current working directory
      //is not the same as the launch directory, we have to copy the user's files here
      fDataSelectorImp = fBatchEnv->GetValue("KVDataSelectorImp", "");
      fDataSelectorDec = fBatchEnv->GetValue("KVDataSelectorDec", "");
      TString launchDir = fBatchEnv->GetValue("LaunchDirectory", gSystem->WorkingDirectory());
      if ((fDataSelectorImp != "") && (launchDir != gSystem->WorkingDirectory())) {
         TString path_src, path_trg;
         //copy user's implementation file
         AssignAndDelete(path_src, gSystem->ConcatFileName(launchDir.Data(), fDataSelectorImp.Data()));
         AssignAndDelete(path_trg, gSystem->ConcatFileName(gSystem->WorkingDirectory(), fDataSelectorImp.Data()));
         gSystem->CopyFile(path_src.Data(), path_trg.Data());
         //copy user's header file
         AssignAndDelete(path_src, gSystem->ConcatFileName(launchDir.Data(), fDataSelectorDec.Data()));
         AssignAndDelete(path_trg, gSystem->ConcatFileName(gSystem->WorkingDirectory(), fDataSelectorDec.Data()));
         gSystem->CopyFile(path_src.Data(), path_trg.Data());
      }
   }

   ok = kTRUE;

   return ok;

}

//_________________________________________________________________

void KVFAZIARawDataAnalyser::SetKVDataSelector(const Char_t* kvs)
{
   //Set name of data selector to use. Look for source files in current working directory.
   fDataSelector = kvs;
   if (fDataSelector != "") KVBase::FindClassSourceFiles(kvs, fDataSelectorImp, fDataSelectorDec);
}

//_________________________________________________________________

const Char_t* KVFAZIARawDataAnalyser::ExpandAutoBatchName(const Char_t* format)
{
   //Replace any 'special' symbols in "format" with their current values
   //
   //  $Date   : current date and time
   //  $System  :  name of system to be analysed
   //  $User  :  name of user
   //  $UserClass or $Selector :  name of user's analysis class (KVSelector)
   //  $DataSelector  :  name of user's data selector (KVDataSelector)

   static KVString tmp;
   tmp = KVDataAnalyser::ExpandAutoBatchName(format);
   tmp.ReplaceAll("$Selector", GetUserClass());
   tmp.ReplaceAll("$DataSelector", GetKVDataSelector());
   return tmp.Data();
}

KVNumberList KVFAZIARawDataAnalyser::PrintAvailableRuns(KVString& datatype)
{
   //Prints list of available runs, sorted according to multiplicity
   //trigger, for selected dataset, data type/analysis task, and system
   //Returns list containing all run numbers

   KVNumberList all_runs =
      fDataSet->GetRunList(datatype.Data(), fSystem);

   KVFAZIADBRun* dbrun;

   //first read list and find what triggers are available
   std::vector<int> triggers;
   all_runs.Begin();
   while (!all_runs.End()) {
      dbrun = (KVFAZIADBRun*)fDataSet->GetDataBase()->GetTable("Runs")->GetRecord(all_runs.Next());
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
         dbrun = (KVFAZIADBRun*)fDataSet->GetDataBase()->GetTable("Runs")->GetRecord(all_runs.Next());
         if (dbrun->GetTrigger() == *it) {
            cout << "    " << Form("%4d", dbrun->GetNumber());
            cout << Form("\t(%7d events)", dbrun->GetEvents());
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

void KVFAZIARawDataAnalyser::preInitAnalysis()
{
   // Called by currently-processed KVSelector before user's InitAnalysis() method.
   // We build the multidetector for the current dataset in case informations on
   // detector are needed e.g. to define histograms in InitAnalysis().
   // Note that at this stage we are not analysing a given run, so the parameters
   // of the array are not set (they will be set in preInitRun()).

   Info("preInitAnalysis", "Start");
   if (!gMultiDetArray) KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());
}


void KVFAZIARawDataAnalyser::preInitRun()
{
   // Called by currently-processed KVSelector when a new file in the TChain is opened.
   // We call gIndra->SetParameters for the current run.

   Info("preInitRun", "Start");

   Int_t run = GetRunNumberFromFileName(theChain->GetCurrentFile()->GetName());
   gMultiDetArray->SetParameters(run);
   //ConnectRawDataTree();
   PrintTreeInfos();

}

void KVFAZIARawDataAnalyser::preAnalysis()
{
   Info("preAnalysis", "Start");
   // Read and set raw data for the current reconstructed event
   //if(!theRawData) return;
   // all recon events are numbered 1, 2, ... : therefore entry number is N-1
   //Long64_t rawEntry = fSelector->GetEventNumber() - 1;

   //gIndra->GetACQParams()->R__FOR_EACH(KVACQParam,Clear)();

   /*
   theRawData->GetEntry(rawEntry);
   for(int i=0; i<NbParFired; i++){
      KVACQParam* par = gIndra->GetACQParam((*parList)[ParNum[i]]->GetName());
      if(par) {par->SetData(ParVal[i]);}
   }
   */
}

/*
void KVFAZIARawDataAnalyser::ConnectRawDataTree()
{
   // Called by preInitRun().
   // When starting to read a new run (=new file), we look for the TTree "RawData" in the
   // current file (it should have been created by KVINDRARawDataReconstructor).
   // If found, it will be used by ReadRawData() to set the values of all acquisition parameters
   // for each event.

   theRawData=(TTree*)theChain->GetCurrentFile()->Get("RawData");
   if(!theRawData){
      Warning("ConnectRawDataTree", "RawData tree not found in file; raw data parameters of detectors will not be available in analysis");
      return;
   }
   else
      Info("ConnectRawDataTree", "Found RawData tree in file");
   Int_t maxNopar = theRawData->GetMaximum("NbParFired");
   if(ParVal) delete [] ParVal;
   if(ParNum) delete [] ParNum;
   ParVal = new UShort_t[maxNopar];
   ParNum = new UInt_t[maxNopar];
   parList = (TObjArray*)theRawData->GetUserInfo()->FindObject("ParameterList");
   theRawData->SetBranchAddress("NbParFired", &NbParFired);
   theRawData->SetBranchAddress("ParNum", ParNum);
   theRawData->SetBranchAddress("ParVal", ParVal);
   Info("ConnectRawDataTree", "Connected raw data parameters");
   Entry=0;
}
*/

TEnv* KVFAZIARawDataAnalyser::GetReconDataTreeInfos() const
{
   return (TEnv*)theChain->GetTree()->GetUserInfo()->FindObject("TEnv");
}

void KVFAZIARawDataAnalyser::PrintTreeInfos()
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
   } else {
      fDataSeries = "";
      fDataReleaseNum = -1;
   }

}
