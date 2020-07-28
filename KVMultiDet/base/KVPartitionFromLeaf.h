//Created by KVClassFactory on Fri May  7 11:30:10 2010
//Author: bonnet

#ifndef __KVPARTITIONFROMLEAF_H
#define __KVPARTITIONFROMLEAF_H

#include "KVPartition.h"
#include "TLeaf.h"

/**
  \class KVPartitionFromLeaf
\brief Derived form KVPartition to optimize filling from a tree
\ingroup Analysis

- Utilisation avec un arbre (TTree* )contenant des branches de ce type
avec par evt un tableau de valeur de dimension fixe ou non
Exemple:
OBJ: TTree  tree  tree : 0 at: 0x9cfc7a8
 OBJ: TBranch  mt mt/I : 0 at: 0x9cfcdc8
 OBJ: TBranch  tabz  tabz[mt]/I : 0 at: 0x9d00120
Attention: cette classe n'est pas adapte au TChain

- Methode ConnectToLeaf(TLeaf* lf) permet de lier la KVPartition a la branche tableau (tree->GetLeaf("tabz"))
Exemple:
KVPartitionFromLeaf* par = new KVPartitionFromLeaf();
par->ConnectToLeaf(tree->GetLeaf("tabz"));

- Methode KVPartitionFromLeaf::SetZminZmax permet de definir des limites inf ou sup des tailles des fragments lors du remplissage de la partition, à definir
avant d'utiliser la methode CheckForUpdate();

- Méthode KVPartitionFromLeaf::ReadEntry(Long64_t entry), jour le role de remplissage de la partition et aussi du calcul des valeurs associées
On peut incrementer les entrees de l'arbre en meme temps ou non (defaut)
Exemple:
for (Int_t ii=0;ii<tt->GetEntries();ii+=1){

   tree->GetEntry(ii);
   ...
   ...
   par->CheckForUpdate();

}
ou
for (Int_t ii=0;ii<tt->GetEntries();ii+=1){

   par->ReadEntry(ii);

}
*/
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
