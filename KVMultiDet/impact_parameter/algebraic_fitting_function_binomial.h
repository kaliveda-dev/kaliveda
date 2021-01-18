#ifndef ALGEBRAIC_FITTING_FUNCTION_BINOMIAL_H
#define ALGEBRAIC_FITTING_FUNCTION_BINOMIAL_H

#include "ipde_fit_parameter.h"
#include <TH1.h>

namespace KVImpactParameters {
   /** \class KVImpactParameters::algebraic_fitting_function_binomial
     \ingroup ImpactParameters

   \brief Algebraic relationship between mean value of observable and centrality (binomial kernel)

   Specialised version of algebraic_fitting_function to be used only with binomial distribution kernel.
   Instead of the theta (reduced variance) fit parameter which was assumed constant for all b,
   in this case the probablity p for the binomial is calculated as mean_obs(b)/NMAX, where NMAX is a new
   fit parameter assumed constant for all b. In this case the reduced variance is given by 1-p=1-mean_obs(b)/NMAX,
   and therefore is not constant for all b.

   \sa bayesian_estimator
         */
   class algebraic_fitting_function_binomial {
      /** \class KVImpactParameters::algebraic_fitting_function_binomial::bce_fit_results
      \brief Contains fit parameters for algebraic impact parameter dependence
      \sa KVImpactParameters::algebraic_fitting_function_binomial
            */
      class bce_fit_results {
      public:
         ipde_fit_parameter k0; // minimum of k at c=1
         ipde_fit_parameter alpha; // power of centrality
         ipde_fit_parameter gamma; // power of (1-pow(c,alpha))
         ipde_fit_parameter NMAX; // the (b-independent) NMAX
         ipde_fit_parameter kmax; // value of k at b=0 Eq(8)
         ipde_fit_parameter Xmax; // value of X at b=0
         double chisquare;
         double ndf;

         bce_fit_results() = default;
         bce_fit_results(const bce_fit_results& other)
            : k0(other.k0), alpha(other.alpha), gamma(other.gamma), NMAX(other.NMAX), kmax(other.kmax), Xmax(other.Xmax)
         {}
         bce_fit_results& operator=(const bce_fit_results& other)
         {
            if (&other != this) {
               k0 = (other.k0);
               alpha = (other.alpha);
               gamma = (other.gamma);
               NMAX = (other.NMAX);
               kmax = (other.kmax);
               Xmax = (other.Xmax);
            }
            return *this;
         }
         void Print(Option_t* = "") const
         {
            //std::cout << "Chi-2 = " << chisquare << " NDF = " << ndf << " (" << chisquare / ndf << ")" << std::endl;
            std:: cout << "   kmax = ";
            kmax.print();
            std::cout << "   k0 = ";
            k0.print();
            std::cout << "   alpha = ";
            alpha.print();
            std:: cout << "   gamma = ";
            gamma.print();
            std::cout << "   NMAX = ";
            NMAX.print();
            std::cout << std::endl;
         }
      } params, param_backup;
   public:
      algebraic_fitting_function_binomial() {}
      algebraic_fitting_function_binomial(const algebraic_fitting_function_binomial&);
      algebraic_fitting_function_binomial(double alph, double gam, double nmax, double Xmi, double Xma);
      void backup_params()
      {
         // Store current values of parameters
         param_backup = params;
      }
      void restore_params()
      {
         // Restore previously stored parameter values
         params = param_backup;
      }
      void normalise_shape_function()
      {
         // Modify values of parameters so that \f$k(c_b)\f$ varies between 1 at \f$c_b=0\f$ to 0 at \f$c_b=1\f$.
//         params.kmax.value = 1. / params.theta.value;
         params.k0.value = 0;
         params.Xmax.value = 1;
      }

      constexpr int npar() const
      {
         return 5;
      }
      double k_cb(double cb) const;
      void fill_params_from_array(double* p);
      void fill_array_from_params(double* p) const;
      double nmax() const
      {
         return params.NMAX.value;
      }
      void set_par_names(TF1& f) const;
      void set_initial_parameters(TH1* h, TF1& f);
      void print_fit_params() const
      {
         params.Print();
      }
      double meanX(double cb) const
      {
         return k_cb(cb);
      }
      double redVar(double cb) const
      {
         return 1 - meanX(cb) / nmax();
      }
   };

}

#endif // ALGEBRAIC_FITTING_FUNCTION_H
