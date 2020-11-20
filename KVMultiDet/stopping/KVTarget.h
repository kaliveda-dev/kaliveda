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
#include "KVEvent.h"

/**
\class KVTarget
\brief Calculation/correction of energy losses of particles through an experimental target
\ingroup Stopping

## MULTILAYER TARGET

In order to make a target with several absorber layers, proceed as follows:

~~~~~~~{.cpp}
   KVTarget targ("Ta", 8.3); // first layer 8.3mg/cm2 Ta
   targ.AddLayer("C", 20.0); // second layer 20mg/cm2 C
~~~~~~~

Note that all "thicknesses" for targets and target layers are in \f$mg/cm^2\f$.

## ORIENTATION OF TARGET

The target can be oriented in an arbitrary way, by defining the vector normal to its
surface. By default, this vector is \f$(0,0,1)\f$ i.e. in the beam-direction. You can access
this vector in order to modify it using GetNormal().

In order to change the target's angle to the beam, as for a rotation about the
+ve x-axis (12 o'clock in lab frame) use SetAngleToBeam() with an angle in degrees.
i.e. to set the angle of the target in the previous example to 30deg. w.r.t. the beam,
~~~~~~~{.cpp}
   targ.SetAngleToBeam( 30.0 );
~~~~~~~

## CALCULATE ENERGY LOSSES OF PARTICLES IN TARGET

To calculate the energy loss of particles created in a reaction in the target, set the
target mode to outgoing and then use GetELostByParticle or DetectParticle:
~~~~~~~{.cpp}
   KVNucleus *part;                                                         //pointer to particle
   targ.SetOutgoing();                                                     //calculate energy loss for particles leaving target
   Double_t eloss = targ.GetELostByParticle(part);
~~~~~~~
By default, the calculation is made for particles leaving an interaction point half-
way through the target along the beam direction \f$(0,0,1)\f$. You can change this by
calling SetRandomized() and/or calling GetInteractionPoint() with a pointer to a
supposedly incident particle whose direction is different from that of the beam.

To simulate the energy losses of all particles in an event, use DetectEvent().

## ELASTIC/INELASTIC SCATTERING

To simulate scattering requires:
  - to set an interaction point (IP) in the target
  - calculate the energy lost by an incident particle in the target up to the IP
  - calculate the energy losses of the outgoing particle(s) after scattering from the IP

\sa KVElasticScatter

### 1.) SETTING AN INTERACTION POINT
#### a.) Interaction point anywhere in the target
Calling GetInteractionPoint() with a pointer to the incident particle will generate a new
interaction point, either halfway along the particle's trajectory through the target (default)
or at a random point along the trajectory if SetRandomized() has previously been called.

Suppose that `KVNucleus* proj` is a pointer to the incident beam particle.
Then, to set the interaction point a random distance inside the target:
~~~~~~{.cpp}
   targ.SetRandomized(); // only needs to be called once, all IPs are randomly generated afterwards
   TVector3 IP = targ.GetInteractionPoint(proj);
~~~~~~
Subsequent calls to GetInteractionPoint() WITHOUT an argument return the last
generated IP vector
~~~~~~{.cpp}
   TVector3 IP2 = targ.GetInteractionPoint(); // IP2 and IP are the same
~~~~~~

#### b.) Interaction point in a specific layer of the target
You can choose in which layer of the target you want to set the IP. To indicate the
layer you can either give the name of the type of material it is made of (as long as no two
layers are of the same type!), or the number of the layer. Layers are numbered 1,2, etc. in the
order they are added to the target.

Suppose that `KVNucleus* proj` is a pointer to the incident beam particle.
Then, to set the interaction point inside the carbon layer of our example target:
~~~~~~{.cpp}
   targ.SetInteractionLayer("C", proj); // <==> targ.SetInteractionLayer(2,proj) using index
   TVector3 IP = targ.GetInteractionPoint(); // get generated IP
~~~~~~
Note that if SetRandomized() has been used the IP will be set at a random depth inside
the layer, or halfway through it by default.

### 2.) ENERGY LOSS OF INCIDENT PARTICLE BEFORE SCATTERING

To calculate the energy lost by incident particles in the target before they reach the IP,
first set the 'mode' of the target to 'incoming':
~~~~~{.cpp}
   targ.SetIncoming();
   targ.DetectParticle(proj); //particle passes through target up to interaction point
~~~~~

### 3.) ENERGY LOSS OF PARTICLES AFTER SCATTERING

Set the 'mode' of the target to 'outgoing' and then:
~~~~~{.cpp}
   targ.SetOutgoing();
   targ.DetectParticle(proj);
~~~~~

## CORRECTION OF PARTICLE ENERGIES FOR TARGET ENERGY LOSS
See method GetParticleEIncFromERes().
 */

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
   TVector3 fNormal;            //normal to target - (0,0,1) by default
   TVector3 fIntPoint;          //last randomly generated interaction point

   void init();

public:

   KVTarget();
   KVTarget(const KVTarget&);
   ROOT_COPY_ASSIGN_OP(KVTarget)
   KVTarget(const Char_t* material, Double_t thick = 0.0);
   virtual ~ KVTarget();

   const TVector3& GetNormal()
   {
      // Return vector normal to target
      return fNormal;
   }
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
      }
      else
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
      }
      else
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
