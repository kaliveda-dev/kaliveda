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

/**
\class KVINDRAPulserDataTree
\brief Handles TTree with mean pulser data for every run
\ingroup INDRADB

 Create and fill tree with pulser data.
 We look for the following two directories:
<pre>
 $KVROOT/KVFiles/name_of_dataset/gene_detecteurs
 $KVROOT/KVFiles/name_of_dataset/gene_pins
</pre>
 If not found, we look for the following compressed archives:
<pre>
 $KVROOT/KVFiles/name_of_dataset/gene_detecteurs.tgz
 $KVROOT/KVFiles/name_of_dataset/gene_pins.tgz
</pre>
 and if found, uncompress them ('tar -zxf').
<br>
 The default names of these directories are defined in .kvrootrc by:
<pre>
 KVINDRAPulserDataTree.GeneDetDir:   gene_detecteurs
 KVINDRAPulserDataTree.GenePinDir:   gene_pins
</pre>
 Dataset-dependent alternatives can be defined using:
<pre>
 dataset_name.KVINDRAPulserDataTree.GeneDetDir:   dataset_specific_value
</pre>
 The first directory (gene_detecteurs) must contain 1 file per run with names like:
<pre>
   run8820.gene
</pre>
 These files are generated using example analysis class GetGeneMean
 (see Examples). They contain the mean value of every acquisition parameter
 associated with a detector in the run:
<pre>
   CI_0201_GG     3095.28
   CI_0201_PG     275.626
   CI_0203_GG     2863.66
   CI_0203_PG     263.308
   CI_0205_GG     3042.83
   etc.
</pre>
 i.e. 'name of acquisition parameter'    'mean value for run'
<br>
 The second directory (gene_pins) must contain 1 file per run with names like:
<pre>
   run8820.genepin
 OR   run8820.laserpin
 OR   run8820.genelaserpin
</pre>
 These files are generated using example analysis class GetGeneMeanPin
 (see Examples). They contain the mean values of the acquisition parameters
 associated with pin diodes in the run:
<pre>
   PILA_01_PG_gene     792.616
   PILA_01_PG_laser     1747.18
   PILA_01_GG_gene     0
   etc.
</pre>
 We create a TTree with 1 branch for each detector acquisition parameter.
 For PILA and SI_PIN parameters, we create a 'gene' and a 'laser' branch for each.
*/

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
