//Created by KVClassFactory on Tue Jul 12 11:43:52 2016
//Author: bonnet,,,

#ifndef __KVEXPDB_H
#define __KVEXPDB_H

#include "KVDataBase.h"
#include "KVDBRun.h"

class KVDBSystem;

class KVExpDB : public KVDataBase {

protected:

   Int_t kFirstRun;
   Int_t kLastRun;
   KVDBTable* fRuns;            //-> table of runs
   KVDBTable* fSystems;         //-> table of systems

   Bool_t OpenCalibFile(const Char_t* type, std::ifstream& fs) const;
   virtual void ReadSystemList();
   void init();

public:

   KVExpDB();
   KVExpDB(const Char_t* name);
   KVExpDB(const Char_t* name, const Char_t* title);

   virtual ~KVExpDB();

   virtual void LinkListToRunRanges(TList* list, UInt_t rr_number,
                                    UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRanges(KVDBRecord* rec, UInt_t rr_number,
                                      UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRange(KVDBRecord* rec, UInt_t first_run,
                                     UInt_t last_run);
   virtual void LinkListToRunRange(TList* list, KVNumberList nl);
   virtual void LinkRecordToRunRange(KVDBRecord* rec,  KVNumberList nl);
   virtual void LinkRecordToRun(KVDBRecord* rec,  Int_t run);

   void AddRun(KVDBRun* r) {
      fRuns->AddRecord(r);
   }
   virtual KVSeqCollection* GetRuns() const {
      return fRuns->GetRecords();
   }
   KVDBRun* GetDBRun(Int_t number) const {
      return (KVDBRun*)fRuns->GetRecord(number);
   }
   Int_t GetFirstRunNumber() const {
      return kFirstRun;
   }
   Int_t GetLastRunNumber() const {
      return kLastRun;
   }

   virtual KVDBSystem* GetSystem(const Char_t* system) const {
      return (KVDBSystem*) fSystems->GetRecord(system);
   }
   virtual KVSeqCollection* GetSystems() const {
      return fSystems->GetRecords();
   }

   void AddSystem(KVDBSystem* r) {
      fSystems->AddRecord(r);
   }
   void WriteSystemsFile() const;
   void WriteRunListFile() const;

   virtual void Save(const Char_t*);

   virtual const Char_t* GetDBEnv(const Char_t*) const;
   const Char_t* GetCalibFileName(const Char_t* type) const {
      return GetDBEnv(type);
   }

   virtual void WriteObjects(TFile*) {}
   virtual void ReadObjects(TFile*) {}
   virtual void PrintRuns(KVNumberList&) const;

   ClassDef(KVExpDB, 1) //base class to describe database of an experiment
};

#endif
