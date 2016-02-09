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

class KVDataBase;
class KVDataSetManager;
class KVDataRepository;
class KVDBSystem;
class KVDataAnalysisTask;
class TObjArray;
class KVDBRun;
class KVAvailableRunsFile;

class KVDataSet: public KVBase {

   friend class KVAvailableRunsFile;
   friend class KVDataSetManager;

protected:
   KVDataRepository* fRepository;      //repository in which dataset is stored
   TString fDatatypes;            //list of types of data which are available for this dataset
   KVList fTasks;                //possible data analysis tasks for this dataset
   TString fCalibDir;           //directory containing database, calibration, identification parameters etc. for dataset
   Bool_t fDBBuild;             //has the database been built by us ?
   TFile* fDBase;               //file containing database
   TString fDBName;             //name of database
   TString fDBFileName;         //name of file in which database is stored on disk
   KVDataBase* fDataBase;       //pointer to dataset's database
   KVList fAvailableRuns;       //!list of KVAvailableRunsFile objects used to read infos on available runs
   enum {
      kAvailable = BIT(14)      //flag set if this dataset is physically present on local machine
   };
   TString fUserGroups;         //list of user groups allowed to read data

   virtual const Char_t* GetBaseFileName(const Char_t* type, Int_t run);

   virtual void OpenDataBase(Option_t* opt = "");
   virtual void OpenDBFile(const Char_t* full_path_to_dbfile);
   virtual void WriteDBFile(const Char_t* full_path_to_dbfile);
   void SetDBFileName(const Char_t* name)
   {
      fDBFileName = name;
   };
   const Char_t* GetDBFileName() const;
   void SetDBName(const Char_t* name)
   {
      fDBName = name;
   };
   const Char_t* GetDBName() const;
   TObject* Open(const Char_t* type, Int_t run, Option_t* opt = "");
   const Char_t* GetFullPathToDB();

public:

   KVAvailableRunsFile* GetAvailableRunsFile(const Char_t* type);

   KVDataSet();
   virtual ~ KVDataSet();

   virtual Bool_t CheckUserCanAccess();

   virtual void SetDataPathSubdir(const Char_t* s)
   {
      SetLabel(s);
   };
   // Returns name of top-level directory in data repository used to store data files for this dataset
   virtual const Char_t* GetDataPathSubdir() const
   {
      return GetLabel();
   };
   const Char_t* GetDataTypeSubdir(const Char_t* type) const
   {
      // returns name to be used for subdirectory corresponding to given data type
      KVString snom;
      snom.Form("KVDataSet.DataType.Subdir.%s", type);
      return GetDataSetEnv(snom.Data(), type);
   };
   virtual const Char_t* GetAvailableDataTypes() const
   {
      return fDatatypes.Data();
   };
   virtual void AddAvailableDataType(const Char_t*);
   virtual void SetUserGroups(const Char_t* groups)
   {
      fUserGroups = groups;
   };
   // Returns kTRUE if this dataset is available for analysis, i.e. if any associated data files are stored in the data repository
   virtual Bool_t IsAvailable() const
   {
      return TestBit(kAvailable);
   };
   virtual void SetAvailable(Bool_t yes = kTRUE)
   {
      SetBit(kAvailable, yes);
   };
   virtual void CheckAvailable();

   virtual Bool_t HasDataType(const Char_t* data_type)
   {
      // Returns kTRUE if data files of the given type are stored in the data repository
      KVString _dt = data_type;
      _dt.Remove(TString::kBoth, ' ');
      return fDatatypes.Contains(_dt);
   };

   virtual void ls(Option_t* opt = "") const;
   virtual void Print(Option_t* opt = "") const;

   virtual void SetAnalysisTasks(const KVSeqCollection*);
   virtual KVDataAnalysisTask* GetAnalysisTask(Int_t) const;
   virtual KVDataAnalysisTask* GetAnalysisTask(const Char_t* keywords) const;
   virtual KVDataAnalysisTask* GetAnalysisTaskAny(const Char_t* keywords) const;
   virtual Int_t GetNtasks() const;

   virtual TList* GetListOfAvailableSystems(const Char_t* datatype,
         KVDBSystem* systol = 0);
   virtual TList* GetListOfAvailableSystems(KVDataAnalysisTask* datan,
         KVDBSystem* systol = 0);

   void cd();

   const Char_t* GetDataSetDir() const;
   void SetName(const char* name);


   KVDataBase* GetDataBase(Option_t* opt = "");
   virtual void SaveDataBase();

   const Char_t* GetDataSetEnv(const Char_t* type, const Char_t* defval = "") const;
   Double_t GetDataSetEnv(const Char_t* type, Double_t defval) const;
   Bool_t GetDataSetEnv(const Char_t* type, Bool_t defval) const;

   TString GetFullPathToRunfile(const Char_t* type, Int_t run);
   const Char_t* GetRunfileName(const Char_t* type, Int_t run);
   TDatime GetRunfileDate(const Char_t* type, Int_t run);
   virtual TObject* OpenRunfile(const Char_t* type, Int_t run);
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
                           const KVDBSystem* sys = 0);

   virtual void MakeAnalysisClass(const Char_t* task, const Char_t* classname);

   virtual Bool_t OpenDataSetFile(const Char_t* filename, std::ifstream& file);
   virtual Bool_t DataBaseNeedsUpdate();

   virtual const Char_t* GetReconstructedEventClassName() const
   {
      // Returns the name of the class used to store reconstructed events for this dataset.
      // This is defined by the value of environment variable
      //        [dataset name].ReconstructedEventClassName:      [name of class]
      // The default value (if no variable defined) is KVReconstructedEvent.

      return GetDataSetEnv("ReconstructedEventClassName", "KVReconstructedEvent");
   };

   ClassDef(KVDataSet, 2)       //Describes a set of experimental data which may be analysed with KaliVeda
};

//........ global variable
R__EXTERN KVDataSet* gDataSet;

#endif
