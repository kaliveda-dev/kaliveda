//Created by KVClassFactory on Fri Oct  8 14:27:07 2010
//Author: bonnet

#ifndef __KVPARTITIONLIST_H
#define __KVPARTITIONLIST_H

#include "KVUniqueNameList.h"

class KVIntegerList;
class KVPartition;
class TTree;

/**
   \class KVPartitionList
 \brief Manage a list of partitions
 \ingroup Analysis

Cette classe a toute son utilité si on s'attend a avoir dans une liste de partitions
une part importante de partition identique un exmeple dans KVBreakUp
-----
Via la methode Fill(KVIntegerList* )
on test la présence ou non d'une partition, si une partition identique est deja
presente, on incremente la population de celle-ci KVIntegerList::AddPopulation()
sinon on ajoute la partition dans la liste.
Le nombre de partition totale est accessible via GetNbreTot()
et le nombre de partitions différentes via GetNbreDiff()
-----
Apres le remplissage, l'utilisateur peut sauvegarder l'ensemble des partitions
dans un arbre et l'ecrire dans un fichier root via la methode SaveAsTree
*/

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
