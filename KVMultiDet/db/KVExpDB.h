//Created by KVClassFactory on Tue Jul 12 11:43:52 2016
//Author: bonnet,,,

#ifndef __KVEXPDB_H
#define __KVEXPDB_H

#include "KVDataBase.h"
#include "KVDBRun.h"

class KVDBSystem;

/**
\class KVExpDB
\brief Base class to describe database of an experiment
\ingroup DM,Calibration
*/

class KVExpDB : public KVDataBase {

protected:
   TString fDataSet;//the name of the dataset to which this database is associated
   TString fDataSetDir;//the directory containing the dataset files
   TString fDBType;//used by GetDBEnv

   KVNumberList fListOfRuns;//list of all run numbers
   KVDBTable* fRuns;            //-> table of runs
   KVDBTable* fSystems;         //-> table of systems

   Bool_t OpenCalibFile(const Char_t* type, std::ifstream& fs) const;
   virtual void ReadSystemList();
   virtual void ReadComments();
   void init();

public:
   KVExpDB();
   KVExpDB(const Char_t* name);
   KVExpDB(const Char_t* name, const Char_t* title);

   void SetDataSet(const TString& s)
   {
      fDataSet = s;
   }
   TString GetDBType() const
   {
      return fDBType;
   }
   void SetDBType(const TString& s)
   {
      fDBType = s;
   }

   virtual ~KVExpDB();

   virtual void LinkListToRunRanges(TList* list, UInt_t rr_number,
                                    UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRanges(KVDBRecord* rec, UInt_t rr_number,
                                      UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRange(KVDBRecord* rec, UInt_t first_run,
                                     UInt_t last_run);
   virtual void LinkListToRunRange(TList* list, const KVNumberList& nl);
   virtual void LinkRecordToRunRange(KVDBRecord* rec,  const KVNumberList& nl);
   virtual void LinkRecordToRun(KVDBRecord* rec,  Int_t run);

   void AddRun(KVDBRun* r)
   {
      fRuns->AddRecord(r);
      fListOfRuns.Add(r->GetNumber());
   }
   virtual KVSeqCollection* GetRuns() const
   {
      return fRuns->GetRecords();
   }
   KVDBRun* GetDBRun(Int_t number) const
   {
      return (KVDBRun*)fRuns->GetRecord(number);
   }
   const KVNumberList& GetRunList() const
   {
      // list of numbers of all runs in database
      return fListOfRuns;
   }
   virtual KVDBSystem* GetSystem(const Char_t* system) const
   {
      return (KVDBSystem*) fSystems->GetRecord(system);
   }
   virtual KVSeqCollection* GetSystems() const
   {
      return fSystems->GetRecords();
   }

   void AddSystem(KVDBSystem* r)
   {
      fSystems->AddRecord(r);
   }
   void RemoveSystem(KVDBSystem* s)
   {
      fSystems->RemoveRecord(s);
   }

   void WriteSystemsFile() const;
   void WriteRunListFile() const;

   virtual void Save(const Char_t*);

   virtual const Char_t* GetDBEnv(const Char_t*) const;
   const Char_t* GetCalibFileName(const Char_t* type) const
   {
      return GetDBEnv(type);
   }

   virtual void WriteObjects(TFile*)
   {
      // Abstract method. Can be overridden in child classes.
      // When the database is written to disk (by the currently active dataset, see
      // KVDataSet::WriteDBFile) any associated objects (histograms, trees, etc.)
      // can be written using this method.
      // The pointer to the file being written is passed as argument.
   }

   virtual void ReadObjects(TFile*)
   {
      // Abstract method. Can be overridden in child classes.
      // When the database is read from disk (by the currently active dataset, see
      // KVDataSet::OpenDBFile) any associated objects (histograms, trees, etc.)
      // stored in the file can be read using this method.
      // The pointer to the file being read is passed as argument.
   }

   virtual void PrintRuns(KVNumberList&) const;

   virtual void cd();

   const Char_t* GetDataSetDir() const
   {
      return fDataSetDir;
   }
   void SetDataSetDir(const Char_t* d)
   {
      fDataSetDir = d;
   }
   virtual void Build()
   {
      AbstractMethod("Build");
   }

   static KVExpDB* MakeDataBase(const Char_t* name, const Char_t* datasetdir);

   ULong64_t GetTotalEvents(int first_run, int last_run = -1) const;
   ULong64_t GetTotalEvents(const KVString& system) const;

   ClassDef(KVExpDB, 2) //base class to describe database of an experiment
};

//........ global variable
R__EXTERN KVExpDB* gExpDB;

#endif
