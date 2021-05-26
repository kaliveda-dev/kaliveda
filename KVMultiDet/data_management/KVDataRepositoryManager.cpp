/*
$Id: KVDataRepositoryManager.cpp,v 1.9 2007/11/20 16:46:21 franklan Exp $
$Revision: 1.9 $
$Date: 2007/11/20 16:46:21 $
*/

//Created by KVClassFactory on Sun Jul 30 12:04:19 2006
//Author: John Frankland

#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "TEnv.h"
#include "KVBase.h"
#include "Riostream.h"
#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "KVDataSet.h"

KVDataRepositoryManager* gDataRepositoryManager;

using namespace std;

ClassImp(KVDataRepositoryManager)

KVDataRepositoryManager::KVDataRepositoryManager()
{
   //Default constructor
   fRepositories.SetOwner(kTRUE);
   gDataRepositoryManager = this;
}

KVDataRepositoryManager::~KVDataRepositoryManager()
{
   //Destructor
   fRepositories.Delete();      //destroy all data repositories
   gDataRepositoryManager = 0;
}

//______________________________________________________________________________

void KVDataRepositoryManager::Init()
{
   //Read .kvrootrc and set up all data repositories it defines.
   //
   //The default data repository (gDataRepository) is defined by the environment variable
   //DataRepository.Default (default value = "default").
   //
   //For this repository, KVDataRepository::cd() will be called and
   //gDataRepository and gDataSetManager will point, respectively, to the repository and
   //to its data set manager.
   //
   //If the repository corresponding to DataRepository.Default is not found,
   //the last repository defined in the .kvrootrc file will be made default.

   //make sure KaliVeda environment is initialised
   KVBase::InitEnvironment();

   //delete any previously defined repositories
   if (fRepositories.GetSize())
      fRepositories.Delete();

   //get whitespace-separated list of data repository names
   TString rep_list = gEnv->GetValue("DataRepository", "");
   if (rep_list == "") {
      cout <<
           "<KVDataRepositoryManager::Init> : no repositories defined in .kvrootrc"
           << endl;
      // Initialise a repository-less dataset manager which will give access to
      // all datasets regardless of data availability (which is unknowable)
      gDataSetManager = new KVDataSetManager;
      gDataSetManager->Init();
      return;
   }
   //split list
   TObjArray* toks = rep_list.Tokenize(' ');
   KVDataRepository* new_rep = 0;
   KVDataRepository* last_defined = 0;
   for (int i_rep = 0; i_rep < toks->GetEntries(); i_rep++) {

      //loop over each defined repository

      TString rep_name = ((TObjString*)(*toks)[i_rep])->String();       //name of repository
      //look for repository type
      TString rep_type =
         gEnv->GetValue(Form("%s.DataRepository.Type", rep_name.Data()),
                        "local");
      rep_type.ToLower();

      //create new repository
      new_rep = KVDataRepository::NewRepository(rep_type.Data());
      new_rep->SetName(rep_name.Data());
      //new_rep->SetType(rep_type.Data()); 'type' is set in default ctor of each repository class
      if (new_rep->Init()) {
         fRepositories.Add(new_rep);
         last_defined = new_rep;    //keep pointer to last defined repository
      }
      else {
         //problem with initialisation of data repository.
         //it is ignored.
         delete new_rep;
         new_rep = 0;
      }

   }
   delete toks;
   //look for 'default' repository
   new_rep = (KVDataRepository*) fRepositories.FindObject(
                gEnv->GetValue("DataRepository.Default", "default"));
   if (new_rep) new_rep->cd();
   else if (last_defined) last_defined->cd();
}

//______________________________________________________________________________

KVDataRepository* KVDataRepositoryManager::GetRepository(const Char_t*
      name) const
{
   //Return pointer to data repository with given name.
   //Data repository names are defined in .kvrootrc file by lines such as
   //
   //DataRepository: default
   //+DataRepository: ccali

   return (KVDataRepository*) fRepositories.FindObject(name);
}

//______________________________________________________________________________

void KVDataRepositoryManager::Print(Option_t* opt) const
{
   //Print list of repositories
   //opt = "all" : print full configuration information for each repository
   KVDataRepository* rep;
   TString _opt(opt);
   _opt.ToUpper();
   Bool_t _all = (_opt == "ALL");
   TIter nxt(&fRepositories);
   cout << "Available data repositories: " << endl << endl;
   while ((rep = (KVDataRepository*) nxt())) {
      if (_all) {
         rep->Print();
      }
      else {
         cout << "\t" << rep->GetName() << "  [";
         if (rep->IsRemote())
            cout << "REMOTE";
         else
            cout << "LOCAL";
         cout << "] : ";
         cout << rep->GetRootDirectory() << endl;
      }
   }
}

//______________________________________________________________________________

KVDataSet* KVDataRepositoryManager::GetDataSet(const Char_t* repository, const Char_t* dataset) const
{
   // Return pointer to named dataset in the given repository
   if (KVDataRepository* R = GetRepository(repository)) {

      return R->GetDataSetManager()->GetDataSet(dataset);

   }
   return 0;
}
