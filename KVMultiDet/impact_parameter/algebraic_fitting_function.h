#ifndef ALGEBRAIC_FITTING_FUNCTION_H
#define ALGEBRAIC_FITTING_FUNCTION_H

#include "ipde_fit_parameter.h"
#include <TH1.h>
/**
  \defgroup ImpactParameters The Impact Parameter module

  Tools for estimating impact parameters for experimental data
 */
namespace KVImpactParameters {
   /** \class KVImpactParameters::algebraic_fitting_function
     \ingroup ImpactParameters

   \brief Algebraic relationship between mean value of observable and centrality

   This class implements the relationship between the mean value of an observable and centrality

   \f[
   k(c_b) = (k_{\mathrm{max}}-k_{\mathrm{min}})\left[ 1 - {c_b}^{\alpha}\right]^{\gamma} + k_{\mathrm{min}}
   \f]

   used in Frankland et al., *Phys. Rev.* **C** xx, yy(2020), describing the centrality dependence of the mean
   value of an observable which decreases monotonically with impact parameter. This function is guaranteed to
   be mononotonic in the range \f$c_b=[0,1]\f$ for all values of the parameters \f$\alpha,\gamma>0\f$.

   \sa bayesian_estimator
         */
   class algebraic_fitting_function {
      /** \class KVImpactParameters::algebraic_fitting_function::bce_fit_results
      \brief Contains fit parameters for algebraic impact parameter dependence
      \sa KVImpactParameters::algebraic_fitting_function
            */
      class bce_fit_results {
      public:
         ipde_fit_parameter k0; // minimum of k at c=1
         ipde_fit_parameter alpha; // power of centrality
         ipde_fit_parameter gamma; // power of (1-pow(c,alpha))
         ipde_fit_parameter theta; // the (b-independent) fluctuation width Eqs(1),(2)
         ipde_fit_parameter kmax; // value of k at b=0 Eq(8)
         ipde_fit_parameter Xmax; // value of X at b=0
         double chisquare;
         double ndf;

         bce_fit_results() = default;
         bce_fit_results(const bce_fit_results& other)
            : k0(other.k0), alpha(other.alpha), gamma(other.gamma), theta(other.theta), kmax(other.kmax), Xmax(other.Xmax)
         {}
         bce_fit_results& operator=(const bce_fit_results& other)
         {
            if (&other != this) {
               k0 = (other.k0);
               alpha = (other.alpha);
               gamma = (other.gamma);
               theta = (other.theta);
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
            std::cout << "   theta = ";
            theta.print();
            std::cout << std::endl;
         }
      } params, param_backup;
   public:
      algebraic_fitting_function() {}
      algebraic_fitting_function(const algebraic_fitting_function&);
      algebraic_fitting_function(double alph, double gam, double thet, double Xmi, double Xma);
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
         params.kmax.value = 1. / params.theta.value;
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
      double theta() const
      {
         return params.theta.value;
      }
      void set_par_names(TF1& f) const;
      void set_initial_parameters(TH1* h, TF1& f);
      void print_fit_params() const
      {
         params.Print();
      }
      double meanX(double cb) const
      {
         return theta() * k_cb(cb);
      }
      double redVar(double cb) const
      {
         return theta();
      }
   };

}

#endif // ALGEBRAIC_FITTING_FUNCTION_H
