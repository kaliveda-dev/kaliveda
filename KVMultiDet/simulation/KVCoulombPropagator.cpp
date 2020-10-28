//Created by KVClassFactory on Mon May 30 12:55:36 2016
//Author: John Frankland,,,

#include "KVCoulombPropagator.h"

#include <KVSimNucleus.h>

ClassImp(KVCoulombPropagator)

void KVCoulombPropagator::updateEvent()
{
   for (int i = 1; i <= fMult; ++i) {
      static_cast<KVSimNucleus*>(theEvent->GetParticle(i))->SetPosition(
         y[particle_position_offset(i)],
         y[particle_position_offset(i) + 1],
         y[particle_position_offset(i) + 2]
      );
      static_cast<KVSimNucleus*>(theEvent->GetParticle(i))->SetVelocity(
         TVector3(KVNucleus::C()*y[particle_velocity_offset(i)],
                  KVNucleus::C()*y[particle_velocity_offset(i) + 1],
                  KVNucleus::C()*y[particle_velocity_offset(i) + 2])
      );
   }
}

KVCoulombPropagator::KVCoulombPropagator(KVSimEvent* e, Double_t precision)
   : KVRungeKutta(e->GetMult() * 6, precision), theEvent(e), fMult(e->GetMult())
{
   // Initialise Coulomb propagation of event

   // y[0],y[1],y[2]: position of first particle (x,y,z)
   // y[3*fMult], y[3*fMult+1], y[3*fMult+2]: velocity of first particle (vx,vy,vz)

   for (int i = 1; i <= fMult; ++i) {
      for (int j = 0; j < 3; ++j) {
         y[particle_position_offset(i) + j] = static_cast<KVSimNucleus*>(e->GetParticle(i))->GetPosition()[j];
         y[particle_velocity_offset(i) + j] = static_cast<KVSimNucleus*>(e->GetParticle(i))->GetVelocity()[j] / KVNucleus::C();
      }
   }
}

//____________________________________________________________________________//

KVCoulombPropagator::~KVCoulombPropagator()
{
   // Destructor
}

void KVCoulombPropagator::CalcDerivs(Double_t, Double_t* Y, Double_t* DYDX)
{

   for (int i = 1; i <= fMult; ++i) {
      for (int j = 0; j < 3; ++j) {
         DYDX[particle_position_offset(i) + j] = Y[particle_velocity_offset(i) + j];
         DYDX[particle_velocity_offset(i) + j] = 0;
      }
   }
   for (int i = 1; i < fMult; ++i) {
      KVSimNucleus* Ni = static_cast<KVSimNucleus*>(theEvent->GetParticle(i));

      for (int j = i + 1; j <= fMult; ++j) {
         KVSimNucleus* Nj = static_cast<KVSimNucleus*>(theEvent->GetParticle(j));
         TVector3 Rij(Y[particle_position_offset(i)] - Y[particle_position_offset(j)],
                      Y[particle_position_offset(i) + 1] - Y[particle_position_offset(j) + 1],
                      Y[particle_position_offset(i) + 2] - Y[particle_position_offset(j) + 2]);
         TVector3 rij = Rij.Unit();
         Double_t F = Ni->GetZ() * Nj->GetZ() * KVNucleus::e2 / Rij.Mag2();
         TVector3 Fij = F * rij;
         Double_t mi = Ni->GetMass();
         Double_t mj = Nj->GetMass();

         for (int k = 0; k < 3; ++k) {
            DYDX[particle_velocity_offset(i) + k] += Fij[k] / mi;
            DYDX[particle_velocity_offset(j) + k] -= Fij[k] / mj;
         }
      }
   }
}

Double_t KVCoulombPropagator::TotalPotentialEnergy() const
{
   Double_t etot = 0;
   for (int i = 1; i < fMult; ++i) {
      KVSimNucleus* Ni = static_cast<KVSimNucleus*>(theEvent->GetParticle(i));

      for (int j = i + 1; j <= fMult; ++j) {
         KVSimNucleus* Nj = static_cast<KVSimNucleus*>(theEvent->GetParticle(j));
         TVector3 Rij = Ni->GetPosition() - Nj->GetPosition();
         Double_t U = Ni->GetZ() * Nj->GetZ() * KVNucleus::e2 / Rij.Mag();

         etot += U;
      }
   }
   return etot;
}

void KVCoulombPropagator::Propagate(int maxTime)
{
   Integrate(y, 0, maxTime, 1.);
   updateEvent();
}

//____________________________________________________________________________//

