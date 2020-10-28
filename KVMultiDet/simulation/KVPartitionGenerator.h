//Created by KVClassFactory on Thu Jun 17 14:20:16 2010
//Author: bonnet

#ifndef __KVPartitionGenerator_H
#define __KVPartitionGenerator_H
#include "KVString.h"
#include "KVUniqueNameList.h"

class TTree;

/**
\class KVPartitionGenerator
\brief Permet de determiner numeriquement et exactement un ensemble de partitions d'entrees remplissante certaines conditions
\ingroup Simulation

Les differentes recontrees grandeurs sont :
- Ztot (Zt) -> Charge/Taille totale a repartir au sein de la partition
- Mtot (Mt) -> Nombre de total d'element (fragment, cluster) de la partition (Zi) i=1,Mtot
- Zmax (Zm) -> Charge/Taille du plus gros element de la partition
- Zinf (Zi) -> Charge minimale des elements de la partition (Zi) Zi>=Zinf quelque soit i (tous les calculs necessitent cet argument)

Plusieurs methodes correspondant conditions initiales possibles :
- BreakUsing_Ztot_Zinf_Criterion (la methode la plus generale), donnant toutes les partitions pour une taille initiale donnee
- BreakUsing_Ztot_Zinf_Criterion, donnant toutes les partitions pour une taille initiale donnee
- BreakUsing_Ztot_Zmax_Zinf_Criterion, donnant toutes les partitions pour une taille initiale donnee, avec une contrainte sur Zmax
- BreakUsing_Mtot_Zmax_Zinf_Criterion, donnant toutes les partitions remplissant les contraintes sur le Zmax et sur le Mtot

Toutes ces methodes utilisent la protected methode MakePartitions() qui calcule les partitions
pour un couple (Ztot,Mtot) donné qui sauvegarde ces partitions dans un fichier ROOT via un arbre, contenant
l'information minimum :
- branche zt
- branche mt
- branche tabz [mt], tableau d'entier comprenant les differentes charges (Zi), trie par ordre decroissant
Le chemin ou les arbres sont ecrits peut etre definis via la methode SetPathForFile, par defaut les arbres seront
ecrits dans le repertoire courant, attention car pour des nombres importants (Ztot>100) la place necessaire devient
non negligeable (ex : toutes les partition de Ztot=200 et Zinf=5 -> 20GB environ)

A chaque fois que la methode MakePartitions() est appelle par les methodes BreakUsing...(), on garde
en memoire l'ensemble des fichiers contenant chacun un arbre, pour a la fin du processus
ecrire dans un fichier ROOT, une TChain permettant de travailler rapidement et facilement sur les partitions generees
----

Un petit exemple de routine simple :
Cette routine produit en sortie un fichier root Partitions_Zf60_Zi1.root
contenant la TChain de tous les arbres ...
~~~~{.cpp}
void test{

   KVPartitionGenerator* pg = new KVPartitionGenerator();
   pg->SetPathForFile("/space/bonnet/");
   pg->BreakUsing_Ztot_Zinf_Criterion(60,1);

   Info in <BreakUsing_Ztot_Zinf_Criterion>: 966467.000000 partitions crees en 5 seconds
   delete pg;

}
~~~~
~~~~
root Partitions_Zf60_Zi1.root
Attaching file Partitions_Zf60_Zi1.root as _file0...
root [2] .ls
TFile**     Partitions_Zf60_Zi1.root
 TFile*     Partitions_Zf60_Zi1.root
  KEY: TChain  PartitionTree;1
root [3] PartitionTree->Draw("tabz")
~~~~
*/

class KVPartitionGenerator {
protected:

   //Compteurs de nombre de partitions crees
   Double_t npar, npar_zt_mt;
   //Chemin ou les arbres seront ecrits
   KVString kwriting_path, tname, cname;

   Bool_t to_be_checked;

   //Variables additionnelles
   //utilisees par les methodes BreakUsing_Zf_Zmax_Zinf_Criterion
   //et BreakUsing_Mtot_Zmax_Zinf_Criterion, pour l enregistrement des
   //partitions dans tabz
   Int_t mshift;
   Int_t zshift;

   //Variables pemettant le remplissage des partitions
   //dans un arbre
   Int_t mtot, ztot;
   Int_t* tabz;   //[ndim]

   Int_t kzt, kmt, kzinf;
   Int_t* kcurrent;

   TTree* tree;
   KVUniqueNameList* flist;

   //To give an estimation of time consuming
   Int_t tstart, tstop, tellapsed;
   void Start();
   void Stop();
   Int_t GetDeltaTime();

   void MakePartitions(Int_t Ztot, Int_t Mtot, Int_t Zinf);
   void SetConditions(Int_t Ztot, Int_t Mtot, Int_t Zinf);
   void PreparTree();
   void Process();

   void BeforeBreak();
   void AfterBreak();

   void init();

public:
   KVPartitionGenerator();
   virtual ~KVPartitionGenerator();

   void SetPathForFile(KVString path);

   void BreakUsing_Ztot_Zinf_Criterion(Int_t Ztot, Int_t Zinf, KVString chain_name = "", Int_t min = -1, Int_t max = -1);
   void BreakUsing_Ztot_Zmax_Zinf_Criterion(Int_t Ztot, Int_t Zmax, Int_t Zinf, KVString chain_name = "");
   void BreakUsing_Mtot_Zmax_Zinf_Criterion(Int_t Mtot, Int_t Zmax, Int_t Zinf, KVString chain_name = "");
   void BreakUsing_Ztot_Mtot_Zinf_Criterion(Int_t Ztot, Int_t Mtot, Int_t Zinf, KVString chain_name = "");

   virtual void TreatePartition();
   virtual void WriteTreeAndCloseFile();

   ClassDef(KVPartitionGenerator, 1) //Calcul numeriquement toutes les partitions d'un couple Ztot/Mtot donne
};

#endif
