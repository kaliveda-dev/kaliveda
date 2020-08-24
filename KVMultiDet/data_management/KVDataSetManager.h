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
#include <vector>
#include "KVDataSet.h"
#include "KVDataRepository.h"
#include "KVDataAnalysisTask.h"

/**
  \class KVDataSetManager
  \brief Manage all datasets contained in a given data repository
  \ingroup DM

KVDataSetManager handles a collection of  datasets contained in a data repository,
which may be analysed with KaliVeda.
The list of all known datasets is defined in $KVROOT/KVFiles/.kvrootrc.
Some of these datasets may not be physically present in the repository. This is checked
by method CheckAvailability(). This will search in the data repository for the subdirectories
corresponding to each dataset, and afterwards KVDataSet::IsAvailable() will return kTRUE
or kFALSE for each dataset depending on the result of the search.

ACTIVE DATA REPOSITORY/DATASET MANAGER
=====================================
Each data repository has a data set manager. At any time, one repository is designated as
being 'active' or 'current', and it's address is held in the global pointer gDataRepository.
One can then access the corresponding dataset manager through gDataSetManager.
*/

class KVDataSetManager {

   friend class KVDataSet;

protected:
   std::ifstream fDatasets;          //for reading cached repository available datasets file
   KVUniqueNameList fDataSets;            //list of datasets handled by manager
   KVUniqueNameList fTasks;               //list of all known analysis tasks
   Int_t fNavailable;           //number of available datasets
   std::vector<Int_t> fIndex;               //array of indices of available datasets
   KVNameValueList fUserGroups;    //list of user groups

   virtual Bool_t ReadDataSetList();
   virtual Bool_t ReadTaskList();
   virtual void ReadUserGroups();

   virtual KVDataSet* NewDataSet();
   KVDataRepository* fRepository;       //the repository for which data sets are handled

   const KVSeqCollection* GetAnalysisTaskList() const
   {
      return &fTasks;
   }
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
   KVDataSet* GetDataSet(Int_t) const;
   KVDataSet* GetDataSet(const Char_t*);
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
   KVDataAnalysisTask* GetAnalysisTaskAny(const Char_t* keywords) const;

   ClassDef(KVDataSetManager, 3)        //Handles datasets in a data repository
};

//................  global variable
R__EXTERN KVDataSetManager* gDataSetManager;

#endif
