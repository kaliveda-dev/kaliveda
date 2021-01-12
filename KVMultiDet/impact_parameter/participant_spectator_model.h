#ifndef PARTICIPANT_SPECTATOR_MODEL_H
#define PARTICIPANT_SPECTATOR_MODEL_H

#include "KVNucleus.h"

namespace KVImpactParameters {
   /**
   \class KVImpactParameters::participant_spectator_model
   \ingroup ImpactParameters
   \brief Formulae for participant-spectator model

   See Gosset et al., {\it Phys. Rev.} {\bf C16} (1977) 629
   */

   class participant_spectator_model {
      double A1, A2, Z1, Z2, R1, R2, _beta, NORM;
      int sector(double BETA, double NU) const;

      enum Sector
      { I = 1, II, III, IV };

   public:
      participant_spectator_model(const KVNucleus& PROJ, const KVNucleus& TARG, Bool_t normalize_b = kFALSE, Bool_t normalize_participants = kFALSE)
         : A1(PROJ.GetA()), A2(TARG.GetA()),
           Z1(PROJ.GetZ()), Z2(TARG.GetZ()),
           R1(1.2 * pow(PROJ.GetA(), 1. / 3.)),
           R2(1.2 * pow(TARG.GetA(), 1. / 3.)),
           _beta(1. / (R1 + R2))
      {
         // Constructor given entrance channel nuclei.
         //
         // \param[in] normalize_b if kTRUE, impact parameter is normalized to sum of radii

         if (normalize_b) _beta = 1;
         if (normalize_participants) NORM = total_participants(0);
         else NORM = 1;
      }
      virtual ~participant_spectator_model() {}

      double  F(double b, bool target = false) const;
      double operator()(double* x, double*)
      {
         // Returns total number of participants versus b.
         //
         // If constructor called with normalize_participants=kTRUE, will be normalized
         // to value at b=0.
         //
         // For use with ROOT TF1 class for drawing:
         //~~~~{.cpp}
         //   participant_spectator_model ps("36Ar", "58Ni", kTRUE);
         //   TF1 f_ps("PS", ps);
         //   f_ps.Draw();
         //~~~~

         return total_participants(*x) / NORM;
      }
      double total_participants(double b) const
      {
         // Returns total number of participants versus b.
         //
         // If constructor called with normalize_participants=kTRUE, will be normalized
         // to value at b=0.

         return (projectile_participants(b) + target_participants(b));
      }
      double projectile_participants(double b) const
      {
         return A1 * F(b);
      }
      double target_participants(double b) const
      {
         return A2 * F(b, true);
      }
      double proton_participants(double b) const
      {
         return (Z1 / A1) * projectile_participants(b) + (Z2 / A2) * target_participants(b);
      }

      ClassDef(participant_spectator_model, 0)
   };

}

#endif // PARTICIPANT_SPECTATOR_MODEL_H
