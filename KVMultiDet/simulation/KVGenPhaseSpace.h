//Created by KVClassFactory on Fri Apr 17 10:19:02 2015
//Author: John Frankland,,,

#ifndef __KVGENPHASESPACE_H
#define __KVGENPHASESPACE_H

#include "KVBase.h"
#include "KVNucleus.h"
#include "KVEvent.h"

/**
\class KVGenPhaseSpace
\brief Generate momenta for an event using microcanonical phase space sampling
\ingroup Simulation

 We use ROOT class TGenPhaseSpace to sample the phase space for each event.
 This corresponds to sampling the microcanonical phase space for a gas
 composed of the nuclei of the break-up channel with a given total kinetic
 energy determined by energy balance.

### Example of use

~~~~{.cpp}
 // 3-alpha decay of 12C*

 KVNucleus CN(6,12);
 CN.SetExcitEnergy(20); // minimum = 7.27 MeV

 //  decay products
 KVEvent decay;
 KVNucleus* n = decay.AddParticle();
 n->SetZandA(2,4);
 n = decay.AddParticle();
 n->SetZandA(2,4);
 n = decay.AddParticle();
 n->SetZandA(2,4);

 KVGenPhaseSpace gps;
 if(!gps.SetBreakUpChannel(CN, &decay))
 {
   printf("Break-up channel is not allowed\n");
   return;
 }

 Double_t etot = gps.GetAvailableEnergy();
 TH1F* h1 = new TH1F("h1","Kinetic energy of alpha particle 3",200,0,etot*2./3.);
 h1->Sumw2();

 Int_t nevents = 10000;
 while(nevents--){
   wgt = gps.Generate();
   h1->Fill(n->GetKE(),wgt);
 }

 h1->Draw();

~~~~
*/

class KVGenPhaseSpace : public KVBase {
private:
   void init();

protected:
   TObject* fMCSampler;//! Monte-Carlo phase space sampler
   KVNucleus fCompound;//initial nucleus which undergoes break-up
   KVEvent* fEvent;//! break-up channel
   Int_t fMult;// multiplicity of channel
   std::vector<Double_t> fMasses;//! masses of nuclei in break-up channel
   Bool_t fOK;// ready to calculate or not
   Double_t fEtot;// available kinetic energy for decay

   Bool_t CheckBreakUpChannel();
   virtual void InitialiseMCSampler();

public:
   KVGenPhaseSpace();
   KVGenPhaseSpace(const Char_t* name, const Char_t* title = "");
   virtual ~KVGenPhaseSpace();
   void Copy(TObject& obj) const;

   Bool_t SetBreakUpChannel(const KVNucleus& CN, KVEvent* e);

   virtual Double_t Generate();

   Double_t GetAvailableEnergy() const
   {
      // Return total kinetic energy available for break-up particles
      return fEtot;
   }

   ClassDef(KVGenPhaseSpace, 1) //Generate momenta for an event using microcanonical phase space sampling
};

#endif
