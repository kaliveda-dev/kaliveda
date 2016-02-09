//Created by KVClassFactory on Fri May  7 11:30:10 2010
//Author: bonnet

#ifndef __KVPARTITIONFROMLEAF_H
#define __KVPARTITIONFROMLEAF_H

#include "KVPartition.h"
#include "TLeaf.h"

class KVPartitionFromLeaf : public KVPartition {

protected:
   TLeaf* linked_leaf;
   Int_t  zmin, zmax;
   Bool_t select_min, select_max;

public:
   KVPartitionFromLeaf();
   virtual ~KVPartitionFromLeaf();

   void ConnectToLeaf(TLeaf* lf);
   void ReadEntry(Long64_t entry = -1);

   void SetZmin(Int_t val);
   void SetZmax(Int_t val);
   void SetZminZmax(Int_t vmin, Int_t vmax);


   ClassDef(KVPartitionFromLeaf, 1) //Derived form KVPartition to optimize filling from a tree
};

#endif
