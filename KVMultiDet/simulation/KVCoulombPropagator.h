//Created by KVClassFactory on Mon May 30 12:55:36 2016
//Author: John Frankland,,,

#ifndef __KVCOULOMBPROPAGATOR_H
#define __KVCOULOMBPROPAGATOR_H

#include "KVRungeKutta.h"
#include "KVSimEvent.h"

/**
\class KVCoulombPropagator
\brief Perform Coulomb propagation of events
\ingroup Simulation
*/

class KVCoulombPropagator : public KVRungeKutta {
   KVSimEvent* theEvent;
   Int_t fMult;

   Int_t particle_position_offset(Int_t i)
   {
      return (i - 1) * 3;
   }
   Int_t particle_velocity_offset(Int_t i)
   {
      return 3 * fMult + particle_position_offset(i);
   }

   void updateEvent();

public:
   KVCoulombPropagator(KVSimEvent*, Double_t precision = 1.e-9);

   virtual ~KVCoulombPropagator();

   void CalcDerivs(Double_t, Double_t* Y, Double_t* DYDX);

   Double_t TotalPotentialEnergy() const;

   void Propagate(int maxTime);

   ClassDef(KVCoulombPropagator, 1) //Perform Coulomb propagation of events
};

#endif
