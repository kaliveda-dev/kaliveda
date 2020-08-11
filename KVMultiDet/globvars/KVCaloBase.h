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

/**
\class KVCaloBase
\brief Calorimetry of hot nuclei
\ingroup GlobalVariables

 ## Principle
 KVCaloBase sums the Z (Zsum), A (Asum), Ek (Eksum) and Q (Qsum) of the considered nuclei (method Fill(KVNucleus* ))
 These ingredients allow to calculate the excitation energy using the following relation:

~~~~~~~~~~~~~~~~~~~~~~~
 Exci + Qini  = Eksum + Qsum -> Exci = Eksum + Qsum - Qini
~~~~~~~~~~~~~~~~~~~~~~~

 ### Example of use

~~~~~~~~~~~~~~~~~~~~~
KVNucleus alpha(2,4,10); //definition of nuclei
KVNucleus triton(1,3);
KVNucleus azote(7,16,40);

KVCaloBase ca;

ca.Fill(&alpha);   //filling the variable
ca.Fill(&triton);
ca.Fill(&azote);

ca.Calculate();
ca.Print("ing");   // print ingredients

Ingredients, 7 stored:
0 | Zsum | 10.00000   Sum of charges
1 | Asum | 23.00000   Sum of masses
2 | Eksum | 50.0000   Sum of kinetic energies (MeV)
3 | Qsum | 23.05840   Sum of mass excess (MeV)
4 | Msum | 3.000000   Multiplicity
5 | Qini | -5.15400   Mass Excess of the initial state (reconstructed source)
6 | Exci | 78.21240   Excitation energy (MeV)

ca.GetValue(0);  // return value "Zsum"
10.0000
ca.GetValue("Exci")
78.21240
~~~~~~~~~~~~~~~~~~~~~

*/
class KVCaloBase: public KVVarGlob {

   void init_KVCaloBase();

protected:

   KVNucleus nn;  //! permet d utiliser des methodes de KVNucleus
   KVNameValueList nvl_ing;//!-> //Contains all ingredients computed

   Bool_t   kIsModified;   //indique les ingredients ont ete modifies
   virtual void SumUp();

   Bool_t   RootSquare(Double_t aaa, Double_t bbb, Double_t ccc);
   Double_t kracine_max, kracine_min; //deux racines issues de la resolution de RootSquare
   Int_t    kroot_status;  //statut pour la methode de RootSquare

   virtual void fill(const KVNucleus*);
   virtual Double_t getvalue_int(Int_t) const;
   Double_t GetIngValue(Int_t idx) const;
   void SetIngValue(KVString name, Double_t value);
   void AddIngValue(KVString name, Double_t value);

   void ComputeExcitationEnergy();

public:

   KVCaloBase() : KVVarGlob("KVCaloBase")
   {
      init_KVCaloBase();
   }
   KVCaloBase(const Char_t* nom) : KVVarGlob(nom)
   {
      init_KVCaloBase();
   }
   ROOT_COPY_CTOR(KVCaloBase, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVCaloBase)
   virtual ~KVCaloBase(void) {}
   virtual void Copy(TObject& obj) const;

   void Init();
   void Reset();
   void Print(Option_t* opt = "") const;
   const KVNameValueList& GetList(Option_t* opt = "ing") const;

   Double_t GetIngValue(const KVString& name) const;
   virtual Char_t GetValueType(Int_t) const;
   std::vector<Double_t> GetValueVector(void) const;

   void AddNeutrons(Int_t mult, Double_t mke);
   virtual void Calculate();

   ClassDef(KVCaloBase, 1) //compute CaloBase

};


#endif
