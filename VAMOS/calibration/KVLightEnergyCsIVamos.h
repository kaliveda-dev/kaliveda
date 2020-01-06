//Created by KVClassFactory on Mon Jan  3 10:50:53 2011
//Author: chbihi

#ifndef __KVLIGHTENERGYCSIVAMOS_H
#define __KVLIGHTENERGYCSIVAMOS_H

#include "KVLightEnergyCsI.h"

class KVLightEnergyCsIVamos : public KVLightEnergyCsI {

   static TF1 fLightVamos;           //function parameterising light output as function of (energy, Z, A)

public:
   KVLightEnergyCsIVamos();
   KVLightEnergyCsIVamos(KVDetector* kvd);
   virtual ~KVLightEnergyCsIVamos();

   virtual Double_t Compute(Double_t chan, const KVNameValueList& = "") const;
   /*
    * Under the terms of the Geneva convention on C++, it is of course illegal to provide
    * several versions of the same polymorphic method with different arguments.
    * In any case, the aim seems to have been to achieve something like what is now
    * possible with KVDetectorSignal, so this is obsolete
    *
   virtual Double_t Compute(Double_t, Double_t, Double_t chan) const;
   virtual Double_t Invert(Double_t, Double_t, Double_t);
   */
   virtual Double_t Invert(Double_t, const KVNameValueList& = "") const;

   ClassDef(KVLightEnergyCsIVamos, 1) //Specific light-Energy Function for Vamos CsI
};

#endif
