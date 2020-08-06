/*
$Id: KVCaloBase.h,v 1.3 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.3 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Mon Apr 14 15:01:51 2008
//Author: eric bonnet,,,

#ifndef __KVCaloBase_H
#define __KVCaloBase_H

#include "KVVarGlob.h"
#include "KVNameValueList.h"
#include "KVNucleus.h"

class KVCaloBase: public KVVarGlob {

protected:

   KVNucleus nn;  //! permet d utiliser des methodes de KVNucleus
   KVNameValueList* nvl_ing;//!-> //Contains all ingredients computed
   KVNameValueList* nvl_par;//!-> //Contains all parameters needed for the computation

   Bool_t   kIsModified;   //indique les ingredients ont ete modifies
   virtual void SumUp();

   Bool_t   RootSquare(Double_t aaa, Double_t bbb, Double_t ccc);
   Double_t kracine_max, kracine_min; //deux racines issues de la resolution de RootSquare
   Int_t    kroot_status;  //statut pour la methode de RootSquare

   virtual Double_t getvalue_int(Int_t) const;
   Double_t GetIngValue(Int_t idx) const;
   void SetIngValue(KVString name, Double_t value) const;
   void AddIngValue(KVString name, Double_t value) const;
   void SetParameter(const Char_t* par, Double_t value);

   void init_KVCaloBase();
   void ComputeExcitationEnergy();

public:

   KVCaloBase(void);
   KVCaloBase(const Char_t* nom);
   KVCaloBase(const KVCaloBase& a);

   virtual ~KVCaloBase(void);
   virtual void Copy(TObject& obj) const;

   KVCaloBase& operator = (const KVCaloBase& a);

   void Reset(void);
   void Print(Option_t* opt = "") const;
   KVNameValueList* GetList(Option_t* opt = "ing") const;

   Int_t GetNumberOfValues() const;

   Double_t GetIngValue(KVString name) const;
   Double_t GetParValue(KVString name) const;
   Bool_t HasParameter(KVString name) const;
   Int_t GetNameIndex(const Char_t* name) const;
   const Char_t* GetValueName(Int_t i) const;
   virtual Char_t GetValueType(Int_t) const;
   std::vector<Double_t> GetValueVector(void) const;

   virtual void   Fill(KVNucleus*);
   void AddNeutrons(Int_t mult, Double_t mke);
   virtual void Calculate();

   ClassDef(KVCaloBase, 1) //compute CaloBase

};


#endif
