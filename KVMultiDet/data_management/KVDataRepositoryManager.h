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

/**
\class KVDataRepositoryManager
\brief Manages access to one or more data repositories
\ingroup DM

<p>
The data repository manager is set up and initialised in the following way:
</p>
<pre>
new KVDataRepositoryManager;
gDataRepositoryManager->Init();
</pre>
<p>
The available data repositories are defined in the user's $HOME/.kvrootrc file
(see <a href="KVDataRepository.html">KVDataRepository</a> for details
of configuration).</p>
<h4>Default data repository</h4>
<p>If more than one repository is defined, which one will be "active" (gDataRepository)
after initialisation of the data repository manager ? You can define the default repository
by setting the variable
</p>
<pre>
DataRepository.Default:   [name]
</pre>
<p>in your .kvrootrc file. If you don't, the default repository "by default" will be either:
the data repository named "default" if there is one; or the last one defined in your .kvrootrc.
</p>
*/

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
