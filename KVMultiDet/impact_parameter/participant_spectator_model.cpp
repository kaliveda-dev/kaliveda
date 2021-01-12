#include "participant_spectator_model.h"

namespace KVImpactParameters {

   int participant_spectator_model::sector(double BETA, double NU) const
   {
      // Return sector number according to values of \f$\nu\f$ and \f$\beta\f$
      //
      // [See Fig. 35 of Gosset et al.]

      if (NU > 0.5 * (1 + BETA)) return Sector::I;
      if (NU > 0.5) return Sector::II;
      if (NU > 0.5 * (1 - BETA)) return Sector::III;
      return Sector::IV;
   }

   double participant_spectator_model::F(double b, bool target) const
   {
      // The number of participant nucleons in a spherical nucleus of mass number \f$A_1\f$ and radius \f$R_1\f$,
      // aimed with impact parameter \f$b\f$ at a spherical nucleus of mass number \f$A_2\f$ and radius \f$R_2\f$
      // is given by
      //\f[
      //N_1=A_1 F(\nu,\beta)
      //\f]
      //
      // \param[in] b impact parameter of collision. This is given in [fm] unless constructor was called with normalize_b=kTRUE: then \f$0\leq b\leq 1\f$.

      double BETA = b * _beta;
      double NU = 1. / (R1 + R2);
      NU *= (target ? R2 : R1 * _beta);
      double MU = 1. / NU - 1.;
      switch (sector(BETA, NU)) {
         case Sector::IV:
            return 1;
         case Sector::III:
            return 0.75 * pow(1. - NU, 0.5) * pow((1 - BETA) / NU, 2) - 0.125 * pow((1 - BETA) / NU, 3.)
                   * (3 * pow(1 - NU, 0.5) - 1);
         case Sector::II:
            return 0.75 * pow(1. - NU, 0.5) * pow((1 - BETA) / NU, 2) - 0.125 * pow((1 - BETA) / NU, 3.)
                   * (3 / MU * pow(1 - NU, 0.5) - (1 - pow(1 - MU * MU, 1.5)) * pow(1 - pow(1 - MU, 2), 0.5) / pow(MU, 3));
         case Sector::I:
            return (1 - pow(1 - MU * MU, 1.5)) * pow(1 - pow(BETA / NU, 2), 0.5);
      }
      return 0;
   }


}
