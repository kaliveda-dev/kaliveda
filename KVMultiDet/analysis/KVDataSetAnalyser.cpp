//Created by KVClassFactory on Tue Feb 14 12:32:06 2017
//Author: John Frankland,,,

#include "KVDataSetAnalyser.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "KVDataSet.h"
#include "KVDBSystem.h"
#include "KV2Body.h"
#include "KVDBRun.h"
#include <iostream>
using namespace std;

ClassImp(KVDataSetAnalyser)


KVDataSetAnalyser::KVDataSetAnalyser()
   : KVDataAnalyser()
{
   // Default constructor
   fSystem = nullptr;
   fDataSet = nullptr;
   fChoozDataSet = kTRUE;
   fChoozTask = kFALSE;
   fChoozSystem = kFALSE;
   fChoozRuns = kFALSE;
}

//____________________________________________________________________________//

KVDataSetAnalyser::~KVDataSetAnalyser()
{
   // Destructor
}

//____________________________________________________________________________//

void KVDataSetAnalyser::Reset()
{
   KVDataAnalyser::Reset();
   fDataSet = nullptr;
   fRunList.Clear();
   fSystem = nullptr;
   fChoozDataSet = kTRUE;
   fChoozTask = kFALSE;
   fChoozSystem = kFALSE;
   fChoozRuns = kFALSE;
}

Bool_t KVDataSetAnalyser::PreSubmitCheck()
{
   // Called just before SubmitTask() in Run()
   // Checks if remote repository is connected if necessary
   if (GetParent() && gDataRepository->IsRemote() && !gDataRepository->IsConnected()) return kFALSE;
   return kTRUE;
}

void KVDataSetAnalyser::PostRunReset()
{
   // Called at end of Run() after execution of analysis task
   KVDataAnalyser::PostRunReset();
   fChoozDataSet = kTRUE;
}

void KVDataSetAnalyser::ChooseWhatToAnalyse()
{
   while (!IsQuit()) {
      if (fChoozSystem)
         ChooseSystem();
      else if (fChoozRuns)
         ChooseRuns();
      else
         break;
   }
}

void KVDataSetAnalyser::ChooseWhatToDo()
{
   while (!IsQuit()) {
      if (fChoozDataSet)
         ChooseDataSet();
      else if (IsChooseTask())
         ChooseAnalysisTask();
      else
         break;
   }
}

Bool_t KVDataSetAnalyser::CheckWhatToAnalyseAndHow()
{
   if (!fDataSet) {
      ChooseDataSet();
      if (!fDataSet) {
         //if after calling ChooseDataSet we STILL don't have a dataset,
         //there is something seriously wrong...
         Error("CheckTaskVariables", "By the pricking of my thumb, something wicked this way comes...");
         Error("CheckTaskVariables", "                        *** ABORTING THE ANALYSIS ***");
         return kFALSE;
      }
   }

   if (!GetAnalysisTask()) {
      ChooseAnalysisTask();
   }

   if (fRunList.IsEmpty()) {
      ChooseSystem();
      ChooseRuns();
   }
   return kTRUE;
}

//_________________________________________________________________

void KVDataSetAnalyser::ChooseDataSet()
{
   //Print list of available datasets and get user to choose one
   //A pointer to the chosen dataset can be retrieved with GetDataSet()

   //not possible in batch mode
   if (BatchMode()) return;

   fChoozDataSet = kFALSE;
   SetQuit(kFALSE);
   _set_dataset_pointer(nullptr);
   if (GetAnalysisTask()) SetAnalysisTask(nullptr);

   cout << "Available datasets :" << endl << endl;
   gDataSetManager->Print("available");
   Int_t n_dataset = -1;
   Int_t n_avail = gDataSetManager->GetNavailable();

   if (n_avail < 1) {
      //no datasets available - force termination
      SetQuit();
      return;
   }

   if (n_avail == 1) {
      //1 dataset available - automatic choice
      n_dataset = 1;
   }
   else {
      KVString tmp;
      while (!IsQuit() && (n_dataset < 1 || n_dataset > n_avail)) {
         cout << endl << "Your choice (1-" << gDataSetManager->
              GetNavailable() << ")";
         if (IsMenus()) cout << " [q - quit]";
         cout << " : ";
         tmp.ReadLine(cin);
         if (IsMenus() && (tmp.Contains("q") || tmp.Contains("Q"))) {
            SetQuit();
            return;
         }
         else if (tmp.IsDigit()) {
            n_dataset = tmp.Atoi();
         }
      }
   }
   //we have chosen a dataset
   _set_dataset_pointer(gDataSetManager->GetAvailableDataSet(n_dataset));
   //in menu-driven mode, next step is choice of analysis task
   SetChooseTask();
}

void KVDataSetAnalyser::ChooseDataType()
{
   //Print list of available types of data for the chosen dataset
   //and get user to choose one.

   //not possible in batch mode
   if (BatchMode()) return;

   SetChooseTask(kFALSE);
   SetQuit(kFALSE);
   SetChooseSystem(kFALSE);
   SetChooseDataSet(kFALSE);
   SetAnalysisTask(nullptr);

   fDataSet->Print("data");
   cout << endl << "Choose data type [d - change dataset | q - quit] : ";
   TString tmp;
   while (!IsQuit() && !IsChooseDataSet() && !IsChooseSystem()) {

      tmp.ReadLine(cin);
      if (tmp == "raw" || tmp == "recon" || tmp == "ident" || tmp == "root") {
         fChoozSystem = kTRUE;
         SetDataType(tmp);
      }
      else if (tmp.Contains("q") || tmp.Contains("Q")) {
         SetQuit();
      }
      else if (tmp.Contains("d") || tmp.Contains("D")) {
         SetChooseDataSet();
      }

   }
}

//_________________________________________________________________

void KVDataSetAnalyser::ChooseAnalysisTask()
{
   //Print list of all possible data analysis tasks for the chosen data set and invite the user
   //to choose one of them.
   //If only one task available, it is automatically selected

   //not possible in batch mode
   if (BatchMode()) return;

   SetChooseTask(kFALSE);
   SetAnalysisTask(nullptr);
   if (fSystem) {
      fSystem = nullptr;
   }

   cout << endl << "Available data analysis tasks :" << endl << endl;
   fDataSet->Print("tasks");
   Int_t n_tasks = fDataSet->GetNtasks();
   if (n_tasks == 1) {
      SetAnalysisTask(fDataSet->GetAnalysisTask(1));
      fChoozSystem = kTRUE;
      return;
   }

   Int_t n_task = -1;
   KVString tmp;

   while (!IsQuit() && (n_task < 1 || n_task > n_tasks)) {
      cout << endl << "Your choice (1-" << n_tasks <<
           ")";
      if (IsMenus()) cout << " [d - change dataset | q - quit]";
      cout << " : ";
      tmp.ReadLine(cin);
      if (IsMenus() && (tmp.Contains("q") || tmp.Contains("Q"))) {
         SetQuit();
         return;
      }
      else if (IsMenus() && (tmp.Contains("d") || tmp.Contains("D"))) {
         fChoozDataSet = kTRUE;
         return;
      }
      else if (tmp.IsDigit()) {
         n_task = tmp.Atoi();
      }
   }
   SetAnalysisTask(fDataSet->GetAnalysisTask(n_task));
   //in menu-driven mode, next step is choice of system
   fChoozSystem = kTRUE;
}

//_________________________________________________________________

void KVDataSetAnalyser::ChooseSystem(const Char_t* data_type)
{
   //Print out list of all available systems for the given data type of the chosen dataset
   //and invite the user to choose one. Return pointer to chosen system.
   //If 'data_type' is not given, we assume that ChooseAnalysisTask was previously
   //called and we use the prerequisite data type for the chosen task (fTask->GetPrereq()).
   //The available system list is deduced from the 'availableruns' file corresponding to the
   //chosen dataset repository subdirectory and the
   //chosen data type (="raw", "recon", "ident", "root").

   //not possible in batch mode
   if (BatchMode()) return;

   fChoozSystem = kFALSE;
   fSystem = 0;
   ClearRunList();

   //if no systems are defined for the dataset, we just want to pick from the runlist
   if (!fDataSet->GetDataBase()->GetSystems()->GetSize()) {
      fChoozRuns = kTRUE;
      return;
   }

   TString d_t(data_type);
   //If 'data_type' is not given, we assume that ChooseAnalysisTask or ChooseDataType
   //was previously called and we use the value of fDataType
   if (d_t == "")
      d_t = GetDataType();

   unique_ptr<TList> sys_list(fDataSet->GetListOfAvailableSystems(d_t.Data()));
   if (!sys_list.get() || sys_list->GetSize() < 1) {
      cout << "No systems found for dataset: " << fDataSet->
           GetName() << ", datatype: " << d_t.Data() << endl;
      //choose a different data type or analysis task
      fChoozTask = kTRUE;
      return;
   }
   TIter next_sys(sys_list.get());
   KVDBSystem* sys;
   int nsys = 0;
   cout << endl << "Available systems : " << endl << endl;
   while ((sys = (KVDBSystem*) next_sys())) {
      nsys++;
      cout << "     " << Form("%3d.", nsys);
      cout << " " << Form("%-30s", sys->GetName());
      cout << " (" << Form("%-3d",
                           sys->GetNumberRuns()) << " runs)" << endl;
   }

   if (sys_list->GetSize() == 1) {
      fChoozRuns = kTRUE;
      fSystem = (KVDBSystem*) sys_list->At(0);
      return;
   }

   int nsys_pick = -1;
   KVString tmp;

   SetQuit(kFALSE);
   SetChooseTask(kFALSE);
   fChoozRuns = kFALSE;

   while (!IsQuit() && !IsChooseTask() && !fChoozRuns && !fChoozDataSet) {
      cout << endl << "Your choice (1-" << nsys << ")";
      if (IsMenus()) {
         cout << " [r - choose runs | "; //ignore systems list & choose from all runs
         if (GetAnalysisTask())
            cout << "t - change task";
         else
            cout << "t - change type";
         cout << " | d - change dataset | q - quit]";
      }
      cout << " : ";
      tmp.ReadLine(cin);
      if (IsMenus() && (tmp.Contains("q") || tmp.Contains("Q"))) {
         SetQuit();
      }
      else if (IsMenus() && (tmp.Contains("t") || tmp.Contains("T"))) {
         SetChooseTask();
      }
      else if (IsMenus() && (tmp.Contains("d") || tmp.Contains("D"))) {
         fChoozDataSet = kTRUE;
      }
      else if (IsMenus() && (tmp.Contains("r") || tmp.Contains("R"))) {
         fChoozRuns = kTRUE;
         return;
      }
      else if (tmp.IsDigit()) {
         nsys_pick = tmp.Atoi();
         if (nsys_pick >= 1 && nsys_pick <= nsys)
            fChoozRuns = kTRUE;
      }
   }
   if (fChoozRuns)
      fSystem = (KVDBSystem*) sys_list->At(nsys_pick - 1);
}

//_________________________________________________________________

void KVDataSetAnalyser::ChooseRuns(KVDBSystem* system,
                                   const Char_t* data_type)
{
   //Print out list of available runs for chosen dataset, task/data type and system
   //and invite user to choose from among them
   //If 'data_type' is not given, we assume that ChooseAnalysisTask was previously
   //called and we use the prerequisite data type for the chosen task (fTask->GetPrereq()).
   //If 'system' is not given, we assume ChooseSystem was previously called and use
   //the internally-stored value of that choice (fSystem).

   //not possible in batch mode
   if (BatchMode()) return;

   fChoozRuns = kFALSE;
   //clear any previous list of runs
   fRunList.Clear();

   if (system)
      fSystem = system;

   KVString d_t(data_type);
   //If 'data_type' is not given, we assume that ChooseAnalysisTask or ChooseDataType
   //was previously called and we use the value of fDataType
   if (d_t == "")
      d_t = GetDataType();

   if (fSystem) {
      cout << endl << "  Chosen system : " << endl;
      fSystem->Print();
   }
   cout << endl << endl << "  Available runs: " << endl << endl;

   KVNumberList all_runs = PrintAvailableRuns(d_t);

   if (all_runs.IsEmpty()) {
      //no runs - choose another system
      fChoozSystem = kTRUE;
      return;
   }

   cout << endl << "Enter list of runs [a - all";
   if (IsMenus()) {
      cout << " | ";
      if (fSystem) cout << "s - change system | ";
      if (GetAnalysisTask())
         cout << "t - change task";
      else
         cout << "t - change type";
      cout << " | d - change dataset | q - quit";
   }
   cout << "] : ";
   TString tmp;
   tmp.ReadLine(cin);
   if (IsMenus() && (tmp.Contains("q") || tmp.Contains("Q"))) {
      SetQuit();
   }
   else if (IsMenus() && (tmp.Contains("s") || tmp.Contains("S"))) {
      fChoozSystem = kTRUE;
   }
   else if (IsMenus() && (tmp.Contains("t") || tmp.Contains("T"))) {
      SetChooseTask();
   }
   else if (IsMenus() && (tmp.Contains("d") || tmp.Contains("D"))) {
      fChoozDataSet = kTRUE;
   }
   else if (tmp.Contains("a") || tmp.Contains("A")) {
      fRunList = all_runs;
      SetSubmit();
   }
   else {
      //cout << "YOU TYPED : " << tmp.Data() << endl;
      KVNumberList user_list(tmp.Data());
      //cout << "YOUR LIST : " << user_list.GetList() << endl;
      //cout << "CLEARED RUNLIST : " <<  endl; fRunList.PrintLimits();
      //remove from list any runs which did not actually appear in list
      user_list.Begin();
      while (!user_list.End()) {
         Int_t user_run = user_list.Next();
         if (all_runs.Contains(user_run)) fRunList.Add(user_run);
      }
      //cout << "CHECKED RUNLIST : " <<  endl; fRunList.PrintLimits();
      if (fRunList.IsEmpty()) {
         Error("ChooseRuns",
               "None of the runs you chose appear in the list");
         //we force the user to choose again
         fChoozRuns = kTRUE;
      }
      else
         SetSubmit();
   }
   if (IsSubmit())
      cout << endl << "Chosen runs : " << fRunList.GetList() << endl;
}

//_________________________________________________________________

void KVDataSetAnalyser::set_dataset_pointer(KVDataSet* ds)
{
   //Set dataset to be used for analysis.
   //If the chosen dataset is not available, an error message is printed
   //Only available datasets can be analysed
   //Moreover, only datasets in the currently active data repository,
   //gDataRepository, can be analysed. This is also checked.

   //allow user to reset dataset pointer to 0
   fDataSet = ds;
   if (!ds)
      return;

   //check availablility
   if (!ds->IsAvailable()) {
      Error("SetDataSet",
            "Dataset %s is not available for analysis", ds->GetName());
      fDataSet = nullptr;
   }
   //check repository
   if (ds->GetRepository() != gDataRepository) {
      Error("SetDataSet",
            "%ld is address of dataset in repository \"%s\", not of the dataset in the current repository, \"%s\"",
            (ULong_t) ds, ds->GetRepository()->GetName(),
            gDataRepository->GetName());
      fDataSet = nullptr;
   }
}

//_________________________________________________________________

void KVDataSetAnalyser::set_dataset_name(const Char_t* name)
{
   //Set dataset to be analysed.
   //If 'name' is not the name of a valid and available dataset
   //in the currently active data repository, gDataRepository,
   //an error message is printed.

   fDataSet = nullptr;
   KVDataSet* ds = gDataSetManager->GetDataSet(name);
   if (!ds) {
      Error("SetDataSet", "Unknown dataset %s", name);
   }
   else {
      set_dataset_pointer(ds);
   }
}
//_________________________________________________________________

KVNumberList KVDataSetAnalyser::PrintAvailableRuns(KVString& datatype)
{
   //Prints list of available runs for selected dataset, data type/analysis task, and system
   //Returns list containing all run numbers

   KVNumberList all_runs =
      fDataSet->GetRunList(datatype.Data(), fSystem);
   KVDBRun* dbrun;
   all_runs.Begin();
   while (!all_runs.End()) {
      dbrun = fDataSet->GetDataBase()->GetDBRun(all_runs.Next());
      if (dbrun) {
         cout << "    " << Form("%4d", dbrun->GetNumber());
         cout << Form("\t(%9llu events)", dbrun->GetEvents());
         cout << "\t[File written: " << dbrun->GetDatime().AsString() << "]";
         if (dbrun->GetComments())
            cout << "\t" << dbrun->GetComments();
         cout << endl;
      }
   }

   return all_runs;
}

//_________________________________________________________________

void KVDataSetAnalyser::SetSystem(KVDBSystem* syst)
{
   // Set the System used in the analysis

   fSystem = syst;
}

//_________________________________________________________________

void KVDataSetAnalyser::SetRuns(const KVNumberList& nl, Bool_t check)
{
   // Sets the run list
   //If check=kTRUE (default), we check that the runs are available, and if they belong to different
   //systems we print a warning message
   if (!fDataSet) {
      Warning("SetRuns", "Data Set not defined... Nothing done");
      return;
   }
   if (!check) {
      fRunList = nl;
      //set fSystem using first run
      KVDBRun* run = fDataSet->GetDataBase()->GetDBRun(nl.First());
      if (run) fSystem = run->GetSystem();
      else fSystem = 0;
      return;
   }
   // Check if all runs are available for this data set and if they all correspond to the same system
   Int_t i = 0;
   Info("SetRuns", "Checking runs %s for Data type %s",
        nl.AsString(), GetDataType().Data());
   nl.Begin();
   fRunList.Clear();
   while (!nl.End()) {
      Int_t run_no = nl.Next();

      if (!(fDataSet->CheckRunfileAvailable(GetDataType(), run_no))) {
         Warning("SetRuns",
                 "The run %d is not present for the data type \"%s\" of data set \"%s\".",
                 run_no, GetDataType().Data(), fDataSet->GetName());
      }
      else {
         fRunList.Add(run_no);
      }
      KVDBRun* run = fDataSet->GetDataBase()->GetDBRun(run_no);
      if (!i) {
         fSystem = run->GetSystem();
         i = 1;
      }
      else {
         if (run->GetSystem() != fSystem) {
            if (fSystem)
               Warning("SetRuns",
                       "The system \"%s\" of run %d differs from the system \"%s\" of the previous runs.",
                       run->GetSystem()->GetName(), run_no, fSystem->GetName());
         }
      }
   }
   Info("SetRuns", "Accepted runs : %s", fRunList.AsString());
}

//_________________________________________________________________

void KVDataSetAnalyser::WriteBatchEnvFile(const Char_t* jobname, Bool_t save)
{
   //Save (in the TEnv fBatchEnv) all necessary information on analysis task which can be used to execute it later
   //(i.e. when batch processing system executes the job).
   //If save=kTRUE (default), write the information in a file whose name is given by ".jobname"
   //where 'jobname' is the name of the job as given to the batch system.

   KVDataAnalyser::WriteBatchEnvFile(jobname, kFALSE);

   GetBatchInfoFile()->SetValue("DataRepository", gDataRepository->GetName());
   GetBatchInfoFile()->SetValue("DataSet", fDataSet->GetName());
   GetBatchInfoFile()->SetValue("Runs", fRunList.GetList());
   GetBatchInfoFile()->SetValue("FullRunList", fFullRunList.GetList());
   if (save) GetBatchInfoFile()->SaveLevel(kEnvUser);
}

//_________________________________________________________________

Bool_t KVDataSetAnalyser::ReadBatchEnvFile(const Char_t* filename)
{
   //Read the batch env file "filename" and initialise the analysis task using the
   //informations in the file
   //Returns kTRUE if all goes well

   Bool_t ok = kFALSE;

   if (!KVDataAnalyser::ReadBatchEnvFile(filename)) return ok;

   TString val = GetBatchInfoFile()->GetValue("DataRepository", "");
   if (val != "") {
      gDataRepositoryManager->GetRepository(val.Data())->cd();
   }
   else {
      Error("ReadBatchEnvFile", "Name of data repository not given");
      return ok;
   }
   val = GetBatchInfoFile()->GetValue("DataSet", "");
   if (val != "") {
      gDataSetManager->GetDataSet(val.Data())->cd();
      SetDataSet(gDataSet);
   }
   else {
      Error("ReadBatchEnvFile", "Name of dataset not given");
      return ok;
   }
   val = GetBatchInfoFile()->GetValue("AnalysisTask", "");
   SetAnalysisTask(nullptr);
   if (val != "") {
      SetAnalysisTask(gDataSet->GetAnalysisTask(val.Data()));
   }
   else {
      Error("ReadBatchEnvFile", "Name of analysis task not given");
      return ok;
   }
   if (!GetAnalysisTask()) {
      Error("ReadBatchEnvFile", "Analysis task \"%s\"not found for dataset %s",
            val.Data(), gDataSet->GetName());
      return ok;
   }
   val = GetBatchInfoFile()->GetValue("Runs", "");
   if (val != "") {
      SetRuns(val.Data());
   }
   else {
      Error("ReadBatchEnvFile", "List of runs not given");
      return ok;
   }
   val = GetBatchInfoFile()->GetValue("FullRunList", "");
   if (val != "") {
      SetFullRunList(val.Data());
   }
   ok = kTRUE;

   return ok;
}

void KVDataSetAnalyser::set_up_analyser_for_task(KVDataAnalyser* the_analyser)
{
   KVDataAnalyser::set_up_analyser_for_task(the_analyser);
   KVDataSetAnalyser* t_a = dynamic_cast<KVDataSetAnalyser*>(the_analyser);
   t_a->SetDataSet(fDataSet);
   t_a->SetSystem(fSystem);
   t_a->SetRuns(fRunList, kFALSE);
}

TString KVDataSetAnalyser::SystemBatchName() const
{
   // Private method used by ExpandAutoBatchName to build name for current system
   // to be used in batch job name
   // Also used by KVDataAnalysisLauncher::SystemBatchName for batch job names
   // and for writing resources in .KVDataAnalysisGUIrc file

   KVString tmp = "Unknown";
   if (!fSystem) return tmp;
   return fSystem->GetBatchName();
}

TString KVDataSetAnalyser::ExpandAutoBatchName(const Char_t* format) const
{
   //Replace any 'special' symbols in "format" with their current values
   //
   //  $System  :  name of system to be analysed

   KVString tmp = KVDataAnalyser::ExpandAutoBatchName(format);
   tmp.ReplaceAll("$System", SystemBatchName());
   return tmp;
}

const Char_t* KVDataSetAnalyser::GetRecognisedAutoBatchNameKeywords() const
{
   static KVString tmp = KVDataAnalyser::GetRecognisedAutoBatchNameKeywords();
   tmp += ", $System";
   return tmp;
}

const KV2Body* KVDataSetAnalyser::GetKinematics() const
{
   // Return pointer to kinematics of analysed system if defined
   if (fSystem) return fSystem->GetKinematics();
   Error("GetKinematics", "No system defined for kinematic calculations");
   return nullptr;
}

Int_t KVDataSetAnalyser::GetRunNumberFromFileName(const Char_t* fileName)
{
   // Get the run number from the filename

   KVAvailableRunsFile* arf;
   arf = GetDataSet()->GetAvailableRunsFile(GetDataType().Data());
   return arf->IsRunFileName(fileName);
}

void KVDataSetAnalyser::AddJobDescriptionList(TList* l)
{
   // Retrieve a KVNameValueList called "JobDescriptionList" from
   // the TList created with KVDataAnalyser::AddJobDescriptionList(l).
   // The parameters in the list describe the properties of the
   // current job. The TList pointer could be, for example, the address of
   // the TSelector::fInput list used by PROOF.

   KVDataAnalyser::AddJobDescriptionList(l);
   KVNameValueList* jdl = (KVNameValueList*)l->FindObject("JobDescriptionList");

   jdl->SetValue("DataRepository", gDataRepository->GetName());
   jdl->SetValue("DataSet", fDataSet->GetName());
}

KVString KVDataSetAnalyser::GetRootDirectoryOfDataToAnalyse() const
{
   // Returns path to data to be analysed
   return gDataRepository->GetRootDirectory();
}
