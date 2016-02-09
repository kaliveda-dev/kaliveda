//Created by KVClassFactory on Fri Oct  8 14:27:07 2010
//Author: bonnet

#ifndef __KVPARTITIONLIST_H
#define __KVPARTITIONLIST_H

#include "KVUniqueNameList.h"

class KVIntegerList;
class KVPartition;
class TTree;

class KVPartitionList : public KVUniqueNameList {
protected:

   Bool_t  atrouve;
   Double_t knbre_diff;    //    Nbre de partitions differentes
   Double_t knbre_tot;     //    Nbre de partitions totales
   KVPartition* mult_range;   //->   Permet d'extraire la gamme en multiplicité

   void init();
   void ValidateEntrance(KVIntegerList* il);
   TTree* GenereTree(const Char_t* treename, Bool_t Compress = kTRUE);

public:

   KVPartitionList(const Char_t* name);
   KVPartitionList();
   virtual ~KVPartitionList();
   void Clear(Option_t* option = "");

   Bool_t IsInTheList();

   Double_t GetNbreTot();
   Double_t GetNbreDiff();

   void Update();

   virtual Bool_t Fill(KVIntegerList* par);

   virtual void AddFirst(TObject* obj);
   virtual void AddLast(TObject* obj);
   virtual void AddAt(TObject* obj, Int_t idx);
   virtual void AddAfter(const TObject* after, TObject* obj);
   virtual void AddBefore(const TObject* before, TObject* obj);
   virtual void Add(TObject* obj);

   void SaveAsTree(const Char_t* filename, const Char_t* treename, Bool_t Compress = kTRUE, Option_t* option = "recreate");

   ClassDef(KVPartitionList, 1) //Store KVIntegerList and increment its population, if one is already in the list

};

#endif
