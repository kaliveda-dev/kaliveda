//Created by KVClassFactory on Mon Mar 29 14:58:00 2010
//Author: bonnet

#ifndef __KVINTEGERLIST_H
#define __KVINTEGERLIST_H
#include "KVString.h"
#include "TArrayI.h"
#include "TNamed.h"

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

   // ATTENTION aux methodes 'virtual' avec surcharge!!!
// virtual void Fill(TArrayI* tab);
   virtual void Fill(Int_t* tab, Int_t mult);
// virtual void Fill(Double_t* tab,Int_t mult);
// virtual void Fill(KVEvent* evt,Option_t* opt);

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
