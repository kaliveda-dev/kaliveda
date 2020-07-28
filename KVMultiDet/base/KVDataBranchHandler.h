//Created by KVClassFactory on Fri Dec 11 11:52:02 2009
//Author: John Frankland,,,

#ifndef __KVDATABRANCHHANDLER_H
#define __KVDATABRANCHHANDLER_H

#include "KVBase.h"
#include "TTree.h"
#include "TBranch.h"

/**
  \class KVDataBranchHandler
  \brief Handles TTree branches for storing member variables of classes
  \ingroup Core
 */
class KVDataBranchHandler : public KVBase {
   TTree* fTree;              // the TTree
   void* fAddr;                 // address of variable
   TBranch* fBranch;       // the branch

public:
   KVDataBranchHandler(TTree*, TObject*, const Char_t*, const Char_t*, const Char_t* = "");
   virtual ~KVDataBranchHandler();

   Bool_t ConnectBranch();
   TBranch* CreateBranch();
   TBranch* GetBranch() const
   {
      return fBranch;
   };

   ClassDef(KVDataBranchHandler, 0) //Handles TTree branches for storing member variables of classes
};

#endif
