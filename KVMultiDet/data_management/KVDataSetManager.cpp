/*
$Id: KVDataSetManager.cpp,v 1.17 2007/10/01 15:03:38 franklan Exp $
$Revision: 1.17 $
$Date: 2007/10/01 15:03:38 $
$Author: franklan $
*/

#include "KVBase.h"
#include "KVDataSetManager.h"
#include "KVDataRepositoryManager.h"
#include "KVString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "Riostream.h"
#include "TEnv.h"
#include "KVBase.h"
#include "TPluginManager.h"
#include "TError.h"

//macro converting octal filemode to decimal value
//to convert e.g. 664 (=u+rw, g+rw, o+r) use CHMODE(6,6,4)
#define CHMODE(u,g,o) ((u << 6) + (g << 3) + o)

using namespace std;

ClassImp(KVDataSetManager)

KVDataSetManager* gDataSetManager;

KVDataSetManager::KVDataSetManager()
{
   fNavailable = 0;
   fRepository = 0;
   fCacheAvailable = kFALSE;
   fMaxCacheTime = 0;
   fCacheFileName = "";
   fDataSets.SetOwner();
   fTasks.SetOwner();
}

KVDataSetManager::~KVDataSetManager()
{
}

Bool_t KVDataSetManager::Init(KVDataRepository* dr)
{
   //Initialisation of dataset manager for the repository 'dr'.
   //If dr=0x0 (default) then all known datasets are 'available', otherwise
   //we check availability of datasets based on data present in repository.
   //Initialise all possible data analysis tasks,
   //then set list of possible tasks for each available dataset.
   //
   //Returns kTRUE if all goes well.
   //For remote data repositories we return kFALSE if no datasets are available
   //(as the access to these is read-only)

   fRepository = dr;

   ReadUserGroups();

   if (!ReadDataSetList())
      return kFALSE;

   //use caching for dataset availability ?
   if (dr) {
      fCacheAvailable = gEnv->GetValue(Form("%s.DataRepository.CacheAvailable", dr->GetName()),
                                       kFALSE);
      fMaxCacheTime = (UInt_t)gEnv->GetValue(Form("%s.DataRepository.MaxCacheTime", dr->GetName()),
                                             0);
   }
   //name of cache file
   if (dr) fCacheFileName.Form("%s.available.datasets", dr->GetName());

   //check which datasets are available
   CheckAvailability();
   if (!GetNavailable() && dr && dr->IsRemote()) return kFALSE;

   if (!ReadTaskList())
      return kFALSE;

   //set available data analysis tasks for available datasets
   if (GetNavailable()) {
      for (Int_t i = 1; i <= GetNavailable(); i++)
         GetAvailableDataSet(i)->SetAnalysisTasks(&fTasks);
   }

   // stand-alone dataset manager: make it the default
   if (!dr) gDataSetManager  = this;

   return kTRUE;
}

void KVDataSetManager::ReadUserGroups()
{
   //Sets up list of user groups defining restricted access to certain datasets.
   //Definition of different user groups is given in $KVROOT/KVFiles/.kvrootrc

   //UserGroups env var contains whitespace-separated list of group names
   TString groups = gEnv->GetValue("UserGroup", "");
   if (groups == "") {
      //cout << "No value for UserGroup" << endl;
      return;
   }
   //split into array of group names
   TObjArray* toks = groups.Tokenize(' ');
   TObjString* group_name;
   TIter next_name(toks);
   fUserGroups.Clear();
   while ((group_name = (TObjString*) next_name())) {
      //for each group_name, the env var 'group_name.Users' contains a whitespace-separated list of user names
      //we store this string in the fUserGroups parameter list with parameter name 'group_name'
      TString users =
         gEnv->
         GetValue(Form("%s.Users", group_name->String().Data()), "");
      fUserGroups.SetValue(group_name->String().Data(), users);
   }
   delete toks;
}

Bool_t KVDataSetManager::ReadDataSetList()
{
   //Initialise list of all known datasets from informations in $KVROOT/KVFIles/.kvrootrc
   //(and user's .kvrootrc)

   KVString manip_list = gEnv->GetValue("DataSet", "");

   fDataSets.Clear();

   TObjArray* manips = manip_list.Tokenize(" ");
   TIter next(manips);
   TObjString* manip;
   while ((manip = (TObjString*)next())) {

      KVDataSet* ds = NewDataSet();
      ds->SetName(manip->GetString().Data());
      ds->SetTitle(gEnv->GetValue(Form("%s.DataSet.Title", manip->GetString().Data()), "Experimental dataset"));
      ds->SetDataPathSubdir(gEnv->GetValue(Form("%s.DataSet.RepositoryDir", manip->GetString().Data()), manip->GetString().Data()));
      ds->SetUserGroups(gEnv->GetValue(Form("%s.DataSet.UserGroup", manip->GetString().Data()), ""));
      ds->SetRepository(fRepository);
      fDataSets.Add(ds);

   }

   delete manips;

   return kTRUE;
}

Bool_t KVDataSetManager::ReadTaskList()
{
   //Initialise list of all known analysis tasks from informations in $KVROOT/KVFIles/.kvrootrc
   //(and user's .kvrootrc)

   KVString task_list = gEnv->GetValue("DataAnalysisTask", "");

   fTasks.Clear();

   task_list.Begin(" ");
   while (!task_list.End()) {

      KVDataAnalysisTask* dat = new KVDataAnalysisTask;
      TString name = task_list.Next();
      dat->SetName(name.Data());
      dat->SetTitle(gEnv->GetValue(Form("%s.DataAnalysisTask.Title", name.Data()), ""));
      dat->SetPrereq(gEnv->GetValue(Form("%s.DataAnalysisTask.Prereq", name.Data()), ""));
      dat->SetDataAnalyser(gEnv->GetValue(Form("%s.DataAnalysisTask.Analyser", name.Data()), "KVDataAnalyser"));
      dat->SetWithUserClass(gEnv->GetValue(Form("%s.DataAnalysisTask.UserClass", name.Data()), kFALSE));
      dat->SetUserBaseClass(gEnv->GetValue(Form("%s.DataAnalysisTask.UserClass.Base", name.Data()), ""));
      dat->SetStatusUpdateInterval(gEnv->GetValue(Form("%s.DataAnalysisTask.StatusUpdateInterval", name.Data()), 1000));
      fTasks.Add(dat);

   }

   return kTRUE;
}

void KVDataSetManager::Print(Option_t* opt) const
{
   //Print list of datasets
   //If opt="" (default) all datasets are shown with full information
   //if opt="available" only available datasets are shown, each with a number which can
   //be used with GetAvailableDataSet(Int_t) in order to retrieve the corresponding dataset.

   TString Sopt(opt);
   Sopt.ToUpper();
   if (Sopt.BeginsWith("AVAIL")) {
      if (!fNavailable) {
         cout << "                    *** No available datasets ***" <<
              endl;
         return;
      }
      else {
         for (int i = 1; i <= fNavailable; i++) {
            KVDataSet* ds = GetAvailableDataSet(i);
            cout << "\t" << i << ". " << ds->GetTitle() << endl;
         }
      }
      return;
   }
   if (fDataSets.GetSize()) {
      TIter next(&fDataSets);
      KVDataSet* ds;
      while ((ds = (KVDataSet*) next()))
         ds->ls();
   }
}

void KVDataSetManager::CheckAvailability()
{
   //Check availability of datasets in repository associated to this data set manager
   //
   //If caching is activated for the parent repository, i.e. if
   //
   //    [repository name].DataRepository.CacheAvailable:    yes
   //
   //then instead of directly checking the existence of the directories for each dataset,
   //we use the cached information written in the file
   //KVBase::WorkingRepository()/[repository name].available.datasets
   //unless (1) it doesn't exist, or (2) the file is older than the maximum
   //cache time (in seconds) defined by
   //
   //   [repository name].DataRepository.MaxCacheSeconds:
   //
   //In either of these 2 cases, we check the existence of the directories and update/
   //create the cache file.
   //
   //If the repository appears to be empty (perhaps because we are using a remote access
   //protocol to check it, and the protocol has some problems...), then as a last resort we
   //we will use the cache if it exists, whatever its age.

   if (fCacheAvailable) {
      //caching of dataset availability is activated
      if (CheckCacheStatus()) {
         //cache file exists and is not out of date
         if (ReadAvailableDatasetsFile()) return;
      }
   }

   // print (repository-dependent) warning/informational message
   if (fRepository) fRepository->PrintAvailableDatasetsUpdateWarning();

   //open temporary file
   ofstream tmp_file;
   TString tmp_file_path = fCacheFileName;
   KVBase::OpenTempFile(tmp_file_path, tmp_file);

   fNavailable = 0;
   if (fDataSets.GetSize()) {
      TIter next(&fDataSets);
      KVDataSet* ds;
      while ((ds = (KVDataSet*) next())) {
         //The results of this check are written in $KVROOT/KVFiles/[repository name].available.datasets
         //This file may be read by KVRemoteDataSetManager::CheckAvailability when this
         //data repository is accessed as a remote data repository from a remote machine.
         //In this case we do not want the identity of the user to influence the contents of the file.
         //Therefore even for 'unavailable' datasets we write the available datatypes (if any)
         //in the file.
         tmp_file << ds->GetName() << " : ";
         ds->CheckAvailable();
         tmp_file << ds->GetAvailableDataTypes() << endl;
         if (ds->IsAvailable()) {
            fNavailable++;
         }
      }

      //close temp file
      tmp_file.close();
      //if datasets are found, then we copy the temporary file to KVFiles directory,
      //overwriting any previous version. if no datasets were found, we try the cache
      //file (if it exists)
      if (fNavailable && fRepository) { //if no repository is associated, no need to keep file
         TString runlist = KVBase::GetWORKDIRFilePath(fCacheFileName.Data());
         gSystem->CopyFile(tmp_file_path, runlist, kTRUE);
         //set access permissions to 664
         gSystem->Chmod(runlist.Data(), CHMODE(6, 6, 4));
      }

      //delete temp file
      gSystem->Unlink(tmp_file_path);

      if (!fNavailable) {
         //no datasets found when checking file system ?
         //can we rely on the cache file ?
         ReadAvailableDatasetsFile();
      }
      else {
         //now set up array of available datasets' indices
         fIndex.clear();
         next.Reset();
         Int_t j(0);
         while ((ds = (KVDataSet*) next())) {
            if (ds->IsAvailable()) {
               fIndex.push_back(j);
            }
            j++;
         }
      }
   }
}

//_____________________________________________________________________________//

KVDataSet* KVDataSetManager::GetDataSet(Int_t index) const
{
   //Return pointer to DataSet using index in list of all datasets, index>=0
   if (fDataSets.GetSize() && index < fDataSets.GetSize())
      return (KVDataSet*) fDataSets.At(index);
   return 0;
}

KVDataSet* KVDataSetManager::GetDataSet(const Char_t* name)
{
   //Return pointer to DataSet using name
   return (KVDataSet*) fDataSets.FindObjectByName(name);
}

KVDataSet* KVDataSetManager::GetAvailableDataSet(Int_t index) const
{
   //Return pointer to available DataSet using index of available datasets
   //Note this index begins at 1, and corresponds to the number printed next to the dataset
   //when Print("available") is called
   if (fNavailable && index && index <= fNavailable)
      return GetDataSet(fIndex[index - 1]);
   return 0;
}

KVDataAnalysisTask* KVDataSetManager::GetTask(const Char_t* name)
{
   //Return pointer to named data analysis task
   return (KVDataAnalysisTask*) fTasks.FindObjectByName(name);
}

Bool_t KVDataSetManager::CheckUser(const Char_t* groupname,
                                   const Char_t* username)
{
   //Check in list of groups fUserGroups if the user name 'username' is part of the group 'groupname'.
   //If 'username' is not given (default) we use current user info (gSystem->GetUserInof()->fUser).

   TString Username = strcmp(username,
                             "") ? username : gSystem->GetUserInfo()->
                      fUser.Data();

   if (fUserGroups.HasParameter(groupname)) {
      if (fUserGroups.GetTStringValue(groupname).Contains(Username.Data()))
         return kTRUE;
   }
   return kFALSE;
}

KVDataSet* KVDataSetManager::NewDataSet()
{
   //Creates and returns pointer to new data set object
   return (new KVDataSet);
}

void KVDataSetManager::Update()
{
   //Called when the physical state of the repository has changed i.e. a subdirectory for
   //a new dataset or datatype has been added or removed. We update the available datatsets,
   //datatypes and analysis tasks.

   //check which datasets are available
   CheckAvailability();

   //set available data analysis tasks for available datasets
   if (GetNavailable()) {
      for (Int_t i = 1; i <= GetNavailable(); i++)
         GetAvailableDataSet(i)->SetAnalysisTasks(&fTasks);
   }
}

Bool_t KVDataSetManager::OpenAvailableDatasetsFile()
{
   //Opens file KVBase::WorkingDirectory()/[repository name].available.datasets
   //containing cached info on available datasets and
   //associated subdirectories in data repository.
   //Opens file for reading, & if all goes well returns kTRUE.
   //Returns kFALSE in case of problems.

   return KVBase::SearchAndOpenKVFile(KVBase::GetWORKDIRFilePath(fCacheFileName), fDatasets);
}

Bool_t KVDataSetManager::ReadAvailableDatasetsFile()
{
   //Opens and reads file containing cached info on available datasets, and sets
   //the availability of the concerned datasets.
   //Returns kTRUE if all goes well.
   //Returns kFALSE if no cache exists or if file cannot be opened.
   if (OpenAvailableDatasetsFile()) {
      Info("ReadAvailableDataSetsFile",
           "Reading cached information in file %s", fCacheFileName.Data());
      //read file
      TString line;
      line.ReadLine(fDatasets);
      while (fDatasets.good()) {

         TObjArray* toks = line.Tokenize(": ,");

         //first entry is dataset name
         TString datasetname = ((TObjString*) toks->At(0))->String();
         KVDataSet* dataset = GetDataSet(datasetname.Data());

         if (dataset) { //check dataset is known to local version of KaliVeda
            //in case of remote repository, there may be datasets in the remote repository which are not defined here
            if (toks->GetEntries() > 1 && dataset->CheckUserCanAccess()) {
               //AVAILABLE DATASET
               dataset->SetAvailable();
               fNavailable++;
               for (int i = 1; i < toks->GetEntries(); i++) {
                  //each following entry is a subdirectory name
                  dataset->AddAvailableDataType(((TObjString*) toks->At(i))->String().
                                                Data());
               }
            }
            else {
               //UNAVAILABLE DATASET (no subdirs)
               dataset->SetAvailable(kFALSE);
            }
         }

         delete toks;
         line.ReadLine(fDatasets);
      }

      //close file
      fDatasets.close();
      fDatasets.clear();

      if (fNavailable) {
         TIter next(&fDataSets);
         //now set up array of available datasets' indices
         fIndex.clear();
         Int_t j(0);
         KVDataSet* ds;
         while ((ds = (KVDataSet*) next())) {
            if (ds->IsAvailable()) {
               fIndex.push_back(j);
            }
            j++;
         }
      }
      //all is OK
      return kTRUE;
   }
   //we could not find/open the cache file
   return kFALSE;
}

//__________________________________________________________________________________//

Bool_t KVDataSetManager::CheckCacheStatus()
{
   //We check the status of the available datasets cache file.
   //We return kTRUE if the file exists & was last modified
   //less than fMaxCacheTime seconds ago.

   TString fullpath;
   Info("KVDataSetManager::CheckCacheStatus", "Checking for available datasets cache file...");
   if (KVBase::SearchKVFile(KVBase::GetWORKDIRFilePath(fCacheFileName), fullpath)) {

      // file exists - how old is it ?
      FileStat_t file_info;
      gSystem->GetPathInfo(fullpath.Data(), file_info);
      TDatime file_date(file_info.fMtime);
      TDatime now;
      UInt_t file_age = now.Convert() - file_date.Convert();
      Info("KVDataSetManager::CheckCacheStatus", "...file found. It is %u seconds old", file_age);
      if (file_age < fMaxCacheTime) {
         Info("KVDataSetManager::CheckCacheStatus", "Using cached file");
         return kTRUE;
      }
      else
         Info("KVDataSetManager::CheckCacheStatus", "File is too old (max time=%u). Update will be performed.", fMaxCacheTime);
   }
   else
      Info("KVDataSetManager::CheckCacheStatus", "...no file found");
   return kFALSE;
}

KVDataAnalysisTask* KVDataSetManager::GetAnalysisTaskAny(const Char_t* keywords) const
{
   // This method returns a pointer to the analysis task whose description (title) contains
   // all of the whitespace-separated keywords (which may be regular expressions)
   // given in the string "keywords". The comparison is case-insensitive.

   //case-insensitive search for matches in list of all analysis tasks, based on 'title' attribute
   return (KVDataAnalysisTask*)GetAnalysisTaskList()->FindObjectAny("title", keywords, kTRUE, kFALSE);
}
