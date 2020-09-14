//Created by KVClassFactory on Fri Sep 11 23:31:56 2020
//Author: henri

#ifndef __KVSIMREADER_DIT_H
#define __KVSIMREADER_DIT_H

#include "KVSimReader.h"
#include "KVString.h"

class KVSimReader_DIT : public KVSimReader {
public:
   KVSimReader_DIT();

   virtual ~KVSimReader_DIT();

private:
   KVString projectile;
   KVString target;

   Double_t energy;
   Double_t weight;
   Double_t inicident_partial_wave;  //!< Incident partial wave in hbar (to determine the impact parameter) in the center of mass
   Double_t excitation_energy_proj;  //!< Projectile like excitation energy (thermal + rotational) in MeV
   Double_t excitation_energy_targ;  //!< Target like excitation energy (thermal + rotational) in MeV
   Double_t spin_proj;  //!< Projectile like spin in hbar(only components normal to the reaction plane)
   Double_t spin_targ;  //!< Target like spin in hbar(only components normal to the reaction plane)

   Int_t itype;
   Int_t a_proj;  //!< Projectile like mass number
   Int_t a_targ;  //!< Target like mass number
   Int_t z_proj;  //!< Projectile like charge number
   Int_t z_targ;  //!< Target like charge number

   ClassDef(KVSimReader_DIT, 1) //
};

#endif









WEIGHT / F:
ANGM0:
IMPAR:
ITYPE / I:
A[2]:
Z[2]:
EX[2] / F:
SPIN[2]:
ELAB[2]:
THETA[2]:
THCM:
Q:
KN / I:
KP:
SMIN / F


C  SUBROUTINE TRAJEX performs trajectory calcution by random drawing
C(Monte Carlo method) of nucleon transfers and associated dissipation
C  At the end we get 2 primary fragments with excitation energy and
C   spin. At this point no nucleon has escaped neither by
C   evaporation nor preequilibrium emission :
all are in fragments.
C
C  CAUTION :
this version enables transfer of nucleons running from the
C    the window(same beheavior as Randrup's calculations leading to
                C    a quadratic dependence of dissipation with the velocity).
c      For the calculation of the transfer probability, this
C    is equivalent to keep RL > 0 and change the sign of THETA and PRR.
C
C  Before any call to the TRAJEX routine, an initialization must be done
C  to pre - calculate parameters. This is done by :
c  - A Fortran open on unit 3 assigned to the file waps.dat which contains
c      mass excesses of known isotopes(from Wapstra and Audi)
C  - CALL INITRAJ(IAP, IZP, IAT, IZT) where IAP(IAT) and IZP(IZT) are mass
C     and charge of projectile(target).
C
C  The user must provide with a function ranf which delivers a random generator
c   uniformly distributed between 0 and 1. This function form is :
c     real * 4 function ranf()
c  This function must be included at the link level
C
C  After this initialization step, TRAJEX is called for every event to
C    be generated :
C  CALL TRAJEX(IW, ELAB0, ANGM0, ITYPE, IAA, IZZ, EX, TT, SPIN, EROT,
               C    1 ELAB, THT, THETCM, Q, KN, KP, SMIN)
   C
   C
C      INPUT :
   C
C   IW    :
   type of calculation
C           = 0 :
                 only nucleons directed to the window are transferred
                 C                 -> no orbiting
                 C                 -> low dissipation in the seperation phase
              C          <>0 :
                 nucleons running from the window can be transferred
                 C                  this setting corresponds to Randrup's model
                 c                   -> orbiting effect
                 c                   -> quadratic dependence of dissipation with velocity
              C   ELAB0 :
                 total lab. bombarding energy(projectile) in MeV
              C   ANGM0 :
                 incident partial wave in hbar(to determine the impact
                       C                          parameter) in the center of mass
              C           The maximal angular momentum lmax can be calculated as follows :
              c            - minimal distance of approach for grazing collisions :
                 c                 d = 1.2 * (Ap** 0.33333 + At** 0.33333) + 2.(in fm)
                                    c            - coulomb barrier :
                                          Vcoul = 1.44 * Zp * Zt / d(in MeV)
                                          c            - reduced mass :
                                                Ared = Ap * At / (Ap + At)
                                                c            - lab incident projectile energy in MeV :
                                                      Ep
                                                      c            - lmax = 0.2191 * d * sqrt(Ared * (Ep / Ap * Ared - Vcoul))(in hbar)
                                                            c   If a number of events proportional to ANGM0 is drawn the cross section
                                                            c of a process is determined from the number of events contributing to this
                                                            c process(N') and the total number of drawn events (N) :
                                                                  c  sigma = pi * d ^ 2 * (1 - Vcoul / Ep* Ap / Ared) * N'/N
                                                                        C
                                                                        C        OUTPUT :
                                                                        C
                                                                        C   ITYPE  : = 0  normal DIC collision
                                                                              C            = -1 an error occured, the trajectory must be replayed
                                                                  C            > 0  fusion occured :
                                                                  C              = 1  collective energy(Ekin + Pot) < 1 MeV
                                                                        C              = 2  a forbidden region for collective motion is reached
                                                                              C              = 3  system trapped in the  interaction potential
                                                                                    C              = 4  system too compact(SMIN < -3fm)
                                                                                          C   IAA(2)  :  IAA(1) and IAA(2) projectile - like and target - like masses
                                                                                          C   IZZ(2)  : projectile - like and target - like Z's
                                                                                          C   EX(2)   : total excitation energies of final fragments :
                                                                                          C               thermal + rotational energy(MeV)
                                                                                          C   TT(2)   : temperatures of final fragments(MeV)
                                                                                          C   SPIN(2) : spins of final fragments(hbar)(only components normal
                                                                                                C                      to the reaction plane)
                                                                                          C   EROT(2) : rotationnal energies of final fragments(MeV)
                                                                                          C   ELAB(2) : lab. kinetic energies of projectile - like and target - like
                                                                                          C               fragments
                                                                                          C   THT(2)  : lab. scattering angle of fragments(degrees)
                                                                                          C   THETCM  : cm.  scattering angle(degrees)
                                                                                          C   Q       : reaction Q value(MeV)
                                                                                          C   KN, KP   : numbers of neutrons and protons exchanged
                                                                  C   SMIN   : minimal distance of approach(fm) between nuclear surfaces
