/*
$Id: KVINDRAPulserDataTree.h,v 1.5 2009/03/27 16:42:58 franklan Exp $
$Revision: 1.5 $
$Date: 2009/03/27 16:42:58 $
*/

//Created by KVClassFactory on Wed Jan 21 11:56:26 2009
//Author: franklan

#ifndef __KVINDRAPULSERDATATREE_H
#define __KVINDRAPULSERDATATREE_H

#include "KVBase.h"
#include "TTree.h"
#include "THashTable.h"
#include "Riostream.h"
#include "KVString.h"
#include "KVTarArchive.h"

#include <KVSeqCollection.h>

class TFile;

class KVINDRAPulserDataTree : public KVBase {
protected:
   TTree* fArb;//!tree containing pulser data
   KVTarArchive* fGeneDir;//directory/archive containing gene data
   KVTarArchive* fPinDir;//directory/archive containing pin data
   Int_t fRun;//!run number used to build tree
   Float_t* fVal;//!array of floats used to fill tree
   Int_t fTab_siz;//!size of array
   THashTable* fIndex;//!associate name of branch to index in fVal
   KVSeqCollection* fRunlist;//!list of runs given by database

   TString GetDirectoryName(const Char_t*);
   void CreateTree();
   void ReadData();
   UChar_t ReadData(Int_t);
   void ReadFile(std::ifstream&);
   Bool_t OpenGeneData(Int_t, std::ifstream&);
   Bool_t OpenPinData(Int_t, std::ifstream&);

public:
   KVINDRAPulserDataTree();
   virtual ~KVINDRAPulserDataTree();

   virtual void Build();
   TTree* GetTree() const
   {
      return fArb;
   };

   void ReadTree(TFile*);
   void WriteTree(TFile*);

   Int_t GetIndex(const Char_t* branchname) const
   {
      if (fIndex) {
         KVBase* iob = (KVBase*)fIndex->FindObject(branchname);
         if (iob) return (Int_t)iob->GetNumber();
      }
      return -1;
   };
   void SetRunList(KVSeqCollection* runs)
   {
      fRunlist = runs;
   };

   Float_t GetMean(const Char_t*, Int_t);

   ClassDef(KVINDRAPulserDataTree, 1) //Handles TTree with mean pulser data for every run
};

#endif
