/*
$Id: KVDataSet.cpp,v 1.41 2009/03/11 14:22:41 franklan Exp $
$Revision: 1.41 $
$Date: 2009/03/11 14:22:41 $
$Author: franklan $
*/

#include "TMethodCall.h"
#include "KVDataSet.h"
#include "KVDataRepository.h"
#include "KVDataRepositoryManager.h"
#include "KVDataSetManager.h"
#include "KVDataBase.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVDBSystem.h"
#include "KVDBRun.h"
#include "TEnv.h"
#include "KVAvailableRunsFile.h"
#include "KVNumberList.h"
#include "TPluginManager.h"
#include "TClass.h"
#include "KVRunFile.h"

using namespace std;

ClassImp(KVDataSet)
/////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataSet</h2>
<h4>Describes a set of experimental data which may be analysed with KaliVeda</h4>
<p>A dataset brings together all the elements necessary for the
analysis of data taken during an experiment or campaign of experiments
using the same experimental configuration. These are:
</p>
<ul>
  <li>the files containing the data in its raw state</li>
  <li>files containing partially and/or completely calibrated data</li>
  <li>a description of the experimental set-up (detectors, geometry, etc.)</li>
  <li>a database describing the different runs &amp; reaction systems
studied, detector parameters (status, calibration parameters, etc.)</li>
</ul>

<h3><a name="init"></a>I. Initialisation</h3>

<p>Different datasets are listed in the configuration file <span
 style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>. This file is read by
 <a style="font-family: monospace;" href="KVDataSetManager.html#KVDataSetManager:Init">KVDataSetManager::Init()</a>,
which sets up and handles all the datasets for a given data repository
(see <a href="KVDataRepository.html"><span
 style="font-family: monospace;">KVDataRepository</span></a>).
The syntax for adding a dataset to this list is described in the file.
</p>
<p>After creation, the availability of this dataset for analysis is
decided based on whether data files associated with it are present or
not in the data repository, and, optionally, whether the user's name
appears in the associated user group (see <a
 style="font-family: monospace;" href="#KVDataSet:CheckAvailable">KVDataSet::CheckAvailable()</a>).
Datasets which are available for analysis by the user have <span
 style="font-family: monospace;"><a href="#KVDataSet:IsAvailable">IsAvailable()</a>==kTRUE</span>.
</p>
<p>All possible <a href="KVDataAnalysisTask.html">data analysis tasks</a>
for this dataset are then determined based on the available types of
data in the repository (see <a style="font-family: monospace;"
 href="#KVDataSet:SetAnalysisTasks">SetAnalysisTasks()</a>) and the
default list of known analysis tasks (see <a
 style="font-family: monospace;"
 href="KVDataSetManager.html#KVDataSetManager:ReadTaskList">KVDataSetManager::ReadTaskList()</a>).
Data analysis tasks can be chosen &amp; performed for any available
dataset by using a <a style="font-family: monospace;"
 href="KVDataAnalyser.html">KVDataAnalyser</a>.
</p>

<h3><a name="what"></a>II. What's In A Dataset ?</h3>

<h4>a. Experimental set-up: multidetector array</h4>
<p>The experimental set-up for each dataset is handled by a child class
of <a style="font-family: monospace;" href="KVMultiDetArray.html">KVMultiDetArray</a>,
which can describe anything from a single detector
to a 4&#960; (pi)-multidetector of charged particles such as INDRA. Each
dataset is uniquely associated to its experimental set-up by defining a
<a href="http://root.cern.ch/root/html/TPluginManager.html">plugin</a>
of the <span style="font-family: monospace;">KVMultiDetArray</span>
class:
</p>
<pre>+Plugin.KVMultiDetArray: name_of_dataset  name_of_class  name_of_plugin_library  constructor_to_call</pre>
<p>See file <span style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>
for examples concerning the default datasets.
There you will also see that the multidetector class can be further
"tweaked" by defining dataset-dependent plugins for the <a
 href="KVIDTelescope.html"><span style="font-family: monospace;">KVIDTelescope</span></a>
class (see <a
 href="KVMultiDetArray.html#KVMultiDetArray:GetIDTelescopes"><span
 style="font-family: monospace;">KVMultiDetArray::GetIDTelescopes</span></a>).
</p>
<p>The experimental set-up for the dataset can be created when
necessary by using <a href="#KVDataSet:BuildMultiDetector"><span
 style="font-family: monospace;">KVDataSet::BuildMultiDetector()</span></a>.
</p>

<h4>b. Runs, systems and calibration parameter database</h4>

<p>A child class of <a href="KVDataBase.html"><span
 style="font-family: monospace;">KVDataBase</span></a> handles all
stored information on the different experimental runs, reaction systems
studied, run-dependent detector characteristics, calibration
parameters, etc. The link between dataset and database is defined by a <a
 href="http://root.cern.ch/root/html/TPluginManager.html">plugin</a>:
</p>

<pre>+Plugin.KVDataBase: name_of_dataset  name_of_class  name_of_plugin_library  constructor_to_call</pre>

<p>See file <span style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>
for examples concerning the default datasets.</p>

<p>Such databases are constructed from a number of different source
files (runlist, system list, etc.) which are kept in the directory <span
 style="font-family: monospace;">$KVROOT/KVFiles/name_of_dataset</span>
(string returned by <a style="font-family: monospace;"
 href="#KVDataSet:GetDataSetDir">GetDataSetDir()</a>).
The database object, once constructed from these files (see <a
 href="KVDataBase.html#KVDataBase:Build"><span
 style="font-family: monospace;">KVDataBase::Build()</span></a>), is
saved in a ROOT file in the same directory. The default name for this
file is given by the environment variable
</p>
<pre>DataSet.DatabaseFile:   DataBase.root</pre>
<p>in <span style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>.
There you will also see examples of definitions of dataset-specific
source file names and formats.
</p>
<p>To acces the database associated with the dataset, use <a
 style="font-family: monospace;" href="#KVDataSet:GetDataBase">GetDataBase()</a>.<br>
</p>

<h4>c. Data files</h4>

<p>All files of experimental data belonging to this dataset are kept
under the <span style="font-family: monospace;">data_repository_subdirectory</span>
defined in the configuration of the dataset. The name of this
directory is returned by <a style="font-family: monospace;"
 href="#KVDataSet:GetDatapathSubdir">GetDatapathSubdir()</a>. There are
four different types of data, each of which is kept in a different
subdirectory of the dataset's repository directory:
</p>
<ul>
      <li><span style="font-family: monospace;">raw/</span>&nbsp;&nbsp;&nbsp;
The raw data written during the experiment by the experimental
acquisition system
      </li>
      <li><span style="font-family: monospace;">recon/</span>&nbsp;&nbsp;&nbsp;
Reconstructed data in which the (uncalibrated) energy losses measured
in different detectors of the set-up have been associated into
(unidentified) particles.
      </li>
      <li><span style="font-family: monospace;">ident/</span>&nbsp;&nbsp;&nbsp;
Reconstructed data of the previous type to which some or all of the
necessary identification and calibration algorithms have been applied
in order to identify and calibrate the previously reconstructed
particles.
      </li>
      <li><span style="font-family: monospace;">root/</span>&nbsp;&nbsp;&nbsp;
Fully-identified and calibrated reconstructed events resulting from the
application of coherency algorithms to the last-produced <span
 style="font-family: monospace;">ident</span> data files.<br>
      </li>
</ul>
<p>It should be noted that the last 3 types of data are obligatorily
stored in <a href="http://root.cern.ch/root/html/TTree.html">TTrees</a>
written in ROOT files. They also have exactly the same structure, i.e.
they are constituted of <a href="KVReconstructedEvent.html">events</a>
containing one or more <a href="KVReconstructedNucleus.html">particles</a>.
The algorithm for reconstructing raw data, i.e. for creating <span
 style="font-family: monospace;">recon</span> files from <span
 style="font-family: monospace;">raw</span> files, is described in <a
 href="KVReconstructedEvent.html#KVReconstructedEvent:ReconstructEvent"><span
 style="font-family: monospace;">KVReconstructedEvent::ReconstructEvent</span></a>.
Both this process, and also <span style="font-family: monospace;">recon</span>&#8594;<span
 style="font-family: monospace;">ident</span>, <span
 style="font-family: monospace;">ident</span>&#8594;<span
 style="font-family: monospace;">root</span> are among the data
analysis tasks predefined (for INDRA data) in <span
 style="font-family: monospace;">$KVROOT/KVFiles/.kvrootrc</span>.
</p>

<p>This classification of data is meant to reflect the chronology of a
typical process of data reduction &amp; analysis. The first step is to
create <span style="font-family: monospace;">recon</span> files from
the raw data using the <span style="font-family: monospace;">raw</span>&#8594;<span
 style="font-family: monospace;">recon</span> analysis task. This can
be performed as soon as the description of the experimental set-up (<a
 href="KVMultiDetArray.html"><span style="font-family: monospace;">KVMultiDetArray</span></a>
class) has been implemented and the bare minimum of source files for
setting up the database (runlist) are available. The advantage of
working with recon files is that (i) the event reconstruction is
performed once, in the <span style="font-family: monospace;">raw</span>&#8594;<span
 style="font-family: monospace;">recon </span>step, so subsequent
analysis is that much faster; (ii) the data is stored in TTrees in ROOT
files which, unless the acquisition system is based on ROOT, means that
they can be analysed much more efficiently and on more platforms than
the original raw files.
</p>
<!-- */
// --> END_HTML
/////////////////////////////////////////////////
KVDataSet* gDataSet;

KVDataSet::KVDataSet()
{
   //Default constructor
   fDBase = 0;
   fRepository = 0;
   fDataBase = 0;
}

KVDataSet::~KVDataSet()
{
   if (fDBase) {
      delete fDBase;
      fDBase = 0;
   }
   fTasks.Delete();
}

KVAvailableRunsFile* KVDataSet::GetAvailableRunsFile(const Char_t* type) const
{
   //Returns available runs file object for given data 'type' (="raw", "recon", "ident", "root")
   //Object will be created and added to internal list if it does not exist

   if (!fRepository) return nullptr;
   KVAvailableRunsFile* avrf =
      (KVAvailableRunsFile*) fAvailableRuns.FindObjectByName(type);
   if (!avrf) {
      avrf = fRepository->NewAvailableRunsFile(type, this);
      fAvailableRuns.Add(avrf);
   }
   return avrf;
}

const Char_t* KVDataSet::GetDBFileName() const
{
   //Returns name of file containing database for dataset.
   //If this is not set explicitly with SetDBFileName(), the default value defined by DataSet.DatabaseFile
   //in $KVROOT/KVFiles/.kvrootrc will be returned.

   if (fDBFileName != "")
      return fDBFileName.Data();
   return gEnv->GetValue("DataSet.DatabaseFile", "");
}

const Char_t* KVDataSet::GetDBName() const
{
   //Returns name of database object in database file.
   //If this is not set explicitly with SetDBName(), we use the name of the dataset by default

   if (fDBName != "")
      return fDBName.Data();
   return GetName();
}

//_______________________________________________________________//

void KVDataSet::OpenDBFile(const Char_t* full_path_to_dbfile) const
{
   //Open the database from a file on disk.

   if (fDBase) {
      delete fDBase;
      fDBase = 0;
   }

   TDirectory* work_dir = gDirectory;   //keep pointer to current directory
   fDBase = new TFile(full_path_to_dbfile, "READ");

   if (fDBase->IsOpen()) {
      fDataBase = (KVDataBase*) fDBase->Get(GetDBName());
      if (!fDataBase) {
         Error("OpenDBFile", "%s not found in file %s", GetDBName(),
               GetDBFileName());
      } else {
         fDataBase->ReadObjects(fDBase);   // read any associated objects
      }
      work_dir->cd();           //back to initial working directory
   }
}

//_______________________________________________________________//

const Char_t* KVDataSet::GetFullPathToDB() const
{
   // Returns full path to file where database is written on disk

   TString dbfile = GetDBFileName();
   static TString dbfile_fullpath;
   TString tmp;

   AssignAndDelete(tmp, gSystem->ConcatFileName(GetDATABASEFilePath(), GetName()));
   AssignAndDelete(dbfile_fullpath, gSystem->ConcatFileName(tmp.Data(), dbfile.Data()));
   return dbfile_fullpath.Data();
}

void KVDataSet::SetDataSetSpecificTaskParameters(KVDataAnalysisTask* t) const
{
   //PROTECTED METHOD
   //Called by KVDataSet::SetAnalysisTasks
   //Check environment variables (i.e. .kvrootrc) to see if the task parameters
   //have been "tweaked" for the dataset.

   KVString envar = GetDataSetEnv(Form("%s.DataAnalysisTask.Analyser", t->GetName()));
   if (envar != "") t->SetDataAnalyser(envar);
   envar = GetDataSetEnv(Form("%s.DataAnalysisTask.UserClass.Base", t->GetName()));
   if (envar != "") t->SetUserBaseClass(envar);
   envar = GetDataSetEnv(Form("%s.DataAnalysisTask.Prereq", t->GetName()));
   if (envar != "") t->SetPrereq(envar);
   envar = GetDataSetEnv(Form("%s.DataAnalysisTask.UserClass.ExtraACliCIncludes", t->GetName()));
   if (envar != "") t->SetExtraAClicIncludes(envar);
   Int_t nev = (Int_t)GetDataSetEnv(Form("%s.DataAnalysisTask.StatusUpdateInterval", t->GetName()), 0.0);
   if (nev > 0) t->SetStatusUpdateInterval(nev);
}

//_______________________________________________________________//

void KVDataSet::SaveDataBase() const
{
   // Write the database to disk (ROOT file).
   // It will be written in the directory
   //   $KVROOT/db/[dataset name]
   // If the directory does not exist, it will be created. Permissions are set to 775 (rwxrwxr-x).
   //
   // # Default name of database file containing informations on runs, systems, calibration parameters etc.
   // DataSet.DatabaseFile:        DataBase.root
   // # Default name of database object in file
   // DataSet.DatabaseName:        DataBase

   TString dbfile_fullpath = GetFullPathToDB();
   TString tmp = gSystem->DirName(dbfile_fullpath.Data());  //full path to directory $KVROOT/db/[dataset name]

   if (gSystem->AccessPathName(tmp.Data())) {   // directory $KVROOT/db/[dataset name] does not exist

      if (gSystem->mkdir(tmp.Data()) == -1) {  // problem creating $KVROOT/db/[dataset name]

         TString tmp2 = gSystem->DirName(tmp.Data());// full path to directory $KVROOT/db

         if (gSystem->AccessPathName(tmp2.Data())) {   // directory $KVROOT/db does not exist

            if (gSystem->mkdir(tmp2.Data()) == -1) { // problem creating $KVROOT/db
               Error("SaveDataBase", "Cannot create directory %s required to save database",
                     tmp2.Data());
               return;
            }
            gSystem->Chmod(tmp2.Data(), 0775);
         } else {
            Error("SaveDataBase", "Cannot create directory %s required to save database, even though %s exists: check disk space ?",
                  tmp.Data(), tmp2.Data());
            return;
         }
         //try again
         if (gSystem->mkdir(tmp.Data()) == -1) {
            Error("SaveDataBase", "Cannot create directory %s required to save database",
                  tmp.Data());
            return;
         } else {
            gSystem->Chmod(tmp.Data(), 0775);
         }
      } else {
         gSystem->Chmod(tmp.Data(), 0775);
      }
   }

   WriteDBFile(dbfile_fullpath.Data());
}

//_______________________________________________________________//

void KVDataSet::WriteDBFile(const Char_t* full_path_to_dbfile) const
{
   //PRIVATE METHOD
   //Write the database to disk.
   //Set permissions to rw for user & group

   TDirectory* work_dir = gDirectory;   //keep pointer to current directory
   if (fDBase) {
      delete fDBase;
      fDBase = 0;
   }
   if (!fDataBase) {
      Error("WriteDBFile", "Database has not been built");
      return;
   }
   fDBase = new TFile(full_path_to_dbfile, "recreate");
   fDBase->cd();                //set as current directory (maybe not necessary)
   fDataBase->Write(GetDBName());    //write database to file with given name
   fDataBase->WriteObjects(fDBase);   //write any associated objects
   fDBase->Write();        // write file header etc.
   fDBase->Close();         // close file
   gSystem->Chmod(full_path_to_dbfile, 0664); // set permissions to rw-rw-r--
   work_dir->cd();              //back to initial working directory
}

KVDataBase* KVDataSet::GetDataBase(Option_t* opt) const
{
   //Returns pointer to database associated with this dataset.
   //Opens, updates or creates database file if necessary
   //(the database is automatically rebuilt if the source files are
   //more recent than the last database file).
   //
   //If opt="update":
   // close and delete database if already open
   // regenerate database from source files
   //Use this option to force the regeneration of the database.

   TString _opt(opt);
   _opt.ToUpper();
   if (_opt == "UPDATE") {
      OpenDataBase(_opt.Data());
   } else {
      OpenDataBase();
   }
   return fDataBase;
}

void KVDataSet::OpenDataBase(Option_t* opt) const
{
   //Open the database for this dataset.
   //If the database does not exist or is older than the source files
   //the database is automatically rebuilt
   //(see DataBaseNeedUpdate()).
   //Use opt="UPDATE" to force rebuilding of the database.
   //
   //First, we look in to see if the database file exists
   //(if no database file name given, use default name for database file defined in
   //.rootrc config files).
   //If so, we open the database contained in the file, after first loading the required plugin
   //library if needed.
   //
   //The name of the dataset must correspond to the name of one of the Plugin.KVDataBase
   //plugins defined in the .rootrc configuration files
   //
   //WARNING: if the database needs to be (re)built, we set gDataSet to
   //point to this dataset in case it was not already done,
   //as in order to (re)build the database it may be necessary for
   //gDataSet to point to the current dataset.

   Bool_t is_glob_db = kFALSE;
   //if option="update" or database out of date or does not exist, (re)build the database
   if ((!strcmp(opt, "UPDATE")) || DataBaseNeedsUpdate()) {
      //check if it is the currently active database (gDataBase),
      //in which case we must 'cd()' to it after rebuilding
      Info("OpenDataBase", "Updating database file");
      is_glob_db = (fDataBase == gDataBase);
      if (fDataBase) {
         delete fDataBase;
         fDataBase = 0;
      }
      if (fDBase) {
         delete fDBase;
         fDBase = 0;
      }
      // make sure gDataSet is set & points to us
      gDataSet = const_cast<KVDataSet*>(this);
      fDataBase = KVDataBase::MakeDataBase(GetDBName(), GetDataSetDir());
      if (!fDataBase) {
         // no database defined for dataset
         Info("OpenDataBase", "No database defined for dataset");
         return;
      }
      SaveDataBase();
      if (fDataBase && is_glob_db) fDataBase->cd();
   } else if (!fDataBase) {
      // if database is not in memory at this point, we need to
      // open the database file and read in the database

      //load plugin for database
      if (!LoadPlugin("KVDataBase", GetName())) {
         Error("GetDataBase", "Cannot load required plugin library");
         return;
      }
      //look for database file in dataset subdirectory
      TString dbfile_fullpath = GetFullPathToDB();
      //open database file
      OpenDBFile(dbfile_fullpath.Data());
   }
}

//___________________________________________________________________________________________________________________

void KVDataSet::ls(Option_t*) const
{
   //Print dataset information
   cout << "Dataset name=" << GetName() << " (" << GetTitle() << ")";
   if (IsAvailable()) {
      cout << "  [ AVAILABLE: ";
      cout << fDatatypes.Data();
      cout << "]";
   } else
      cout << "  [UNAVAILABLE]";
   cout << endl;
}

void KVDataSet::Print(Option_t* opt) const
{
   //Print dataset information
   //If option string contains "tasks", print numbered list of tasks that can be performed
   //If option string contains "data", print list of available data types

   TString Sopt(opt);
   Sopt.ToUpper();
   if (Sopt.Contains("TASK")) {
      if (!GetNtasks()) {
         cout << "                    *** No available analysis tasks ***"
              << endl;
         return;
      } else {
         for (int i = 1; i <= GetNtasks(); i++) {
            KVDataAnalysisTask* dat = GetAnalysisTask(i);
            cout << "\t" << i << ". " << dat->GetTitle() << endl;
         }
      }
      cout << endl;
   } else if (Sopt.Contains("DATA")) {
      cout << "Available data types: " << fDatatypes.Data() << endl;
   } else {
      ls(opt);
   }
}

void KVDataSet::CheckAvailable()
{
   //Check if this data set is physically present and available for analysis.
   //In other words we check if the value of GetDatapathSubdir() is a subdirectory
   //of the given data repository
   //If so, we proceed to check for the existence of any of the datatypes defined in
   //
   //KVDataSet.DataTypes:
   //
   //by checking for the associated sudirectories defined in the corresponding variables:
   //
   //KVDataSet.DataType.Subdir.[type]:
   //
   //If none of them exists, the dataset will be reset to 'unavailable'
   //Otherwise the corresponding flags are set.
   //
   //Note that if SetUserGroups has been called with a list of groups allowed to read this data,
   //the current user's name (gSystem->GetUserInfo()->fUser) will be used to check if the
   //dataset is available. The user name must appear in the group defined by SetUserGroups.

   if (!fRepository)   // for a stand-alone KVDataSetManager not linked to a KVDataRepository,
      SetAvailable();   // all known datasets are 'available'
   else
      SetAvailable(fRepository->CheckSubdirExists(GetDataPathSubdir()));
   if (!IsAvailable())
      return;
   //check subdirectories
   KVString data_types = GetDataSetEnv("KVDataSet.DataTypes", "");
   if (data_types == "") {
      Warning("CheckAvailable", "No datatypes defined for this dataset: %s\nCheck value of KVDataSet.DataTypes or %s.KVDataSet.DataTypes",
              GetName(), GetName());
      SetAvailable(kFALSE);
   }
   fDatatypes = "";
   // loop over data types
   data_types.Begin(" ");
   while (!data_types.End()) {
      KVString type = data_types.Next(kTRUE);
      if (!fRepository ||
            (fRepository && fRepository->CheckSubdirExists(GetDataPathSubdir(), GetDataTypeSubdir(type.Data())))
         ) {
         AddAvailableDataType(type.Data());
      }
   }
   //check at least one datatype exists
   SetAvailable(fDatatypes != "");
   //check user name against allowed groups
   if (!CheckUserCanAccess()) {
      SetAvailable(kFALSE);
      return;
   }
}

void KVDataSet::AddAvailableDataType(const Char_t* type)
{
   if (fDatatypes != "") fDatatypes += " ";
   KVString _type = type;
   _type.Remove(TString::kBoth, ' '); //strip whitespace
   fDatatypes += _type;
}

void KVDataSet::SetAnalysisTasks(const KVSeqCollection* task_list)
{
   // Add to fTasks list any data analysis task in list 'task_list'
   // whose pre-requisite datatype is present for this dataset.
   // Any dataset-specific "tweaking" of the task (including the prerequisite datatype) is done here.

   TString availables = gEnv->GetValue(Form("%s.DataAnalysisTask", GetName()), "");
   fTasks.Delete();
   TIter nxt(task_list);
   KVDataAnalysisTask* dat;
   while ((dat = (KVDataAnalysisTask*) nxt())) {
      //make new copy of default analysis task
      if (availables == "" || availables.Contains(dat->GetName())) {
         KVDataAnalysisTask* new_task = new KVDataAnalysisTask(*dat);
         //check if any dataset-specific parameters need to be changed
         SetDataSetSpecificTaskParameters(new_task);
         if (HasDataType(new_task->GetPrereq())) {
            fTasks.Add(new_task);
         } else
            delete new_task;
      }
   }
}

Int_t KVDataSet::GetNtasks() const
{
   //Returns the number of tasks associated to dataset which are compatible
   //with the available data

   return fTasks.GetSize();
}

KVDataAnalysisTask* KVDataSet::GetAnalysisTask(Int_t k) const
{
   //Return kth analysis task in list of available tasks.
   //k=[1, GetNtasks()] and corresponds to the number shown next to the title of the task when
   //KVDataSet::Print("tasks") is called
   return (KVDataAnalysisTask*) fTasks.At(k - 1);
}

TList* KVDataSet::GetListOfAvailableSystems(const Char_t* datatype, KVDBSystem* systol)
{
   //Create and fill a list of available systems for this dataset and the given datatype
   //
   //This uses the database associated to the dataset.
   //USER MUST DELETE THE LIST AFTER USE.
   //
   //For each system in the list we set the number of available runs : this number
   //can be retrieved with KVDBSystem::GetNumberRuns()
   //
   //If systol!=0 then in fact the list contains a list of runs for the given system which are available.
   //
   //If no systems are defined for the dataset then we return a list of available runs
   //for the given datatype

   //open the available runs file for the data type
   if (!GetAvailableRunsFile(datatype)) {
      Error("GetListOfAvailableSystems(const Char_t*)",
            "No available runs file for type %s", datatype);
      return 0;
   }
   return GetAvailableRunsFile(datatype)->GetListOfAvailableSystems(systol);
}

TList* KVDataSet::GetListOfAvailableSystems(KVDataAnalysisTask* datan, KVDBSystem* systol)
{
   //Create and fill a list of available systems for this dataset and the prerequisite
   //datatype for the given data analysis task.
   //This uses the database associated to the dataset.
   //USER MUST DELETE THE LIST AFTER USE.
   //
   //For each system in the list we set the number of available runs : this number
   //can be retrieved with KVDBSystem::GetNumberRuns()
   //
   //If systol!=0 then in fact the list contains a list of runs for the given system which are available.
   //
   //If no systems are defined for the dataset then we return a list of available runs
   //for the given datatype

   return GetListOfAvailableSystems(datan->GetPrereq(), systol);
}

void KVDataSet::SetName(const char* name)
{
   // Set name of dataset.
   // Also sets path to directory containing database informations
   // for this dataset, i.e. list of runs, systems, calibration files etc.
   //     By default, just the name of the dataset is used, i.e.
   //         [DATADIR]/name
   //     (where DATADIR = path given by KVBase::GetDATADIRFilePath())
   //     However, if the variable
   //          [name].DataSet.Directory:   [path]
   //     has been set, the value of [path] will be used.
   //     If [path] is an absolute path name, it will be used as such.
   //     If [path] is an incomplete or relative path, it will be prepended
   //      with [DATADIR]/
   TNamed::SetName(name);
   TString path = GetDataSetEnv("DataSet.Directory", name);
   if (gSystem->IsAbsoluteFileName(path)) fCalibDir = path;
   else fCalibDir = GetDATADIRFilePath(path);
}

const Char_t* KVDataSet::GetDataSetDir() const
{
   //Returns full path to directory containing database and calibration/identification parameters etc.
   //for this dataset.
   return fCalibDir.Data();
}

void KVDataSet::cd() const
{
   //Data analysis can only be performed if the data set in question
   //is "activated" or "selected" using this method.
   //At the same time, the data repository, dataset manager and database associated with
   //this dataset also become the "active" ones (pointed to by the respective global
   //pointers, gDataRepository, gDataBase, etc. etc.)

   gDataSet = const_cast<KVDataSet*>(this);
   if (fRepository) fRepository->cd();
   KVDataBase* db = GetDataBase();
   if (db) db->cd();
}

//__________________________________________________________________________________________________________________

const Char_t* KVDataSet::GetDataSetEnv(const Char_t* type, const Char_t* defval) const
{
   //Will look for gEnv->GetValue "name_of_dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value (="" by default)

   return GetDataSetEnv(GetName(), type, defval);
}

//__________________________________________________________________________________________________________________

Double_t KVDataSet::GetDataSetEnv(const Char_t* type, Double_t defval) const
{
   //Will look for gEnv->GetValue "name_of_dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value

   return GetDataSetEnv(GetName(), type, defval);
}

//__________________________________________________________________________________________________________________

Bool_t KVDataSet::GetDataSetEnv(const Char_t* type, Bool_t defval) const
{
   //Will look for gEnv->GetValue "name_of_dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value

   return GetDataSetEnv(GetName(), type, defval);
}

const Char_t* KVDataSet::GetDataSetEnv(const Char_t* dataset, const Char_t* type, const Char_t* defval)
{
   // Static method to interrogate dataset-specific variables in configuration
   // Will look for gEnv->GetValue "dataset.type"
   // then simply "type" if no dataset-specific value is found.
   // If neither resource is defined, return the "defval" default value (="" by default)
   TString temp;
   temp.Form("%s.%s", dataset, type);
   if (gEnv->Defined(temp.Data())) return gEnv->GetValue(temp.Data(), "");
   return gEnv->GetValue(type, defval);
}

Double_t KVDataSet::GetDataSetEnv(const Char_t* dataset, const Char_t* type, Double_t defval)
{
   // Static method to interrogate dataset-specific variables in configuration
   // Will look for gEnv->GetValue "dataset.type"
   // then simply "type" if no dataset-specific value is found.
   // If neither resource is defined, return the "defval" default value

   TString temp;
   temp.Form("%s.%s", dataset, type);
   if (gEnv->Defined(temp.Data())) return gEnv->GetValue(temp.Data(), 0.0);
   return gEnv->GetValue(type, defval);
}

Bool_t KVDataSet::GetDataSetEnv(const Char_t* dataset, const Char_t* type, Bool_t defval)
{
   // Static method to interrogate dataset-specific variables in configuration
   //Will look for gEnv->GetValue "dataset.type"
   //then simply "type" if no dataset-specific value is found.
   //If neither resource is defined, return the "defval" default value

   TString temp;
   temp.Form("%s.%s", dataset, type);
   if (gEnv->Defined(temp.Data())) return gEnv->GetValue(temp.Data(), kFALSE);
   return gEnv->GetValue(type, defval);
}

//__________________________________________________________________________________________________________________

TObject* KVDataSet::OpenRunfile(const Char_t* type, Int_t run)
{
   // Open file containing data of given datatype for given run number of this dataset.
   // Returns a pointer to the opened file; if the file is not available, we return nullptr.
   // The user must cast the returned pointer to the correct class, which will
   // depend on the data type and the dataset (see $KVROOT/KVFiles/.kvrootrc)

   return GetRepository()->OpenDataSetRunFile(this, type, run);
}

//__________________________________________________________________________________________________________________

TString KVDataSet::GetFullPathToRunfile(const Char_t* type,
                                        Int_t run) const
{
   //Return full path to file containing data of given datatype for given run number
   //of this dataset. NB. only works for available run files, if their is no file in the repository for this run,
   //the returned path will be empty.
   //This path should be used with e.g. TChain::Add.

   //get name of file from available runs file
   TString file("");
   if (fRepository) file = GetRunfileName(type, run);
   if (file == "")
      return file.Data();
   return fRepository->GetFullPathToOpenFile(this, type, file.Data());
}

//__________________________________________________________________________________________________________________

const Char_t* KVDataSet::GetRunfileName(const Char_t* type, Int_t run) const
{
   //Return name of file containing data of given datatype
   //for given run number of this dataset.
   //NB. only works for available run files, if there is no file in the repository for this run,
   //the returned path will be empty.

   //check data type is available
   if (!HasDataType(type)) {
      Error("GetRunfileName",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
      return 0;
   }
   //get name of file from available runs file
   return GetAvailableRunsFile(type)->GetFileName(run);
}

//__________________________________________________________________________________________________________________

TDatime KVDataSet::GetRunfileDate(const Char_t* type, Int_t run)
{
   //Return date of file containing data of given datatype
   //for given run number of this dataset.
   //NB. only works for available run files, if there is no file in the repository for this run,
   //an error will be printed and the returned data is set to "Sun Jan  1 00:00:00 1995"
   //(earliest possible date for TDatime class).

   static TDatime date;
   date.Set(1995, 1, 1, 0, 0, 0);
   //check data type is available
   if (!HasDataType(type)) {
      Error("GetRunfileDate",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
      return date;
   }
   //get date of file from available runs file
   TString filename;
   if (!GetAvailableRunsFile(type)->GetRunInfo(run, date, filename)) {
      Error("GetRunfileDate",
            "Runfile not found for run %d (data type: %s)", run, type);
   }
   return date;
}

//__________________________________________________________________________________________________________________

Bool_t KVDataSet::CheckRunfileAvailable(const Char_t* type, Int_t run)
{
   //We check the availability of the run by looking in the available runs file associated
   //with the given datatype.

   //check data type is available
   if (!HasDataType(type)) {
      Error("CheckRunfileAvailable",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
      return 0;
   }
   return GetAvailableRunsFile(type)->CheckAvailable(run);
}

//___________________________________________________________________________

const Char_t* KVDataSet::GetBaseFileName(const Char_t* type, Int_t run) const
{
   //PRIVATE METHOD: Returns base name of data file containing data for the run of given datatype.
   //The filename corresponds to one of the formats defined in $KVROOT/KVFiles/.kvrootrc
   //by variables like:
   //
   //[dataset].DataSet.RunFileName.[type]:    run%d.dat
   //
   //The actual name of the file, if it has already been written in the data repository,
   //is contained in the available_runs.*.* file; if the file has not been written, or it is to
   //be updated, a new name will be generated from this base, adding the date and time
   //to the end.

   static TString tmp;
   //get format string
   TString fmt = GetDataSetEnv(Form("DataSet.RunFileName.%s", type));
   if (fmt != "") {
      tmp.Form(fmt, run);
   }
   return tmp.Data();
}

//___________________________________________________________________________

void KVDataSet::UpdateAvailableRuns(const Char_t* type)
{
   //Update list of available runs for given data 'type'

   //check data type is available
   if (!HasDataType(type)) {
      Error("UpdateAvailableRuns",
            "No data of type \"%s\" available for dataset %s", type,
            GetName());
   }
   KVAvailableRunsFile* a = GetAvailableRunsFile(type);
   a->Update(!a->FileExists());
}

//___________________________________________________________________________

TFile* KVDataSet::NewRunfile(const Char_t* type, Int_t run)
{
   // Create a new runfile for the dataset of given datatype.
   // (only if this dataset is associated with a data repository)
   // The name of the new file will be a concatenation of GetBaseFileName(type,run)
   // and the current date and time (TDatime::AsSQLString).
   // Once the file has been filled, use CommitRunfile to submit it to the repository.

   if (!fRepository) return nullptr;
   TDatime now;
   TString tmp;
   tmp.Form("%s.%s", GetBaseFileName(type, run), now.AsSQLString());
   //turn any spaces into "_"
   tmp.ReplaceAll(" ", "_");
   return fRepository->CreateNewFile(this, type, tmp.Data());
}

//___________________________________________________________________________

void KVDataSet::DeleteRunfile(const Char_t* type, Int_t run, Bool_t confirm)
{
   // Delete the file for the given run of data type "type" from the repository.
   // By default, confirm=kTRUE, which means that the user will be asked to confirm
   // that the file should be deleted. If confirm=kFALSE, no confirmation will be asked
   // for and the file will be deleted straight away.
   //
   // WARNING: this really does DELETE files in the repository, they cannot be
   // retrieved once they have been deleted.

   if (!fRepository) return;

   //get name of file to delete
   TString filename = GetAvailableRunsFile(type)->GetFileName(run);
   if (filename == "") {
      Error("DeleteRunfile", "Run %d of type %s does not exist.", run, type);
      return;
   }
   //delete file
   //prevent accidental deletion of certain types of runfiles
   KVString doNotDelete = GetDataSetEnv("DataSet.RunFile.DoNotDelete", "all");
   if (doNotDelete == "all" || doNotDelete.Contains(type)) {
      Error("DeleteRunFile", "%s files cannot be deleted", type);
      return;
   }
   fRepository->DeleteFile(this, type, filename.Data(), confirm);
   //was file deleted ? if so, remove entry from available runs file
   if (!fRepository->CheckFileStatus(this, type, filename.Data()))
      GetAvailableRunsFile(type)->Remove(run);
}

//___________________________________________________________________________

void KVDataSet::DeleteRunfiles(const Char_t* type, KVNumberList nl, Bool_t confirm)
{
   //Delete files corresponding to a list of runs of data type "type" from the repository.
   //By default, confirm=kTRUE, which means that the user will be asked to confirm
   //that each file should be deleted. If confirm=kFALSE, no confirmation will be asked
   //for and the file will be deleted straight away.
   //if "nl" is empty (default value) all runs of the dataset corresponding to the given type
   //will be deleted
   //WARNING: this really does DELETE files in the repository, they cannot be
   //retrieved once they have been deleted.

   if (nl.IsEmpty()) nl = GetRunList(type);
   if (nl.IsEmpty()) return;
   nl.Begin();
   while (!nl.End()) {
      DeleteRunfile(type, nl.Next(), confirm);
   }
}

//___________________________________________________________________________

KVNumberList KVDataSet::GetRunList_DateSelection(const Char_t* type, TDatime* min, TDatime* max)
{
   //Prints out and returns list of runs after date / time selection
   //Runs generated between ]min;max[ are selected
   //if min=NULL  runs with date <max are selected
   //if max=NULL runs with date >min are selected
   //if max and min are NULL returns empty KVNumberList

   if (!min && !max) return 0;

   if (min) printf("date minimum %s\n", min->AsString());
   if (max) printf("date maximum %s\n", max->AsString());

   KVNumberList numb;

   unique_ptr<TList> ll(GetListOfAvailableSystems(type));
   KVDBSystem* sys = 0;
   KVRunFile* run = 0;
   unique_ptr<TList> lrun;
   for (Int_t nl = 0; nl < ll->GetEntries(); nl += 1) {
      sys = (KVDBSystem*)ll->At(nl);
      lrun.reset(GetListOfAvailableSystems(type, sys));
      KVNumberList oldList = numb;
      for (Int_t nr = 0; nr < lrun->GetEntries(); nr += 1) {
         run = (KVRunFile*)lrun->At(nr);

         if (min && max) {
            if (*min < run->GetRun()->GetDatime() && run->GetRun()->GetDatime() < *max) {
               numb.Add(run->GetRunNumber());
            }
         } else if (min) {
            if (*min < run->GetRun()->GetDatime()) {
               numb.Add(run->GetRunNumber());
            }
         } else if (max) {
            if (run->GetRun()->GetDatime() < *max) {
               numb.Add(run->GetRunNumber());
            }
         }
      }
      // print runs for system if any
      if (numb.GetEntries() > oldList.GetEntries()) printf("%s : %s\n", sys->GetName(), (numb - oldList).AsString());
   }
   return numb;

}

//___________________________________________________________________________

KVNumberList KVDataSet::GetRunList_StageSelection(const Char_t* type, const Char_t* ref_type, KVDBSystem* system, Bool_t OnlyCol)
{
   // Returns list of runs which are present for data type "base_type" but not for "other_type"
   // if type is NULL or ="" returns empty KVNumberList
   // If pointer to system is given, only runs for the system are considered.
   // If OnlyCol=kTRUE (kFALSE default) only systems with KVDBSystem::IsCollision()=kTRUE are considered

   KVNumberList manquant;
   TList* ll = GetListOfAvailableSystems(ref_type);
   if (!ll || !ll->GetEntries()) {
      //numb.Clear();
      Info("GetRunList_StageSelection", "No data available of type \"%s\"", ref_type);
      if (ll) delete ll;
      return manquant;
   }
   if (system && !ll->FindObject(system)) {
      Info("GetRunList_StageSelection", "No data available of type \"%s\" for system %s", ref_type, system->GetName());
      delete ll;
      return manquant;
   }

   Info("GetRunList_StageSelection", "Liste des runs presents dans \"%s\" mais absent dans \"%s\"", ref_type, type);

   KVDBSystem* sys = 0;
   KVNumberList nsys_ref;
   KVNumberList nsys;

   for (Int_t nl = 0; nl < ll->GetEntries(); nl += 1) {

      sys = (KVDBSystem*)ll->At(nl);
      if (system && sys != system) continue;
      if (OnlyCol && !sys->IsCollision()) continue;
      nsys = GetRunList(type, sys);
      nsys_ref = GetRunList(ref_type, sys);
      Int_t nref = nsys_ref.GetNValues();

      nsys_ref.Remove(nsys);

      Info("GetRunList_StageSelection", "\nKVDBSystem : %s --> %d runs manquants sur %d : %s",
           sys->GetName(),
           nsys_ref.GetNValues(),
           nref,
           nsys_ref.AsString()
          );
      manquant.Add(nsys_ref);
   }
   delete ll;
   return manquant;

}
//___________________________________________________________________________

KVNumberList KVDataSet::GetRunList_VersionSelection(const Char_t* type, const Char_t* version, KVDBSystem* sys)
{
   // Returns list of runs of given type that were created with the given version of KaliVeda.
   // If system!="" then only runs for the given system are considered

   KVNumberList runs;
   if (sys) {
      unique_ptr<TList> lrun(GetListOfAvailableSystems(type, sys));
      TIter next(lrun.get());
      KVRunFile* run;
      while ((run = (KVRunFile*)next())) {
         if (!strcmp(run->GetVersion(), version)) runs.Add(run->GetRunNumber());
      }
      return runs;
   }
   unique_ptr<TList> ll(GetListOfAvailableSystems(type));
   if (!ll.get() || !ll->GetEntries()) {
      //numb.Clear();
      Info("GetRunList_VersionSelection", "No data available of type \"%s\"", type);
      return runs;
   }
   Int_t nsys = ll->GetEntries();
   for (Int_t nl = 0; nl < nsys; nl += 1) {
      sys = (KVDBSystem*)ll->At(nl);
      unique_ptr<TList> lrun(GetListOfAvailableSystems(type, sys));
      TIter next(lrun.get());
      KVRunFile* run;
      while ((run = (KVRunFile*)next())) {
         if (!strcmp(run->GetVersion(), version)) runs.Add(run->GetRunNumber());
      }
   }
   return runs;
}

//___________________________________________________________________________
void KVDataSet::CommitRunfile(const Char_t* type, Int_t run, TFile* file)
{
   // Commit a runfile previously created with NewRunfile() to the repository.
   // Any previous version of the runfile will be deleted.
   // The available runs list for this data 'type'  is updated.

   if (!fRepository) return;

   //keep name of file for updating available runs list
   TString newfile = gSystem->BaseName(file->GetName());

   fRepository->CommitFile(file, type, this);
   //update list of available datatypes of dataset,
   //in case this addition has created a new subdirectory
   CheckAvailable();
   //check if previous version of file exists
   //get name of file from available runs file
   //note that when the file is the first of a new subdirectory, GetAvailableRunsFile->GetFileName
   //will cause the available runs file to be created, and it will contain one entry:
   //the new file!
   TString oldfile = GetAvailableRunsFile(type)->GetFileName(run);
   if (oldfile != "" && oldfile != newfile) {
      //delete previous version - no confirmation
      fRepository->DeleteFile(this, type, oldfile.Data(),
                              kFALSE);
      //was file deleted ? if so, remove entry from available runs file
      if (!fRepository->CheckFileStatus(this, type, oldfile.Data()))
         GetAvailableRunsFile(type)->Remove(run);
   }
   if (oldfile != newfile) {
      //add entry for new run in available runs file
      GetAvailableRunsFile(type)->Add(run, newfile.Data());
   }
}

//___________________________________________________________________________

Bool_t KVDataSet::CheckUserCanAccess()
{
   //if fUserGroups has been set with SetUserGroups(), we check that the current user's name
   //(gSystem->GetUserInfo()->fUser) appears in at least one of the groups in the list.
   //Returns kFALSE if user's name is not found in any of the groups.
   //if fUserGroups="" (default), we return kTRUE for all users.

   if (fUserGroups == "")
      return kTRUE;             /* no groups set, all users have access */

   //split into array of group names
   unique_ptr<TObjArray> toks(fUserGroups.Tokenize(' '));
   TObjString* group_name;
   TIter next_name(toks.get());
   while ((group_name = (TObjString*) next_name())) {
      //for each group_name, we check if the user's name appears in the group
      if (!fRepository || (fRepository && fRepository->GetDataSetManager()->
                           CheckUser(group_name->String().Data()))
         ) {
         return kTRUE;
      }
   }
   return kFALSE;
}

//___________________________________________________________________________

void KVDataSet::SetRepository(KVDataRepository* dr)
{
   //Set pointer to data repository in which dataset is stored
   fRepository = dr;
}

//___________________________________________________________________________

KVDataRepository* KVDataSet::GetRepository() const
{
   //Get pointer to data repository in which dataset is stored
   return fRepository;
}

//___________________________________________________________________________

void KVDataSet::CheckMultiRunfiles(const Char_t* data_type)
{
   //Check all runs for a given datatype and make sure that only one version
   //exists for each run. If not, we print a report on the runfiles which occur
   //multiple times, with the associated date and file name.

   KVNumberList doubles =
      GetAvailableRunsFile(data_type)->CheckMultiRunfiles();
   if (doubles.IsEmpty()) {
      cout << "OK. No runs appear more than once." << endl;
   } else {
      cout << "Runs which appear more than once: " << doubles.
           AsString() << endl << endl;
      //print dates and filenames for each run

      doubles.Begin();
      KVList filenames, dates;
      while (!doubles.End()) {

         Int_t rr = doubles.Next();

         //get infos for current run
         GetAvailableRunsFile(data_type)->GetRunInfos(rr, &dates, &filenames);

         cout << "Run " << rr << " : " << dates.
              GetEntries() << " files >>>>>>" << endl;
         for (int i = 0; i < dates.GetEntries(); i++) {

            cout << "\t" << ((TObjString*) filenames.At(i))->String().
                 Data() << "\t" << ((TObjString*) dates.At(i))->String().
                 Data() << endl;

         }
      }
   }
}

//___________________________________________________________________________

void KVDataSet::CleanMultiRunfiles(const Char_t* data_type, Bool_t confirm)
{
   // Check all runs for a given datatype and make sure that only one version
   // exists for each run. If not, we print a report on the runfiles which occur
   // multiple times, with the associated date and file name, and then we
   // destroy all but the most recent version of the file in the repository, and
   // update the runlist accordingly.
   // By default, we ask for confirmation before deleting each file.
   // Call with confirm=kFALSE to delete WITHOUT CONFIRMATION (DANGER!! WARNING!!!)

   if (!fRepository) return;

   KVAvailableRunsFile* ARF = GetAvailableRunsFile(data_type);
   KVNumberList doubles = ARF->CheckMultiRunfiles();
   if (doubles.IsEmpty()) {
      cout << "OK. No runs appear more than once." << endl;
   } else {
      cout << "Runs which appear more than once: " << doubles.
           AsString() << endl << endl;
      //print dates and filenames for each run

      KVList filenames, dates;
      doubles.Begin();
      while (!doubles.End()) {

         Int_t rr = doubles.Next();

         //get infos for current run
         ARF->GetRunInfos(rr, &dates, &filenames);

         TDatime most_recent("1998-12-25 00:00:00");
         Int_t i_most_recent = 0;
         cout << "Run " << rr << " : " << dates.
              GetEntries() << " files >>>>>>" << endl;
         for (int i = 0; i < dates.GetEntries(); i++) {

            //check if run is most recent
            TDatime rundate(((TObjString*) dates.At(i))->String().Data());
            if (rundate > most_recent) {

               most_recent = rundate;
               i_most_recent = i;

            }
         }
         //Now, we loop over the list again, this time we destroy all but the most recent
         //version of the runfile
         for (int i = 0; i < dates.GetEntries(); i++) {

            if (i == i_most_recent) {
               cout << "KEEP : ";
            } else {
               cout << "DELETE : ";
            }
            cout << "\t" << ((TObjString*) filenames.At(i))->String().
                 Data() << "\t" << ((TObjString*) dates.At(i))->String().
                 Data() << endl;
            if (i != i_most_recent) {
               //delete file from repository forever and ever
               fRepository->DeleteFile(this, data_type,
                                       ((TObjString*) filenames.At(i))->
                                       String().Data(), confirm);
               //remove file entry from available runlist
               ARF->Remove(rr,
                           ((TObjString*) filenames.At(i))->String().
                           Data());
            }
         }
      }
   }
}

//___________________________________________________________________________

Bool_t KVDataSet::CheckRunfileUpToDate(const Char_t* data_type, Int_t run,
                                       KVDataRepository* other_repos)
{
   //Use this method to check whether the file of type "data_type" for run number "run"
   //in the data repository "other_repos" is more recent than the file contained in the data
   //repository corresponding to this dataset.
   //Returns kFALSE if file in other repository is more recent.

   if (!other_repos)
      return kTRUE;
   //get dataset with same name as this one from dataset manager of other repository
   KVDataSet* ds = other_repos->GetDataSetManager()->GetDataSet(GetName());
   if (!ds) {
      Error("CheckRunfileUpToDate",
            "Dataset \"%s\" not found in repository \"%s\"", GetName(),
            other_repos->GetName());
      return kFALSE;
   }
   //compare dates of the two runfiles
   if (GetRunfileDate(data_type, run) < ds->GetRunfileDate(data_type, run))
      return kFALSE;
   return kTRUE;
}

//___________________________________________________________________________

Bool_t KVDataSet::CheckRunfileUpToDate(const Char_t* data_type, Int_t run,
                                       const Char_t* other_repos)
{
   //Use this method to check whether the file of type "data_type" for run number "run"
   //in the data repository "other_repos" is more recent than the file contained in the data
   //repository corresponding to this dataset.
   //Returns kTRUE if no repository with name "other_repos" exists.
   //Returns kFALSE if file in other repository is more recent.

   KVDataRepository* _or =
      gDataRepositoryManager->GetRepository(other_repos);
   if (_or)
      return CheckRunfileUpToDate(data_type, run, _or);
   Error("CheckRunfileUpToDate",
         "No data repository known with this name : %s", other_repos);
   return kTRUE;
}

//___________________________________________________________________________

void KVDataSet::CheckUpToDate(const Char_t* data_type,
                              const Char_t* other_repos)
{
   //Check whether all files of type "data_type" for run number "run" in the data repository
   //are up to date (i.e. at least as recent) as compared to the files in data repository "other_repos".

   if (!fRepository) return;

   KVDataRepository* _or =
      gDataRepositoryManager->GetRepository(other_repos);
   if (!_or) {
      Error("CheckUpToDate",
            "No data repository known with this name : %s", other_repos);
      return;
   }
   KVNumberList runlist = GetAvailableRunsFile(data_type)->GetRunList();
   runlist.Begin();
   Int_t need_update = 0;
   while (!runlist.End()) {
      //check run
      Int_t rr = runlist.Next();
      if (!CheckRunfileUpToDate(data_type, rr, _or)) {
         cout << " *** run " << rr << " needs update ***" <<
              endl;
         cout << "\t\tREPOSITORY: " << fRepository->
              GetName() << "\tDATE: " << GetRunfileDate(data_type,
                    rr).
              AsString() << endl;
         cout << "\t\tREPOSITORY: " << other_repos << "\tDATE: " << _or->
              GetDataSetManager()->GetDataSet(GetName())->
              GetRunfileDate(data_type,
                             rr).AsString() << endl;
         need_update++;
      }
   }
   if (!need_update) {
      cout << " *** All runfiles are up to date for data type " <<
           data_type << endl;
   }
}

//___________________________________________________________________________

KVNumberList KVDataSet::GetUpdatableRuns(const Char_t* data_type,
      const Char_t* other_repos)
{
   //Returns list of all runs of type "data_type" which may be updated
   //from the repository named "other_repos". See CheckUpToDate().

   KVNumberList updates;
   if (!fRepository) return updates;

   KVDataRepository* _or =
      gDataRepositoryManager->GetRepository(other_repos);
   if (!_or) {
      Error("CheckUpToDate",
            "No data repository known with this name : %s", other_repos);
      return updates;
   }
   KVNumberList runlist = GetAvailableRunsFile(data_type)->GetRunList();
   runlist.Begin();
   while (!runlist.End()) {
      //check run
      Int_t rr = runlist.Next();
      if (!CheckRunfileUpToDate(data_type, rr, _or)) {
         //run is out of date
         updates.Add(rr);
      }
   }
   return updates;
}

//___________________________________________________________________________

KVNumberList KVDataSet::GetRunList(const Char_t* data_type,
                                   const KVDBSystem* system) const
{
   //Returns list of all runs available for given "data_type"
   //If a pointer to a reaction system is given, only runs for the
   //given system will be included in the list.
   KVNumberList list;
   if (!fRepository || !HasDataType(data_type)) {
      Error("GetRunList",
            "No data of type %s available. Runlist will be empty.",
            data_type);
   } else {
      list = GetAvailableRunsFile(data_type)->GetRunList(system);
   }
   return list;
}

//___________________________________________________________________________

KVDataAnalysisTask* KVDataSet::GetAnalysisTask(const Char_t* keywords) const
{
   //This method returns a pointer to the available analysis task whose description (title) contains
   //all of the whitespace-separated keywords (which may be regular expressions)
   //given in the string "keywords". The comparison is case-insensitive.
   //
   //WARNING: this method can only be used to access analysis tasks that are
   //available for this dataset, i.e. for which the corresponding prerequisite data type
   //is available in the repository.
   //For unavailable data/tasks, use GetAnalysisTaskAny(const Char_t*).
   //
   //EXAMPLES
   //Let us suppose that the current dataset has the following list of tasks:
   //
// root [2] gDataSet->Print("tasks")
//         1. Event reconstruction from raw data (raw->recon)
//         2. Analysis of raw data
//         3. Identification of reconstructed events (recon->ident)
//         4. Analysis of reconstructed events (recon)
//         5. Analysis of partially identified & calibrated reconstructed events (ident)
//         6. Analysis of fully calibrated physical data (root)
   //Then the following will occur:
   //
// root [14] gDataSet->GetAnalysisTask("raw->recon")->Print()
// KVDataAnalysisTask : Event reconstruction from raw data (raw->recon)
//
// root [10] gDataSet->GetAnalysisTask("analysis root")->Print()
// KVDataAnalysisTask : Analysis of fully calibrated physical data (root)
//

   //case-insensitive search for matches in list based on 'title' attribute
   return (KVDataAnalysisTask*)fTasks.FindObjectAny("title", keywords, kTRUE, kFALSE);
}

//___________________________________________________________________________

void KVDataSet::MakeAnalysisClass(const Char_t* task, const Char_t* classname)
{
   //Create a skeleton analysis class to be used for analysis of the data belonging to this dataset.
   //
   //  task = keywords contained in title of analysis task (see GetAnalysisTask(const Char_t*))
   //              (you do not need to include 'analysis', it is added automatically)
   //  classname = name of new analysis class
   //
   //Example:
   //  MakeAnalysisClass("raw", "MyRawDataAnalysis")
   //       -->  make skeleton raw data analysis class in files MyRawDataAnalysis.cpp & MyRawDataAnalysis.h
   //  MakeAnalysisClass("fully calibrated", "MyDataAnalysis")
   //       -->  make skeleton data analysis class in files MyDataAnalysis.cpp & MyDataAnalysis.h

   KVString _task = task;
   _task += " analysis";
   //We want to be able to write analysis classes even when we don't have any data
   //to analyse. Therefore we use GetAnalysisTaskAny.
   unique_ptr<KVDataAnalysisTask> dat(GetAnalysisTaskAny(_task.Data()));
   if (!dat.get()) {
      Error("MakeAnalysisClass",
            "called for unknown or unavailable analysis task : %s", _task.Data());
      return;
   }
   if (!dat->WithUserClass()) {
      Error("MakeAnalysisClass",
            "no user analysis class for analysis task : %s", dat->GetTitle());
      return;
   }

   //all analysis base classes must define a static Make(const Char_t * classname)
   //which generates the skeleton class files.

   TClass* cl = 0x0;
   //has the user base class for the task been compiled and loaded ?
   if (dat->CheckUserBaseClassIsLoaded()) cl = TClass::GetClass(dat->GetUserBaseClass());
   else
      return;

   //set up call to static Make method
   unique_ptr<TMethodCall> methcall(new  TMethodCall(cl, "Make", Form("\"%s\"", classname)));

   if (!methcall->IsValid()) {
      Error("MakeAnalysisClass", "static Make(const Char_t*) method for class %s is not valid",
            cl->GetName());
      return;
   }

   //generate skeleton class
   methcall->Execute();
}

//___________________________________________________________________________

Bool_t KVDataSet::OpenDataSetFile(const Char_t* filename, ifstream& file)
{
   //Look for (and open for reading, if found) the named file in the directory which
   //contains the files for this dataset
   return SearchAndOpenKVFile(filename, file, GetName());
}

//___________________________________________________________________________

KVDataAnalysisTask* KVDataSet::GetAnalysisTaskAny(const Char_t* keywords) const
{
   //This method returns a pointer to the analysis task whose description (title) contains
   //all of the whitespace-separated keywords (which may be regular expressions)
   //given in the string "keywords". The comparison is case-insensitive.
   //The analysis task does not need to be "available", i.e. the associated prerequisite
   //data type does not have to be present in the repository (see GetAnalysisTask).
   //
   //WARNING! WARNING! WARNING!
   //  *** the user must delete the KVDataAnalysisTask object returned by this method after use ***
   //

   //case-insensitive search for matches in list of all analysis tasks, based on 'title' attribute
   KVDataAnalysisTask* tsk =
      (KVDataAnalysisTask*) gDataSetManager->GetAnalysisTaskList()->FindObjectAny("title", keywords, kTRUE, kFALSE);
   if (!tsk) {
      Error("GetAnalysisTaskAny", "No task found with the following keywords in its title : %s",
            keywords);
      return 0;
   }
   //make new copy of default analysis task
   KVDataAnalysisTask* new_task = new KVDataAnalysisTask(*tsk);
   //check if any dataset-specific parameters need to be changed
   SetDataSetSpecificTaskParameters(new_task);
   return new_task; //must be deleted by user
}

//___________________________________________________________________________

Bool_t KVDataSet::DataBaseNeedsUpdate() const
{
   // Returns kTRUE if database needs to be regenerated from source files,
   // i.e. if source files are more recent than DataBase.root
   // In case no directory exists for dataset (dataset added 'on the fly')
   // we create the directory and fill it with dummy files (Makefile, Runlist.csv, Systems.dat)

   TString pwd = gSystem->pwd();

   TString path = "";
   if (!SearchKVFile(GetDataSetDir(), path)) {
      // dataset directory doesn't exist - create it
      Info("DataBaseNeedsUpdate", "%s: Creating new dataset directory %s",
           GetName(), GetDataSetDir());
      if (gSystem->mkdir(GetDataSetDir())) {
         // problem creating directory
         Error("DataBaseNeedsUpdate",
               "%s: Dataset directory %s does not exist and cannot be created ?",
               GetName(), GetDataSetDir());
         return kFALSE;
      }
      // create dummy files
      SearchKVFile(GetDataSetDir(), path); // get full path
      path += "/";
      TString filename = path + "Makefile";
      ofstream of1(filename.Data());
      of1 << "$(KV_WORK_DIR)/db/" << GetName() << "/DataBase.root : Runlist.csv Systems.dat" << endl;
      of1 << "\t@echo Database needs update" << endl;
      of1.close();
      filename = path + "Runlist.csv";
      ofstream of2(filename.Data());
      of2 << "# Automatically generated dummy Runlist.csv file" << endl;
      of2.close();
      filename = path + "Systems.dat";
      ofstream of3(filename.Data());
      of3 << "# Automatically generated dummy Systems.dat file" << endl;
      of3.close();
   }
   gSystem->cd(GetDataSetDir());
   TString cmd = "make -q";
   Int_t ret = gSystem->Exec(cmd.Data());
   gSystem->cd(pwd.Data());
   return (ret != 0);
}

TString KVDataSet::GetOutputRepository(const Char_t* taskname)
{
   // Returns name of output repository for given task.
   // By default it is the name of the repository associated with this dataset,
   // but can be changed by the following environment variables:
   //
   // [repository].DefaultOutputRepository: [other repository]
   //    - this means that all tasks carried out on data in [repository]
   //      will have their output files placed in [other repository]
   //
   // [taskname].DataAnalysisTask.OutputRepository: [other repository]
   //    - this means that for [taskname], any output files will
   //      be placed in [other repository]
   //
   // [dataset].[taskname].DataAnalysisTask.OutputRepository: [other repository]
   //    - this means that for given [dataset] & [taskname],
   //      any output files will be placed in [other repository]

   if (gEnv->Defined(Form("%s.DataRepository.DefaultOutputRepository", GetRepository()->GetName())))
      return TString(gEnv->GetValue(Form("%s.DataRepository.DefaultOutputRepository", GetRepository()->GetName()), ""));
   TString orep = GetDataSetEnv(Form("%s.DataAnalysisTask.OutputRepository", taskname), GetRepository()->GetName());
   return orep;
}

//___________________________________________________________________________

void KVDataSet::CopyRunfilesFromRepository(const Char_t* type, KVNumberList runs, const Char_t* destdir)
{
   // Copies the runfiles of given "type" into the local directory "destdir".
   // Run numbers given as a list of type "1-10".

   KVDataRepository* repo = GetRepository();
   runs.Begin();
   while (!runs.End()) {
      int run = runs.Next();
      TString filename = GetRunfileName(type, run);
      TString destpath;
      AssignAndDelete(destpath, gSystem->ConcatFileName(destdir, filename));
      repo->CopyFileFromRepository(this, type, filename, destpath);
   }
}

//___________________________________________________________________________

void KVDataSet::CopyRunfilesToRepository(const Char_t* type, KVNumberList runs, const Char_t* destrepo)
{
   // Copies the runfiles of given "type" from the data repository associated
   // with this dataset into the local repository "destrepo".
   // Run numbers given as a list of type "1-10".

   KVDataRepository* repo = GetRepository();
   KVDataRepository* dest_repo = gDataRepositoryManager->GetRepository(destrepo);

   if (!dest_repo) {
      Error("CopyRunfilesToRepository", "Unknown destination repository : %s", destrepo);
      gDataRepositoryManager->Print();
      return;
   }

   KVDataSet* dest_ds = dest_repo->GetDataSetManager()->GetDataSet(GetName());
   dest_repo->CreateAllNeededSubdirectories(dest_ds, type);
   runs.Begin();
   while (!runs.End()) {
      int run = runs.Next();
      TString filename = GetRunfileName(type, run);
      TString destpath = dest_repo->GetFullPathToTransferFile(dest_ds, type, filename);
      repo->CopyFileFromRepository(this, type, filename, destpath);
   }
}
