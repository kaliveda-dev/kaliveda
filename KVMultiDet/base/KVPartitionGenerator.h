//Created by KVClassFactory on Thu Jun 17 14:20:16 2010
//Author: bonnet

#ifndef __KVPartitionGenerator_H
#define __KVPartitionGenerator_H
#include "KVString.h"
#include "KVUniqueNameList.h"

class TTree;

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
