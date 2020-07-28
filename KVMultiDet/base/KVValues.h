//Created by KVClassFactory on Wed May 19 17:23:29 2010
//Author: bonnet

#ifndef __KVVALUES_H
#define __KVVALUES_H

#include "TNamed.h"
#include "TMath.h"
#include "TFormula.h"

#include "KVString.h"
#include "KVNameValueList.h"


class KVNumberList;
class KVList;

/**
\class KVValues
\brief Handle Operation on variable
\ingroup Analysis
*/

class KVValues : public KVNameValueList {

protected:

   Int_t kdeb; //separation entre les valeurs MIN et MAX et les valeurs moments SUM%d, par defaut kdeb=2

   Int_t kval_base;//nbre de valeurs de base definies par le constructeur, kval_base = kdeb+kordre_mom_max+1
   Int_t kval_add; //nbre de valeurs additionelles definies via la methode DefineAdditionalValue
   Int_t kval_tot; //nbre total de valeurs definies kval_tot = kval_base+kval_add (=KVNameValueList::GetNpar())

   Int_t knbre_val_max;//nombre maximum de valeurs pouvant etre calculees (argument du constructeur)
   Int_t kordre_mom_max;//ordre maximum pour le calcul des moments (argument du constructeur)
   Int_t kTimesFillVarIsCalled; //nombre de fois ou la methode FillVar est appelee
   Double_t* values; //[knbre_val_max]  tableau ou sont stockees les valeurs

   Bool_t kToBeRecalculated;

   /*
   kform_add et kpar_add sont remplies en parallele et permettent le calcul des variables
   additionnelles
   */
   KVList* kform_add;   //->liste contenant les formules des variables additionnelle (stockees en TFormula)
   KVList* kpar_add;    //->liste contenant les liste de parametres (stockees en KVNumberList)


   void init_val_base();
   void init_val_add();
   void init_val();

   void ComputeAdditionalValues(Int_t min = -1, Int_t max = -1);
   void LinkParameters(KVNumberList* nl);
   Bool_t AddFormula(KVString name, KVString expr);
   KVNumberList*  TransformExpression(KVString& expr);

public:

   KVValues();
   KVValues(const Char_t* name, Int_t ordre_max = 3, Int_t nbre_max = 20);
   virtual ~KVValues();

   void Reset();
   void Clear(Option_t* option = "");
   void Print(Option_t* option = "") const;

   void FillVar(Double_t val);
   void FillVar(Double_t val, Double_t weight);

   Int_t GetOrdreMax() const;
   Int_t GetNumberOfFilling() const;
   Int_t GetShift() const;

   Double_t GetValue(Int_t idx) const;

   Double_t GetValue(const Char_t* name) const;
   Int_t GetValuePosition(const Char_t* name) const;

   TString GetValueExpression(const Char_t* name) const;
   TString GetValueExpression(Int_t idx) const;

   Int_t GetNValues(KVString opt = "base") const;

   void DefineAdditionalValue(KVString name, KVString expr);

   Bool_t Add(KVValues* val);

   ClassDef(KVValues, 1) //Handle Operation on variable

};

#endif
