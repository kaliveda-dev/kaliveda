/*
$Id: KVBatchSystemManager.cpp,v 1.4 2007/11/20 16:45:22 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/20 16:45:22 $
*/

//Created by KVClassFactory on Mon May  7 17:29:05 2007
//Author: franklan

#include "KVBatchSystemManager.h"
#include "KVString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TEnv.h"

using namespace std;

ClassImp(KVBatchSystemManager)



KVBatchSystemManager* gBatchSystemManager = 0;

KVBatchSystemManager::KVBatchSystemManager()
{
   //Default constructor

   if (gBatchSystemManager) return;

   Init();
   gBatchSystemManager = this;
}

KVBatchSystemManager::~KVBatchSystemManager()
{
   //Destructor
   gBatchSystemManager = 0;
   gBatchSystem = 0;
}

void KVBatchSystemManager::Init()
{
   //Set up list of available batch systems

   gBatchSystem = 0;

   KVString list = gEnv->GetValue("BatchSystem", "");

   fBatchSystems.Clear();

   unique_ptr<TObjArray> systems(list.Tokenize(" "));
   TIter next(systems.get());
   TObjString* batch_sys;
   while ((batch_sys = (TObjString*)next())) {

      KVBatchSystem* bs = KVBatchSystem::GetBatchSystem(batch_sys->GetString().Data());
      fBatchSystems.Add(bs);
   }

   //set default
   fDefault = (KVBatchSystem*)fBatchSystems.FindObjectByName(gEnv->GetValue("Default.BatchSystem", ""));
}

KVBatchSystem* KVBatchSystemManager::GetBatchSystem(const Char_t* name)
{
   //Get batch system by name
   return (KVBatchSystem*)fBatchSystems.FindObjectByName(name);
}

KVBatchSystem* KVBatchSystemManager::GetBatchSystem(Int_t index)
{
   //Get batch system by number in the list printed by Print()
   return (KVBatchSystem*)fBatchSystems.At(index - 1);
}

void KVBatchSystemManager::Print(Option_t* opt) const
{
   //Print list of available batch systems
   // Default is to show numbered list with title of each batch system
   //if opt="all", print detailed information on each batch system
   if (!strcmp(opt, "all")) {
      for (int i = 1; i <= fBatchSystems.GetSize(); i++) {
         fBatchSystems.At(i - 1)->Print("all");
      }
   }
   else {
      for (int i = 1; i <= fBatchSystems.GetSize(); i++) {
         cout << "\t" << i << ". " << ((KVBatchSystem*)fBatchSystems.At(i - 1))->GetTitle() << endl;
      }
   }
}
