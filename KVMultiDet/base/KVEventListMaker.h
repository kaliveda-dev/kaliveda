/*
$Id: KVEventListMaker.h,v 1.2 2008/12/17 15:27:25 ebonnet Exp $
$Revision: 1.2 $
$Date: 2008/12/17 15:27:25 $
*/

//Created by KVClassFactory on Thu Mar 20 11:58:48 2008
//Author: Eric Bonnet

#ifndef __KVEVENTLISTMAKER_H
#define __KVEVENTLISTMAKER_H

#include "KVList.h"
#include "KVString.h"

/**
   \class KVEventListMaker
   \brief Compute TEventList for TTree
   \ingroup Analysis

Inputs are filename, treename and space-separated list of branchnames.

Only branches containing integer values are handled.

All tree entries are read and TEventList are created when a new value of concerned
branches are found.
*/

class KVEventListMaker {

protected:
   KVString ktname;  //name of the tree
   KVString kfname;  //name of the file
   KVString kbname;  //name of branches, si plusieurs branches sont concernees
   //il faut separer les noms par un espace exemple "module couronne"
   Bool_t ktag_tree;

   void init()
   {
      //initialisation des variables
      ktname = kfname = kbname = "";
      ktag_tree = kFALSE;
   }

public:
   KVEventListMaker()
   {
      //default constructor
      init();
   }
   virtual ~KVEventListMaker()
   {
      //destructor
   };

   void Clear()
   {
      /*do nothing*/
   }

   void SetTreeName(KVString name)
   {
      ktname = name;
   }
   KVString GetTreeName()
   {
      return ktname;
   }

   void SetBranchName(KVString name)
   {
      kbname = name;
   }
   KVString GetBranchName()
   {
      return kbname;
   }

   void SetFileName(KVString name)
   {
      kfname = name;
   }
   KVString GetFileName()
   {
      return kfname;
   }

   void TagWithTreeName(Bool_t tag = kTRUE)
   {
      ktag_tree = tag;
   }

   Bool_t IsReady()
   {
      //just check if all fields have been set
      Bool_t temp = kTRUE;
      temp &= (GetFileName() != "");
      temp &= (GetTreeName() != "");
      temp &= (GetBranchName() != "");
      return temp;
   }

   void Process();

   ClassDef(KVEventListMaker, 1) //compute TEventList on trees

};

#endif
