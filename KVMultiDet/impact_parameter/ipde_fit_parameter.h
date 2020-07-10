#ifndef IPDE_ipde_fit_parameter_H
#define IPDE_ipde_fit_parameter_H

#include "TF1.h"
namespace KVImpactParameters {

   /**
   \class KVImpactParameters::ipde_fit_parameter
   \ingroup ImpactParameters

   \brief Structure to hold value & uncertainty of fit parameters for Bayesian impact parameter estimators
    */
   struct ipde_fit_parameter {
      double value;///< value of parameter
      double error;///< estimated uncertainty/error of parameter
      double value_backup;///< stored value of parameter
      double error_backup;///< stored estimated uncertainty/error of parameter

      ipde_fit_parameter() : value(0), error(0), value_backup(0), error_backup(0) {}
      ipde_fit_parameter(const ipde_fit_parameter& o)
         : value(o.value), error(o.error), value_backup(o.value_backup), error_backup(o.error_backup) {}
      ipde_fit_parameter& operator=(const ipde_fit_parameter& o)
      {
         /// Take value and error from other object, don't change backed-up values
         /// \param[in] o the object to copy
         if (&o != this) {
            value = (o.value);
            error = (o.error);
         }
         return *this;
      }

      ///
      /// Print value and uncertainty on parameter
      ///
      void print() const
      {
         std::cout << value << "+/-" << error;
      }

      /////////////////////////////////////////////////
      /// Retrieve parameter value and error from TF1
      ///
      ///  \param[in] f pointer to TF1
      ///  \param[in] npar index of parameter in TF1
      ///
      void set(TF1* f, int npar)
      {
         value = f->GetParameter(npar);
         error = f->GetParError(npar);
      }

      ///
      /// Store current value and error in backup
      ///
      void backup()
      {
         value_backup = value;
         error_backup = error;
      }

      ///
      /// Restore backed-up value and error
      ///
      void restore()
      {
         value = value_backup;
         error = error_backup;
      }
   };
}

#endif // IPDE_ipde_fit_parameter_H
