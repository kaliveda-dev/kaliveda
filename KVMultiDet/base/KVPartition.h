//Created by KVClassFactory on Mon Mar 29 14:58:00 2010
//Author: bonnet

#ifndef __KVPARTITION_H
#define __KVPARTITION_H
#include "KVString.h"
#include "KVIntegerList.h"
#include "KVValues.h"

class KVPartition : public KVIntegerList {

protected:

   TArrayI* ftab; //!-> tableau regroupant tous les entiers de la partition (ordre decroissant)
   TArrayI* ftab_diff; //!-> tableau regroupant tous les entiers differents de la partition (ordre decroissant)
   Int_t fMult_diff; //!   Nbre de valeurs differentes
   KVValues* fValues;   //!-> Object permettant de gerer les moments ou autre

   void Update();
   void init(Int_t mommax = 5);

   void add_values(Int_t val, Int_t freq);
   Bool_t remove_values(Int_t val, Int_t freq);

public:

   KVPartition();
   KVPartition(Int_t mommax);
   virtual ~KVPartition();

   void Clear(Option_t* option = "");
   void Copy(TObject& obj) const;
   void Print(Option_t* option = "") const;

   Int_t GetOrdreMax() const
   {
      return fValues->GetOrdreMax();
   }

   //Methodes donnant aux valeurs uniques de la partition
   Int_t GetMultDiff(void) const
   {
      return fMult_diff;
   }
   Int_t* GetValeursDiff() const
   {
      return ftab_diff->fArray;
   }
   Int_t GetValeurDiff(Int_t rang) const
   {
      return ftab_diff->At(rang);
   }

   //Methodes donnant accès à toutes les valeus de la partition avec la notion d'occurence/frequence
   //en utilisant le rang ou la valeur
   Int_t GetMult(void) const
   {
      return GetNbre();
   }
   Int_t* GetValeurs() const
   {
      return ftab->fArray;
   }
   Int_t GetValeur(Int_t rang) const
   {
      return ftab->At(rang);
   }
   Int_t GetFrequenceAt(Int_t rang) const
   {
      return fRegle->At(GetValeur(rang));
   }

   //Methodes donnant acces aux variables calculees de la partition
   Double_t GetMoment(Int_t ordre) const
   {
      return ((ordre <= fValues->GetOrdreMax()) ? fValues->GetValue(ordre + fValues->GetShift()) : -1);
   }
   Double_t GetMomentNormalise(Int_t ordre) const
   {
      return fValues->GetValue(ordre + fValues->GetShift()) / GetMoment(0);
   }

   Double_t GetZtot() const
   {
      return GetMoment(1);
   }
   Double_t GetMtot() const
   {
      return GetMoment(0);
   }
   Double_t GetZmean() const
   {
      return GetMomentNormalise(1);
   }

   Double_t GetZmax(Int_t rang = 0) const;
   Double_t GetZmin(Int_t rang = 0) const;
   Double_t GetZ1() const
   {
      return GetZmax(0);
   }
   Double_t GetZ2() const
   {
      return GetZmax(1);
   }

   Bool_t RemoveAllValuesAt(Int_t rang);
   Bool_t RemoveAt(Int_t rang);

   KVValues* GetAddValues()
   {
      return fValues;
   }

   ClassDef(KVPartition, 1) //Permet de gerer des partitions de nombres entiers et le calcul de grandeurs associees
};

#endif
