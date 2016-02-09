/*
$Id: KVDataSetManager.h,v 1.9 2007/09/20 11:30:17 franklan Exp $
$Revision: 1.9 $
$Date: 2007/09/20 11:30:17 $
$Author: franklan $
*/

#ifndef __KVDATASETMAN_H
#define __KVDATASETMAN_H

#include "KVUniqueNameList.h"
#include "KVNameValueList.h"

#include "KVDataSet.h"
#include "KVDataRepository.h"
#include "KVDataAnalysisTask.h"

class KVDataSetManager {

   friend class KVDataSet;

protected:
   std::ifstream fDatasets;          //for reading cached repository available datasets file
   KVUniqueNameList fDataSets;            //list of datasets handled by manager
   KVUniqueNameList fTasks;               //list of all known analysis tasks
   Int_t fNavailable;           //number of available datasets
   Int_t* fIndex;               //array of indices of available datasets
   KVNameValueList fUserGroups;    //list of user groups

   virtual Bool_t ReadDataSetList();
   virtual Bool_t ReadTaskList();
   virtual void ReadUserGroups();

   virtual KVDataSet* NewDataSet();
   KVDataRepository* fRepository;       //the repository for which data sets are handled

   const KVSeqCollection* GetAnalysisTaskList() const
   {
      return &fTasks;
   };
   virtual Bool_t OpenAvailableDatasetsFile();
   virtual Bool_t ReadAvailableDatasetsFile();
   Bool_t fCacheAvailable;//kTRUE if caching is activated for parent repository
   UInt_t fMaxCacheTime;//maximum allowed age of cache file in seconds
   TString fCacheFileName;//name of cache file ( = [repository name].available.datasets)
   virtual Bool_t CheckCacheStatus();

public:

   KVDataSetManager();
   virtual ~ KVDataSetManager();

   virtual Bool_t Init(KVDataRepository* /*rep*/ = 0);
   virtual void CheckAvailability();
   virtual void Print(Option_t* opt = "") const;
   virtual KVDataSet* GetDataSet(Int_t) const;
   virtual KVDataSet* GetDataSet(const Char_t*);
   virtual KVDataSet* GetAvailableDataSet(Int_t) const;
   virtual Int_t GetNavailable() const
   {
      return fNavailable;
   };
   virtual Int_t GetNtotal() const
   {
      return fDataSets.GetSize();
   };
   const KVNameValueList& GetUserGroups()
   {
      return fUserGroups;
   };
   virtual Bool_t CheckUser(const Char_t* groupname,
                            const Char_t* username = "");

   virtual KVDataAnalysisTask* GetTask(const Char_t* name);

   virtual void Update();

   ClassDef(KVDataSetManager, 3)        //Handles datasets in a data repository
};

//................  global variable
R__EXTERN KVDataSetManager* gDataSetManager;

#endif
