//Created by KVClassFactory on Fri Apr 17 10:19:02 2015
//Author: John Frankland,,,

#ifndef __KVGENPHASESPACE_H
#define __KVGENPHASESPACE_H

#include "KVBase.h"
#include "KVNucleus.h"
#include "KVEvent.h"

class KVGenPhaseSpace : public KVBase {
private:
   void init();

protected:
   TObject* fMCSampler;//! Monte-Carlo phase space sampler
   KVNucleus fCompound;//initial nucleus which undergoes break-up
   KVEvent* fEvent;//! break-up channel
   Int_t fMult;// multiplicity of channel
   Double_t* fMasses;//! array of masses of nuclei in break-up channel
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
