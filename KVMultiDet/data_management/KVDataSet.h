/*
$Id: KVDataSet.h,v 1.26 2009/01/27 08:06:56 franklan Exp $
$Revision: 1.26 $
$Date: 2009/01/27 08:06:56 $
$Author: franklan $
*/

#ifndef __KVDATASET_H
#define __KVDATASET_H

#include "KVString.h"
#include "KVBase.h"
#include "KVList.h"
#include "KVNumberList.h"
#include "TDatime.h"
#include "TFile.h"
#include "TEnv.h"
#include "TSystem.h"
#include "KVDataAnalysisTask.h"
#include "KVExpDB.h"

class KVDataSetManager;
class KVDataRepository;
class KVDBSystem;
class TObjArray;
class KVDBRun;
class KVAvailableRunsFile;

/**
\class KVDataSet
\brief Manage an experimental dataset corresponding to a given experiment or campaign
\ingroup DM

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
*/

class KVDataSet: public KVBase {

   friend class KVAvailableRunsFile;
   friend class KVDataSetManager;

protected:
   KVDataRepository* fRepository;      //repository in which dataset is stored
   TString fDatatypes;            //list of types of data which are available for this dataset
   KVList fTasks;                //possible data analysis tasks for this dataset
   TString fCalibDir;           //directory containing database, calibration, identification parameters etc. for dataset
   Bool_t fDBBuild;             //has the database been built by us ?
   mutable unique_ptr<TFile> fDBase;               //file containing database
   TString fDBName;             //name of database
   TString fDBFileName;         //name of file in which database is stored on disk
   mutable KVExpDB* fDataBase;       //pointer to dataset's database
   mutable KVList fAvailableRuns;       //!list of KVAvailableRunsFile objects used to read infos on available runs
   enum {
      kAvailable = BIT(14)     //flag set if this dataset is physically present on local machine
   };
   TString fUserGroups;         //list of user groups allowed to read data

   virtual const Char_t* GetBaseFileName(const Char_t* type, Int_t run) const;

   virtual void OpenDataBase(Option_t* opt = "") const;
   virtual void OpenDBFile(const Char_t* full_path_to_dbfile) const;
   virtual void WriteDBFile(const Char_t* full_path_to_dbfile) const;
   void SetDBFileName(const Char_t* name)
   {
      fDBFileName = name;
   }
   const Char_t* GetDBFileName() const;
   void SetDBName(const Char_t* name)
   {
      fDBName = name;
   }
   const Char_t* GetDBName() const;
   const Char_t* GetFullPathToDB() const;
   void SetDataSetSpecificTaskParameters(KVDataAnalysisTask*) const;
   TObject* open_runfile(const Char_t* type, Int_t run);

public:

   KVAvailableRunsFile* GetAvailableRunsFile(const Char_t* type) const;

   KVDataSet();
   virtual ~ KVDataSet() {}

   virtual Bool_t CheckUserCanAccess();

   virtual void SetDataPathSubdir(const Char_t* s)
   {
      SetLabel(s);
   }
   // Returns name of top-level directory in data repository used to store data files for this dataset
   virtual const Char_t* GetDataPathSubdir() const
   {
      return GetLabel();
   }
   const Char_t* GetDataTypeSubdir(const Char_t* type) const
   {
      // returns name to be used for subdirectory corresponding to given data type
      KVString snom;
      snom.Form("KVDataSet.DataType.Subdir.%s", type);
      return GetDataSetEnv(snom.Data(), type);
   }
   virtual const Char_t* GetAvailableDataTypes() const
   {
      return fDatatypes.Data();
   }
   virtual void AddAvailableDataType(const Char_t*);
   virtual void SetUserGroups(const Char_t* groups)
   {
      fUserGroups = groups;
   }
   // Returns kTRUE if this dataset is available for analysis, i.e. if any associated data files are stored in the data repository
   virtual Bool_t IsAvailable() const
   {
      return TestBit(kAvailable);
   }
   virtual void SetAvailable(Bool_t yes = kTRUE)
   {
      SetBit(kAvailable, yes);
   }
   virtual void CheckAvailable();

   virtual Bool_t HasDataType(const Char_t* data_type) const
   {
      // Returns kTRUE if data files of the given type are stored in the data repository
      // If data_type="*" this method always returns kTRUE

      KVString _dt = data_type;
      _dt.Remove(TString::kBoth, ' ');
      return (_dt == "*" || fDatatypes.Contains(_dt));
   }

   virtual void ls(Option_t* opt = "") const;
   virtual void Print(Option_t* opt = "") const;

   virtual void SetAnalysisTasks(const KVSeqCollection*);
   KVDataAnalysisTask* GetAnalysisTask(Int_t) const;
   KVDataAnalysisTask* GetAnalysisTask(const Char_t* keywords) const;
   virtual KVDataAnalysisTask GetAnalysisTaskAny(const Char_t* keywords) const;
   virtual Int_t GetNtasks() const;

   virtual TList* GetListOfAvailableSystems(const Char_t* datatype,
         KVDBSystem* systol = 0);
   virtual TList* GetListOfAvailableSystems(KVDataAnalysisTask* datan,
         KVDBSystem* systol = 0);

   void cd() const;

   const Char_t* GetDataSetDir() const;
   void SetName(const char* name);


   KVExpDB* GetDataBase(Option_t* opt = "") const;
   virtual void SaveDataBase() const;

   const Char_t* GetDataSetEnv(const Char_t* type, const Char_t* defval = "") const;
   Double_t GetDataSetEnv(const Char_t* type, Double_t defval) const;
   Bool_t GetDataSetEnv(const Char_t* type, Bool_t defval) const;

   TString GetFullPathToRunfile(const Char_t* type, Int_t run) const;
   const Char_t* GetRunfileName(const Char_t* type, Int_t run) const;
   TDatime GetRunfileDate(const Char_t* type, Int_t run);
   template <typename FileType>
   FileType* OpenRunfile(const Char_t* type, Int_t run)
   {
      return dynamic_cast<FileType*>(open_runfile(type, run));
   }
   TObject* OpenRunfile(const Char_t* type, Int_t run)
   {
      return open_runfile(type, run);
   }

   Bool_t CheckRunfileAvailable(const Char_t* type, Int_t run);
   void UpdateAvailableRuns(const Char_t* type);
   TFile* NewRunfile(const Char_t* type, Int_t run);
   void CommitRunfile(const Char_t* type, Int_t run, TFile* file);

   void CopyRunfilesFromRepository(const Char_t* type, KVNumberList runs, const Char_t* destdir);

   void DeleteRunfile(const Char_t* type, Int_t run, Bool_t confirm = kTRUE);
   void DeleteRunfiles(const Char_t* type, KVNumberList lrun = "", Bool_t confirm = kTRUE);
   KVNumberList GetRunList_DateSelection(const Char_t* type, TDatime* min = 0, TDatime* max = 0);
   KVNumberList GetRunList_StageSelection(const Char_t* other_type, const Char_t* base_type, KVDBSystem* sys = 0, Bool_t OnlyCol = kFALSE);
   KVNumberList GetRunList_VersionSelection(const Char_t* type, const Char_t* version, KVDBSystem* sys = 0);
   void SetRepository(KVDataRepository*);
   KVDataRepository* GetRepository() const;

   void CheckMultiRunfiles(const Char_t* data_type);
   void CleanMultiRunfiles(const Char_t* data_type, Bool_t confirm = kTRUE);

   Bool_t CheckRunfileUpToDate(const Char_t* data_type, Int_t run,
                               KVDataRepository* other_repos);
   Bool_t CheckRunfileUpToDate(const Char_t* data_type, Int_t run,
                               const Char_t* other_repos);
   void CheckUpToDate(const Char_t* data_type,
                      const Char_t* other_repos);
   KVNumberList GetUpdatableRuns(const Char_t* data_type,
                                 const Char_t* other_repos);

   KVNumberList GetRunList(const Char_t* data_type,
                           const KVDBSystem* sys = 0) const;

   virtual void MakeAnalysisClass(const Char_t* task, const Char_t* classname);

   Bool_t OpenDataSetFile(const Char_t* filename, std::ifstream& file);
   static Bool_t OpenDataSetFile(const TString& dataset, const Char_t* filename, std::ifstream& file);
   TString GetFullPathToDataSetFile(const Char_t* filename);
   static TString GetFullPathToDataSetFile(const TString& dataset, const Char_t* filename);
   static Bool_t FindDataSetFile(const TString& dataset, const Char_t* filename);
   Bool_t FindDataSetFile(const Char_t* filename);

   virtual Bool_t DataBaseNeedsUpdate() const;

   virtual const Char_t* GetReconstructedEventClassName() const
   {
      // Returns the name of the class used to store reconstructed events for this dataset.
      // This is defined by the value of environment variable
      //        [dataset name].ReconstructedEventClassName:      [name of class]
      // The default value (if no variable defined) is KVReconstructedEvent.

      return GetDataSetEnv("ReconstructedEventClassName", "KVReconstructedEvent");
   }

   TString GetOutputRepository(const Char_t* taskname) const;
   void CopyRunfilesToRepository(const Char_t* type, KVNumberList runs, const Char_t* destrepo);

   Bool_t HasCalibIdentInfos() const
   {
      // If calibration/identification parameters are available for this dataset
      // This is mostly important for filtering simulations: identification telescopes are only
      // considered 'ready for identification' if they have (at least one) associated identification
      // grid. If no identification grids exist, this condition should not be applied.
      return GetDataSetEnv("HasCalibIdentInfos", kTRUE);
   }

   ClassDef(KVDataSet, 2)       //Describes a set of experimental data which may be analysed with KaliVeda
};

//........ global variable
R__EXTERN KVDataSet* gDataSet;

#endif
