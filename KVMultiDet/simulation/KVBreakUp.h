//Created by KVClassFactory on Thu Mar 25 11:25:27 2010
//Author: bonnet

#ifndef __KVBREAKUP_H
#define __KVBREAKUP_H

#include "TRandom.h"
#include "KVNumberList.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "KVHashList.h"
#include "KVString.h"
#include "TTree.h"
#include "TH2F.h"
#include "TProfile.h"
#include "KVIntegerList.h"
#include "KVPartitionList.h"
#include "KVTemplateEvent.h"
#include "TDatime.h"
#include <cstdlib>
#include "Riostream.h"
#include "KVEvent.h"

/**
\class KVBreakUp
\brief Permet de casser aleatoirement un nombre entier (ztot) en un nombre (mtot) d'entiers plus petits d'une valeur minimale (zmin)
donnée. Plusieurs methodes de cassures sont proposees
\ingroup Simulation

Initialisation :
 - Méthode SetConditions(Int_t zt,Int_t mt,Int_t zmin=1), définie la taille totale de la partition, le nombre de fragments et la taille minimale de la partition
 - Méthode DefineBreakUpMethod(KVString bup_method=""), permet de definir la facon de casser la taille initiale
 Sont implémentées :
 ~~~~{.cpp}
      BreakUsingChain (defaut) -> On casse aléatoirement les liens entre charge
      BreakUsingLine          -> On casse aléatoirement les liens entre charge + "effets de bords"
      BreakUsingIndividual    -> On tire aleatoirement une taille puis les autres tailles seront tirées a partir de la charge restante
      BreakUsingPile          -> On distribue 1 par un 1 les charges entre les fragments
      etc...
 ~~~~
  Normalement toutes ces methodes garantissent à chaque tirage, les conditions imposées par SetConditions

 - Méthode RedefineTRandom(KVString TRandom_Method), permet de redefinir la classe de tirage aleatoire voir TRandom et classes filles, la classe par default est TRandom3

Exemple d'utilisation :
~~~~{.cpp}
void test{

KVBreakUp* bu = new KVBreakUp();
bu->SetConditions(80,6,5);
bu->StorePartitions(kFALSE);
bu->BreakNtimes(10000);

bu->DrawPanel();

bu->SaveAsTree("essai.root","tree")
bu->SaveHistos("essai.root","","update")

}
~~~~
*/

class KVBreakUp: public KVPartitionList {

protected:

   Int_t Ztotal;
   Int_t Mtotal;
   Int_t Zmin;
   Int_t* bound;  //[Ztotal] tableau permettant de gérer les cassures de liens
   KVString BreakUpMethod, TRandom_Method;

   TRandom* alea;

   KVNumberList nl;
   Int_t nbre_nuc;
   Int_t size_max;
   Int_t* size;   //[size_max]->
   Int_t niter_tot;
   Int_t tstart, tstop, tellapsed;

   TH1F* hzz; //->
   TH1F* hzt;
   TH1F* hmt;

   KVHashList* lobjects;   //->
   KVHashList* lhisto;  //->
   KVIntegerList* partition;
   KVEvent* current_event;

   enum {
      kStorePartitions = BIT(14),   //Variables has to be recalculated
   };

   void init();
   void SetZtot(Int_t zt);
   void SetMtot(Int_t mt);
   void SetZmin(Int_t zlim);
   void SetBreakUpMethod(KVString bup_method);

   void Start();
   void Stop();

public:

   Int_t nraffine;

   //Methodes qui devraient etre declaree
   //en privee
   //mais celà rend impossible leur utilisation
   //via TMethodCall
   Int_t BreakUsingChain();
   Int_t BreakUsingPile();
   Int_t BreakUsingIndividual();
   Int_t BreakUsingLine();

   KVBreakUp(Int_t taille_max = 1000);
   virtual ~KVBreakUp();
   void Clear(Option_t* = "");
   void LinkEvent(KVEvent*);
   void RedefineTRandom(KVString TRandom_Method);
   void SetConditions(Int_t zt, Int_t mt, Int_t zmin = 1);
   void DefineBreakUpMethod(KVString bup_method = "");
   void StorePartitions(Bool_t choix = kTRUE);
   virtual void DefineHistos();

   Int_t GetZtot(void) const;
   Int_t GetMtot(void) const;
   Int_t GetZmin(void) const;
   KVString GetBreakUpMethod(void) const;

   virtual void TreatePartition();

   void BreakNtimes(Int_t times = 1000);
   void BreakNtimesOnGaussian(Int_t times, Double_t Ztot_moy, Double_t Ztot_rms, Double_t Mtot_moy, Double_t Mtot_rms, Int_t zmin = 1);
   void BreakFromHisto(TH2F* hh_zt_VS_mt, Int_t zmin = 1);

   KVEvent* BreakOnGaussian(Double_t Ztot_moy, Double_t Ztot_rms, Double_t Mtot_moy, Double_t Mtot_rms, Int_t zmin);
   KVEvent* GetCurrentEvent();

   virtual void DrawPanel();

   Int_t GetTotalIterations(void);
   KVHashList* GetHistos();
   KVHashList* GetObjects();

   virtual void ResetTotalIterations();
   virtual void ResetHistos();

   virtual void SaveHistos(KVString filename = "", KVString suff = "", Option_t* option = "recreate");

   void PrintConfig() const;
   Int_t GetDeltaTime();


   ClassDef(KVBreakUp, 1) //Exemple de differentes facons de casser un entier en plusieurs autres entiers, avec comme principales contraintes la charge totale et le nombre de clusters
};

#endif
