//Created by KVClassFactory on Fri Oct  8 14:27:07 2010
//Author: bonnet

#include "KVPartitionList.h"
#include "KVPartition.h"
#include "KVIntegerList.h"
#include "TTree.h"

ClassImp(KVPartitionList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVPartitionList</h2>
<h3>Classe dérivant de KVUniqueNameList</h3>
<h4>Gestion d'une liste de partition (objet de la classe KVIntegerList ou derivee)</h4>
<!-- */
// --> END_HTML
/*
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
////////////////////////////////////////////////////////////////////////////////

//_______________________________________________________
void KVPartitionList::init()
{
   //Initialisation
   atrouve = kFALSE;
   knbre_diff = 0;
   knbre_tot = 0;
   SetOwner(kTRUE);
   mult_range = new KVPartition();

}

//_______________________________________________________
KVPartitionList::KVPartitionList()
{
   // Default constructor
   init();
}

//_______________________________________________________
KVPartitionList::KVPartitionList(const Char_t* name)
{
   // constructor with name
   init();
   SetName(name);
}

//_______________________________________________________
KVPartitionList::~KVPartitionList()
{
   // Destructor
   delete mult_range;
   mult_range = 0;
}

//_______________________________________________________
void KVPartitionList::Clear(Option_t* option)
{
   //Mise a zero de la liste
   KVSeqCollection::Clear(option);
   mult_range->Clear();
   knbre_diff = 0;
   knbre_tot = 0;

}
//_______________________________________________________
Bool_t KVPartitionList::IsInTheList()
{
   // retourne un booléen indiquant si la dernière partition
   // utilisée dans les méthodes Add...()
   // était déjà dans la liste (kTRUE) ou non (kFALSE)
   return atrouve;

}

//_______________________________________________________
Double_t KVPartitionList::GetNbreTot()
{
   //Retourne le nombre de partitions totales ie le nombre de fois ou la methode
   //Fill a été appelée
   //Exemple: si 3 partitions differentes et pop(i) leur population associée
   //knbre_tot = pop(1) + pop(2) + pop(3)
   return knbre_tot;

}

//_______________________________________________________
Double_t KVPartitionList::GetNbreDiff()
{
   //Retourne le nombre de partitions différentes
   //
   //Exemple: si 3 partitions differentes et pop(i) leur population associée
   //knbre_diff = 3
   return knbre_diff;

}

//_______________________________________________________
Bool_t KVPartitionList::Fill(KVIntegerList* par)
{
   //Incrémente le nombre totale de partitions
   //retourne kTRUE si une partition identique est déjà dans la liste
   //
   knbre_tot += 1.;

   Add(par);

   return IsInTheList();

}

//_______________________________________________________
void KVPartitionList::Add(TObject* obj)
{
   // Add an object to the list if it is not already in it
   // (no object with same name in list)
   //
   // if it is in, the population of it is incremented

   TObject* find = 0;
   //Test la présence d'une partition identique
   if (!(find = FindObject(obj->GetName()))) {
      //Ajout de la partition
      KVHashList::Add(obj);
      ValidateEntrance((KVIntegerList*)obj);
   } else {
      //une partition identique existe deja
      atrouve = kTRUE;
      //on incremente la population de celle ci
      ((KVIntegerList*)find)->AddPopulation(1);
   }
}

//_______________________________________________________
void KVPartitionList::ValidateEntrance(KVIntegerList* il)
{
   //Protected methode
   //appelée dans le cas ou il y a une nouvelle partition
   atrouve = kFALSE;
   knbre_diff += 1; //on incremente le nombre de partitions differentes
   //on incremente egalement la liste contenant les multiplicités
   mult_range->Add(il->GetNbre());
}

//_______________________________________________________
void KVPartitionList::AddFirst(TObject* obj)
{
   // Add an object to the list if it is not already in it
   // (no object with same name in list)
   // if it is in, the population of it is incremented

   TObject* find = 0;
   if (!(find = FindObject(obj->GetName()))) {
      KVHashList::AddFirst(obj);
      ValidateEntrance((KVIntegerList*)obj);
   } else {
      atrouve = kTRUE;
      ((KVIntegerList*)find)->AddPopulation(1);
   }
}

//_______________________________________________________
void KVPartitionList::AddLast(TObject* obj)
{
   // Add an object to the list if it is not already in it
   // (no object with same name in list)
   // if it is in, the population of it is incremented

   TObject* find = 0;
   if (!(find = FindObject(obj->GetName()))) {
      KVHashList::AddLast(obj);
      ValidateEntrance((KVIntegerList*)obj);
   } else {
      atrouve = kTRUE;
      ((KVIntegerList*)find)->AddPopulation(1);
   }
}

//_______________________________________________________
void KVPartitionList::AddAt(TObject* obj, Int_t idx)
{
   // Add an object to the list if it is not already in it
   // (no object with same name in list)
   // if it is in, the population of it is incremented

   TObject* find = 0;
   if (!(find = FindObject(obj->GetName()))) {
      KVHashList::AddAt(obj, idx);
      ValidateEntrance((KVIntegerList*)obj);
   } else {
      atrouve = kTRUE;
      ((KVIntegerList*)find)->AddPopulation(1);
   }
}

//_______________________________________________________
void KVPartitionList::AddAfter(const TObject* after, TObject* obj)
{
   // Add an object to the list if it is not already in it
   // (no object with same name in list)
   // if it is in, the population of it is incremented

   TObject* find = 0;
   if (!(find = FindObject(obj->GetName()))) {
      KVHashList::AddAfter(after, obj);
      ValidateEntrance((KVIntegerList*)obj);
   } else {
      atrouve = kTRUE;
      ((KVIntegerList*)find)->AddPopulation(1);
   }
}

//_______________________________________________________
void KVPartitionList::AddBefore(const TObject* before, TObject* obj)
{
   // Add an object to the list if it is not already in it
   // (no object with same name in list)
   // if it is in, the population of it is incremented

   TObject* find = 0;
   if (!(find = FindObject(obj->GetName()))) {
      KVHashList::AddBefore(before, obj);
      ValidateEntrance((KVIntegerList*)obj);
   } else {
      atrouve = kTRUE;
      ((KVIntegerList*)find)->AddPopulation(1);
   }
}

//_______________________________________________________
void KVPartitionList::Update()
{
   //Met a jour l'intervalle de multiplicité associées aux partitions dans la liste
   mult_range->CheckForUpdate();
   mult_range->Print("Partitions");

}

//_______________________________________________________
TTree* KVPartitionList::GenereTree(const Char_t* treename, Bool_t Compress)
{
   //Protected method
   //Creation de l'arbre et remplissage de toutes les partitions
   if (GetEntries() == 0) return 0;

   Update();

   Int_t mmax = Int_t(mult_range->GetZmax(0));  //Multiplicité max enregistrée
   Info("GenereTree", "Multiplicite max entregistree %d", mmax);
   Int_t* tabz = new Int_t[mmax];
   Int_t mtot;
   Int_t pop;
   Info("GenereTree", "Nbre de partitions entregistrees %d", GetEntries());

   TTree* tree = new TTree(treename, Class_Name());
   //Declaration des branches
   tree->Branch("mtot",       &mtot,   "mtot/I");  //multiplicité
   tree->Branch("tabz",       tabz,    "tabz[mtot]/I");  //partition sous forme de tableau d'entiers
   if (Compress)
      tree->Branch("pop",  &pop,    "pop/I");   //Si Compress==kTRUE, la branche population
   //associée à chaue partition
   KVIntegerList* par;
   TArrayI* table = 0;
   for (Int_t kk = 0; kk < GetEntries(); kk += 1) {
      par = (KVIntegerList*)At(kk);
      table = par->CreateTArrayI();

      mtot = par->GetNbre();
      if (kk % 5000 == 0)
         Info("GenereTree", "%d partitions traitees", kk);
      pop = par->GetPopulation();

      for (Int_t mm = 0; mm < mtot; mm += 1)
         tabz[mm] = table->At(mm);

      //Si Compress==kTRUE, on remplie une fois avec la population (pop) associée
      //sinon on rempli pop fois cette partition
      if (Compress) {
         tree->Fill();
      } else {
         for (Int_t pp = 0; pp < pop; pp += 1)  tree->Fill();
      }
      delete table;
   }
   Info("GenereTree", "Fin du remplissage");

   tree->ResetBranchAddresses();
   delete [] tabz;

   return tree;

}

//_______________________________________________________
void KVPartitionList::SaveAsTree(const Char_t* filename, const Char_t* treename, Bool_t, Option_t* option)
{
   //Open a file, save all the registered partitions in a tree format, and close the file
   //filename -> name of the root file where the tree is stored
   //treename -> name of the tree
   //Compress -> if kTRUE : a branch associated to the population of the partition is created
   //         -> if kFALSE : all the partitions are written one by one
   //option   -> option for the file

   TFile* file = new TFile(filename, option);
   TTree* tt = 0;
   if ((tt = GenereTree(treename))) tt->Write();
   file->Close();
}

