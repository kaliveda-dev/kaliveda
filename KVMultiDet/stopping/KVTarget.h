/***************************************************************************
                          KVTarget.h  -  description
                             -------------------
    begin                : 2/12/2003
    copyright            : (C) 2003 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTarget.h,v 1.23 2008/12/11 16:39:47 ebonnet Exp $
 ***************************************************************************/
#ifndef _KV_TARGET_H_
#define _KV_TARGET_H_

#include "KVNucleus.h"
#include "KVMaterial.h"

class KVEvent;

class KVTarget: public KVMaterial {

private:

   enum {
      kRandom = BIT(14),        //random interaction depth or half-way ?
      kIncoming = BIT(15),      //calculate only energy loss from entrance up to interaction depth
      kOutgoing = BIT(16)       //calculate energy loss from interaction depth to exit
   };

protected:

   KVList* fTargets;           //list of layers
   Int_t fNLayers;              //number of layers
   TVector3 fNormal;            //normal to target - (0,0,1) by default. Keep for backwards compatibility!!
   TVector3 fIntPoint;          //last randomly generated interaction point

   void init();

public:

   KVTarget();
   KVTarget(const KVTarget&);
   KVTarget(const Char_t* material, Double_t thick = 0.0);
   virtual ~ KVTarget();

   const TVector3& GetNormal()
   {
      // Return vector normal to target
      return fNormal;
   };
   void SetAngleToBeam(Double_t a);
   Double_t GetAngleToBeam();

   virtual void SetMaterial(const Char_t* type);
   void SetLayerThickness(Float_t thick, Int_t ilayer = 1);

   void AddLayer(const Char_t* material, Double_t thick);
   Int_t NumberOfLayers() const
   {
      return fNLayers;
   };
   KVList* GetLayers() const
   {
      return fTargets;
   };
   KVMaterial* GetLayerByIndex(Int_t ilayer) const
   {
      return (ilayer >
              0 ? (ilayer <=
                   NumberOfLayers() ? (KVMaterial*) GetLayers()->
                   At(ilayer - 1) : 0) : 0);
   };
   KVMaterial* GetLayer(TVector3& depth);
   Int_t GetLayerIndex(TVector3& depth);
   KVMaterial* GetLayerByDepth(Double_t depth);
   Int_t GetLayerIndex(Double_t depth);
   KVMaterial* GetLayer(const Char_t* name);
   Int_t GetLayerIndex(const Char_t* name);

   Double_t GetTotalThickness();
   Double_t GetTotalThickness(Int_t lay1, Int_t lay2);
   Double_t GetThickness() const
   {
      return const_cast <KVTarget* >(this)->GetTotalThickness();
   };
   Double_t GetTotalEffectiveThickness(KVParticle* part = 0);
   Double_t GetTotalEffectiveThickness(TVector3&, Int_t lay1 =
                                          1, Int_t lay2 = 0);
   Double_t GetEffectiveThickness(KVParticle* part = 0, Int_t ilayer = 1);
   Double_t GetEffectiveThickness(TVector3& direction, Int_t ilayer = 1);
   Double_t GetThickness(Int_t ilayer) const;

   TVector3& GetInteractionPoint(KVParticle* part = 0);
   void SetInteractionLayer(Int_t ilayer, TVector3& dir);
   void SetInteractionLayer(const Char_t* name, TVector3& dir);
   void SetInteractionLayer(const Char_t* name, KVParticle* part);

   Bool_t IsIncoming() const
   {
      return TestBit(kIncoming);
   };
   // Set mode of target for calculation of energy loss.
   //    target->SetIncoming()
   //    target->SetIncoming(kTRUE) : energy loss of particles calculated along beam direction up to
   //                                 interaction point inside target. Note that this automatically sets
   //                                 the 'Outgoing' flag to false.
   //    target->SetIncoming(kFALSE) : if used with target->SetOutgoing(kFALSE), energy loss
   //                                  calculated for particle passing through the whole of the target
   void SetIncoming(Bool_t r = kTRUE)
   {
      if (r) {
         SetBit(kIncoming);
         ResetBit(kOutgoing);
      } else
         ResetBit(kIncoming);
   };
   Bool_t IsOutgoing() const
   {
      return TestBit(kOutgoing);
   };
   // Set mode of target for calculation of energy loss.
   //    target->SetOutgoing()
   //    target->SetOutgoing(kTRUE) : energy loss calculated from interaction point inside
   //                                 target upto exit of particle from target taking into account
   //                                 particle's direction of motion. Note that this automatically sets
   //                                 the 'Incoming' flag to false.
   //    target->SetOutgoing(kFALSE) : if used with target->SetIncoming(kFALSE), energy loss
   //                                  calculated for particle passing through the whole of the target
   void SetOutgoing(Bool_t r = kTRUE)
   {
      if (r) {
         SetBit(kOutgoing);
         ResetBit(kIncoming);
      } else
         ResetBit(kOutgoing);
   };

   Bool_t IsRandomized() const
   {
      return TestBit(kRandom);
   };
   void SetRandomized(Bool_t r = kTRUE)
   {
      if (r)
         SetBit(kRandom);
      else
         ResetBit(kRandom);
   };

   virtual void DetectParticle(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetELostByParticle(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetParticleEIncFromERes(KVNucleus*, TVector3* norm = 0);
   virtual Double_t GetIncidentEnergyFromERes(Int_t Z, Int_t A,
         Double_t Eres);
   void DetectEvent(KVEvent*);

   void Print(Option_t* opt = "") const;
   void Clear(Option_t* opt = "");
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   // virtual UInt_t GetUnits() const;

   Double_t GetAtomsPerCM2() const;

   ClassDef(KVTarget, 1)        //Simulate targets for experiments
};



#endif
