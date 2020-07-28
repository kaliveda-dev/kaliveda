//Created by KVClassFactory on Fri Dec 11 11:52:02 2009
//Author: John Frankland,,,

#include "KVDataBranchHandler.h"

ClassImp(KVDataBranchHandler)

KVDataBranchHandler::KVDataBranchHandler(TTree* tree, TObject* obj, const Char_t* var,
      const Char_t* type, const Char_t* prefix) :
   KVBase()
{
   // Initialise branch handler for given TTree, object, member variable named 'var',
   // of leaf type 'type' (see TBranch constructor) and an optional branch prefix p
   // (will be prepended as 'p.').

   fTree = tree;
   Long_t offset = obj->IsA()->GetDataMemberOffset(var);
   if (!offset) {
      Error("KVDataBranchHandler", "Offset for member %s of %s is 0", var, obj->GetName());
      fAddr = 0;
      fBranch = 0;
      return;
   }
   fAddr = (void*)((Long_t)obj + offset);
   if (strcmp(prefix, "")) SetName(Form("%s.%s.%s", prefix, obj->GetName(), var));
   else SetName(Form("%s.%s", obj->GetName(), var));
   SetType(Form("%s/%s", GetName(), type));
   fBranch = fTree->GetBranch(GetName());
   if (!ConnectBranch()) CreateBranch();
}

KVDataBranchHandler::~KVDataBranchHandler()
{
   // Destructor
}

Bool_t KVDataBranchHandler::ConnectBranch()
{
   // Connect member variable to branch for reading.
   // If branch exists returns kTRUE.
   // If branch does not exist, returns kFALSE.

   if (!fBranch) return kFALSE;
   fBranch->SetAddress(fAddr);
   return kTRUE;
}

TBranch* KVDataBranchHandler::CreateBranch()
{
   // Create new branch in TTree for member variable and return its address.

   if (!fAddr) return 0;
   //Info("CreateBranch", "fTree->Branch(\"%s\", %p, \"%s\")", GetName(), fAddr, GetType());
   return (fBranch = fTree->Branch(GetName(), fAddr, GetType()));
}

