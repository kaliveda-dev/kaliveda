/*
$Id: KVElasticScatterEvent.h,v 1.3 2009/01/14 15:35:50 ebonnet Exp $
$Revision: 1.3 $
$Date: 2009/01/14 15:35:50 $
*/

//Created by KVClassFactory on Thu Dec 11 14:45:29 2008
//Author: eric bonnet,,,

#ifndef __KVELASTICSCATTEREVENT_H
#define __KVELASTICSCATTEREVENT_H

#include "KVString.h"
#include "TVector3.h"
#include "KVTarget.h"
#include "KVLayer.h"
#include "KVMaterial.h"
#include "KVNucleus.h"
#include "KVSimEvent.h"
#include "KV2Body.h"
#include "KVMultiDetArray.h"
#include "KVReconstructedNucleus.h"
#include "KVReconstructedEvent.h"
#include "TObject.h"
#include "TTree.h"
#include "KVHashList.h"
#include "KVBase.h"
#include "KVDBSystem.h"
#include "KVPosition.h"
#include "KVParticle.h"

/**
\class KVElasticScatterEvent
\brief simulate ElasticScatterEvent and answer of a given (multi-)detector : A + B -> A + B
\ingroup Simulation

Definition de la voie d'entrée avec les methodes suivantes :
   - SetSystem(KVDBSystem* sys);
   - SetSystem(Int_t zp,Int_t ap,Double_t ekin,Int_t zt,Int_t at);
   - SetTargNucleus(KVNucleus *nuc);
   - SetProjNucleus(KVNucleus *nuc);
   - SetTargetMaterial(KVTarget *targ,Bool_t IsRandomized=kTRUE);

La possibilité est donnée d'effectuer des diffusions sur un noyau différent des noyaux de la cible

Ex :  SetTargetNucleus(new KVNucleus("181Ta"));
      SetTargetMaterial(new KVTarget("40Ca",1.0)) //cible de Ca de 1 mg/cm2
         Diffusion sur un noyau de Ta et propagation dans une cible de Ca
Si SetTargetMaterial est appelé et pas SetTargNucleus, le noyau cible est débuit du materiel choisi pour la cible

Definition du domaine angulaire de diffusion et mode de tirage
   - DefineAngularRange(TObject*) domaine angulaire deilimité par une structure geometrique (KVPosition etc ...)
   - DefineAngularRange(Double_t tmin, Double_t tmax, Double_t pmin, Double_t pmax) intervalle en theta et phi (degree)
   - SetDiffNucleus(KVString name) name="PROJ" ou "TARG" determine a quel noyau projectile ou cible
   est associe le domaine angulaire choisi
   - SetRandomOption(Option_t* opt); opt="isotropic" ou "random" permet soit un tirage en theta aleatoire ou isotropique

Réalistion des diffusions
   la méthode Process(Int_t ntimes) permet la réalisation de tous le processus :
   - propagation du noyau projectile dans la cible jusqu'au point d'intéraction
   - tirage d'un théta et phi pour la diffusion sur le noyau cible
   - calcul de la cinématique pour cette direction choisie (réalisé par la classe KV2Body)
   - si un KVMultiDetArray est défini et que la méthode SetDetectionOn(), détection des projectiles et cibles en voie de sortie
   - enregistrement des evts diffuses dans un arbre sous forme de KVEvent

Exemple :
---------
~~~~{.cpp}
es = new KVElasticScatterEvent();
//Defintion de la voie d'entree
es->SetSystem(54,129,8*129,28,58); 129Xe+58Ni@8MeV/A

es->SetRandomOption("isotropic"); tirage en theta isotrope
es->DefineAngularRange(6,8.5,12,30); theta 1 a 15 et phi 0 a 360

10000 diffusion
es->Process(10000);
~~~~
*/

class KVElasticScatterEvent : public KVBase {

protected:
   KV2Body*                kb2;//!
   TVector3                kIPPVector;//!
   KVTarget*               ktarget;//!
   KVNucleus*              proj;//!->
   KVNucleus*              targ;//!->
   KVReconstructedEvent*   rec_evt;//!
   KVSimEvent*                sim_evt; //!

   KVHashList*             lhisto;//! to store control histogram
   KVHashList*             ltree;//!   to store tree

   Double_t                th_min, th_max, phi_min, phi_max; //!
   Double_t                kXruth_evt;//!
   Int_t                   kchoix_layer;//!
   Int_t                   kTreatedNevts;//!    number of diffusion performed

   Int_t                   kDiffNuc;//!
   Option_t*               kRandomOpt;//!
   KVPosition              kposalea;//!
   Int_t                   kChoixSol;
   //Variables permettant de traiter les doubles solutions cinematiques
   //pour un theta donne
   /*
   Bool_t                  SecondTurn;
   Double_t                Ekdiff_ST;
   Double_t                Thdiff_ST;
   Double_t                Phdiff_ST;
   */
   void     init();
   void     GenereKV2Body();
   Bool_t   DefineTargetNucleusFromLayer(KVString layer_name = "");
   void     PropagateInTargetLayer();
   void     NewInteractionPointInTargetLayer();
   void     StartEvents();
   virtual void MakeDiffusion();
   void     SortieDeCible();

public:

   enum {
      kProjIsSet = BIT(14),      //kTRUE if projectile nucleus defined
      kTargIsSet = BIT(15),      //kTRUE if target nucleus defined
      kHasTarget = BIT(16),      //kTRUE if target material defined
      kIsUpdated = BIT(17),      //flag indicating if ValidateEntrance method has to be called
      kIsDetectionOn = BIT(18)   //flag indicating if user asked detection of events
   };

   Bool_t IsProjNucSet() const
   {
      return TestBit(kProjIsSet);
   }
   Bool_t IsTargNucSet() const
   {
      return TestBit(kTargIsSet);
   }
   Bool_t IsTargMatSet() const
   {
      return TestBit(kHasTarget);
   }
   Bool_t IsUpdated()   const
   {
      return TestBit(kIsUpdated);
   }
   Bool_t IsDetectionOn() const
   {
      return TestBit(kIsDetectionOn);
   }

   void ChooseKinSol(Int_t choix = 1);

   KVElasticScatterEvent();
   virtual ~KVElasticScatterEvent();

   virtual void SetSystem(KVDBSystem* sys);
   virtual void SetSystem(Int_t zp, Int_t ap, Double_t ekin, Int_t zt, Int_t at);
   virtual void SetTargNucleus(KVNucleus* nuc);
   virtual void SetProjNucleus(KVNucleus* nuc);

   void SetDetectionOn(Bool_t On = kTRUE);


   KVNucleus* GetNucleus(const Char_t* name) const;
   KVNucleus* GetNucleus(Int_t ii) const;

   virtual void SetTargetMaterial(KVTarget* targ, Bool_t IsRandomized = kTRUE);
   KVTarget* GetTarget() const;

   virtual TVector3& GetInteractionPointInTargetLayer();

   KVReconstructedNucleus* GetReconstructedNucleus(KVString nucname)
   {
      return GetReconstructedEvent()->GetParticleWithName(nucname);
   }
   KVReconstructedEvent* GetReconstructedEvent(void) const
   {
      return rec_evt;
   }
   KVEvent* GetSimEvent(void) const
   {
      return sim_evt;
   }

   virtual void Reset();
   virtual Bool_t ValidateEntrance();

   void SetDiffNucleus(KVString name = "PROJ");
   void SetRandomOption(Option_t* opt = "isotropic");
   Bool_t IsIsotropic() const;

   virtual void Process(Int_t ntimes = 1, Bool_t reset = kTRUE);

   virtual void SetAnglesForDiffusion(Double_t theta, Double_t phi);
   virtual void Filter();
   virtual void TreateEvent();

   virtual void DefineAngularRange(TObject*);
   void DefineAngularRange(Double_t tmin, Double_t tmax, Double_t pmin, Double_t pmax);
   Double_t GetTheta(KVString opt) const;
   Double_t GetPhi(KVString opt)const;

   KV2Body* GetKV2Body()
   {
      return kb2;
   }
   void Print(Option_t* /*opt*/ = "") const;

   virtual void ClearTrees();
   virtual void ResetTrees();
   virtual void DefineTrees();
   KVHashList* GetTrees() const;

   virtual void ClearHistos();
   virtual void DefineHistos();
   virtual void ResetHistos();
   KVHashList* GetHistos() const;

   ClassDef(KVElasticScatterEvent, 1) //simulate ElasticScatterEvent and answer of a given (multi-)detector
};

#endif
