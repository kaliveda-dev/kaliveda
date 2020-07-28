//Created by KVClassFactory on Mon Mar 29 14:58:00 2010
//Author: bonnet

#ifndef __KVINTEGERLIST_H
#define __KVINTEGERLIST_H
#include "KVString.h"
#include "TArrayI.h"
#include "TNamed.h"

/**

\class KVIntegerList
\brief Handle a list of positive integers (partition)
\ingroup Analysis

Values are sorted into decreasing order.

Two examples of filling a list:

#### Example 1
Fill() can be used to fill the list from an array of values and update relevant quantities:
~~~~{.cpp}
KVIntegerList ilist;
int tab[] = {1,3,6,4,9,7,3,59,8,160};
ilist->Fill(tab,10);
ilist->Print();

Info in <KVIntegerList::Print>: 160 59 9 8 7 6 4 3(2) 1 : population 1
~~~~{.cpp}

#### Example 2
Les methodes de type Add(), remplit successivement la liste, avant d'utiliser les grandeurs reliées<br>
l'utilisateur doit appeler la méthode CheckForUpdate(), pour que la mise a jour du nom soit faite<br>
par contre le nombre d'elements est mis a jour automatiquement<br>
<br>
<code>
KVIntegerList* ilist = new KVIntegerList();<br>
ilist->Add(23,6);<br>
ilist->Add(12);<br>
Int_t tab[3]={1,3,6};<br>
ilist->Add(tab,3);<br>
ilist->CheckForUpdate();<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 23(6) 12 6 3 1 : population 1<br>
ilist->Remove(23,3);<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 23(6) 12 6 3 1 : population 1       &nbsp  &nbsp    !!! pour le nom  RIEN A CHANGER<br>
<code>ilist->GetNbre();<br>
(const Int_t)7                &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp le nombre, lui est mis à jour<br>
ilist->CheckForUpdate();<br>
ilist->Print();<br>
Info in <KVIntegerList::Print>: 23(3) 12 6 3 1 : population 1       &nbsp  &nbsp    OK : les modifs ont été prises en compte<br>
ilist->GetNbre()<br>
(const Int_t)7
</code>
END_HTML

Les methodes de type Remove(), retirent des valeurs de la liste, là encore l'utilisateur doit appeler
la méthode CheckForUpdate(), pour que la mise a jour soit faite
Les grandeurs disponibles sont:
- le nombre d'entiers, ie la multiplicité GetNbre()
- le nom de la partition definie comme suit : ent1(occ) ent2 ent3(occ3), où entre parenthèses
sont notées les occurences aupérieures à 1 d'une valeur
- la population GetPopulation(), permet de gérer un ensemble de partitions (KVPartitionManager)

*/

class KVIntegerList : public TNamed {

protected:

   TArrayI* fRegle;  //!      tableau d'entiers où sont stockée l'occurence des valeurs
   Int_t fLimiteRegle;  //!-> taille max de fRegle
   Int_t fPop; //!            population de la liste/partition consideree, permet le comptage de partitions identiques dansun lot de donée
   Int_t fMult;//!            Nombre d'éléments dans la liste
   Ssiz_t fLength; //!        Longueur du nom de la liste/partition usilisée dans Compare

   void init();
   Bool_t ToBeUpdated();
   virtual void Update();
   virtual void ResetRegle();

   virtual void SetPartition(const Char_t* par);
   virtual void DeducePartitionFromTNamed();
   virtual void DeducePopulationFromTitle();
   virtual void DeducePartitionFromName();

   virtual void add_values(Int_t val, Int_t freq);
   virtual Bool_t remove_values(Int_t val, Int_t freq);


public:

   enum {
      kHastobeComputed = BIT(14) //Variables has to be recalculated or not
   };

   KVIntegerList();
   virtual ~KVIntegerList();

   Int_t Compare(const TObject* obj) const;
   void Clear(Option_t* option = "");
   void Copy(TObject& named) const;
   void Print(Option_t* option = "") const;

   void CheckForUpdate();

   //void Fill(TArrayI* tab);
   void Fill(Int_t* tab, Int_t mult);
   //void Fill(Double_t* tab,Int_t mult);

   void Add(TArrayI* tab);
   void Add(Int_t* tab, Int_t mult);
   void Add(Double_t* tab, Int_t mult);

   void Add(Int_t val);
   void Add(Int_t val, Int_t freq);

   void Add(Double_t val);
   void Add(Double_t val, Int_t freq);

   Bool_t Remove(Int_t val, Int_t freq);
   Bool_t Remove(Int_t val);
   Bool_t RemoveAll(Int_t val);

   void ResetPopulation();
   Int_t GetPopulation() const;
   void AddPopulation(Int_t pop);
   void SetPopulation(Int_t pop);

   Int_t GetNbre() const;

   Int_t GetFrequency(Int_t val) const;
   Bool_t Contains(Int_t val) const;
   Ssiz_t GetLengthName() const;

// KVPartition* CreateKVPartition(Int_t mom_max=5);
   TNamed* CreateTNamed();
   TArrayI* CreateTArrayI();
   Int_t* CreateTableOfValues();

   ClassDef(KVIntegerList, 1) //Permet de gerer une liste de nombres entiers positifs
};

#endif
