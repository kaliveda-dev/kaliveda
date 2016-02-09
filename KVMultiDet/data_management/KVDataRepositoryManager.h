/*
$Id: KVDataRepositoryManager.h,v 1.4 2007/11/16 13:41:32 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/16 13:41:32 $
*/

//Created by KVClassFactory on Sun Jul 30 12:04:19 2006
//Author: John Frankland

#ifndef __KVDATAREPOSITORYMANAGER_H
#define __KVDATAREPOSITORYMANAGER_H

#include "TList.h"
class KVDataRepository;
class KVDataSet;

class KVDataRepositoryManager {
private:
   TList fRepositories;         //list of available repositories

protected:

public:

   KVDataRepositoryManager();
   virtual ~ KVDataRepositoryManager();

   void Init();
   const TList* GetListOfRepositories() const
   {
      return &fRepositories;
   };
   KVDataRepository* GetRepository(const Char_t* name) const;

   KVDataSet* GetDataSet(const Char_t* repository, const Char_t* dataset) const;

   void Print(Option_t* opt = "") const;

   ClassDef(KVDataRepositoryManager, 1)        //Handles available data repositories
};

//................  global variable
R__EXTERN KVDataRepositoryManager* gDataRepositoryManager;

#endif
