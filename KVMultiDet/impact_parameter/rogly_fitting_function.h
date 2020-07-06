#ifndef ROGLY_FITTING_FUNCTION_H
#define ROGLY_FITTING_FUNCTION_H

#include "ipde_fit_parameter.h"
#include <TH1.h>
#include <vector>
#include "TRandom.h"

namespace KVImpactParameters {
   /**
   \class KVImpactParameters::rogly_fitting_function
   \ingroup ImpactParameters

   Exponential function relating mean value of observable to centrality

   This implements the function
   \f[
   k(c_b) = k_0 \exp{\left(-\sum_{i=1}^{N} a_i c_b^i\right)}
   \f]
   from Rogly et al., *Phys. Rev.* **C** 98, 024902(2018), describing the centrality dependence of the mean
   value of an observable which decreases monotonically with impact parameter. The degree of the
   polynomial, \f$N\f$, is given as template parameter PolyDegree.
   */

   template<int PolyDegree>
   class rogly_fitting_function {
      ipde_fit_parameter theta_p;///< fluctuation parameter \f$\theta\f$
      ipde_fit_parameter kmax;///< maximum of mean value for \f$b=0\f$
      std::vector<ipde_fit_parameter> poly_param;///< polynomial coefficients

   public:
      rogly_fitting_function()
         : poly_param(PolyDegree)
      {}
      rogly_fitting_function(const rogly_fitting_function& prev_fit)
         : theta_p(prev_fit.theta_p), kmax(prev_fit.kmax), poly_param(PolyDegree)
      {
         poly_param = prev_fit.poly_param;
      }
      constexpr int npar() const
      {
         // Number of parameters for fitting function
         return PolyDegree + 2;
      }

      double k_cb(double cb) const
      {
         // Calculate and return value of \f$k(c_b)\f$ for given centrality
         // \param[in] cb centrality
         double kcb = 0;
         for (int j = 1; j <= PolyDegree; ++j) kcb += poly_param[j - 1].value * pow(cb, j);
         return kmax.value * TMath::Exp(-kcb);
      }
      void fill_params_from_array(double* p)
      {
         // Set values of parameters from values in the array (interface to ROOT TF1)
         // \param[in] p address of an array of size given by npar()
         theta_p.value = p[0];
         kmax.value = p[1];
         for (int j = 1; j <= PolyDegree; ++j) poly_param[j - 1].value = p[j + 1];
      }
      void fill_array_from_params(double* p) const
      {
         // Copy values of parameters into the array (interface to ROOT TF1)
         // \param[in] p address of an array of size given by npar()
         p[0] = theta_p.value;
         p[1] = kmax.value;
         for (int j = 1; j <= PolyDegree; ++j) p[j + 1] = poly_param[j - 1].value ;
      }
      double theta() const
      {
         // Get value of \f$\theta\f$ parameter
         return theta_p.value;
      }
      void set_par_names(TF1& f) const
      {
         // Set name of parameters in TF1 object
         // \param[in,out] f ROOT function using this implementation of \f$k(c_b)\f$
         f.SetParName(0, "#theta");
         f.SetParName(1, "k_{max}");
         for (int j = 1; j <= PolyDegree; ++j) f.SetParName(j + 1, Form("a_{%d}", j));
      }
      void set_initial_parameters(TH1* h, TF1& f)
      {
         // Set initial values for parameters in TF1 object based on data in histogram h
         // \param[in] h histogram with data (inclusive distribution of observable, \f$P(X)\f$) to be fitted
         // \param[in,out] f ROOT function using this implementation of \f$k(c_b)\f$
         f.SetParameter(0, theta_p.value = 0.5);
         f.SetParLimits(0, 0.1, 5);
         double xmax = h->GetXaxis()->GetBinUpEdge(h->GetNbinsX());
         double xmin = xmax / 7.;
         f.SetParameter(1, kmax.value = (xmin * 2) / theta_p.value);
         f.SetParLimits(1, xmin / theta_p.value, xmax / theta_p.value);
         for (int j = 1; j <= PolyDegree; ++j) {
            f.SetParameter(j + 1, gRandom->Uniform(-1, 1));
            f.SetParLimits(j + 1, -2, 2);
         }
      }
      void print_fit_params() const
      {
         std::cout << "Theta = ";
         theta_p.print();
         std::cout << "\n";
         std::cout << "kmax = ";
         kmax.print();
         std::cout << "(Xmax = " << theta_p.value* kmax.value << ")";
         std::cout << "\n";
         for (int j = 1; j <= PolyDegree; ++j) {
            std::cout << "a" << j << " = ";
            poly_param[j - 1].print();
            std::cout << "  ";
         }
         std::cout << "\n";
      }
      void backup_params()
      {
         // Store all current values of parameters in backup
         theta_p.backup();
         kmax.backup();
         std::for_each(poly_param.begin(), poly_param.end(), [](ipde_fit_parameter & p) {
            p.backup();
         });
      }
      void restore_params()
      {
         // Retore all current values of parameters from backup
         theta_p.restore();
         kmax.restore();
         std::for_each(poly_param.begin(), poly_param.end(), [](ipde_fit_parameter & p) {
            p.restore();
         });
      }
      void normalise_shape_function()
      {
         // Modify values of parameters so that \f$k(c_b)\f$ varies between 1 at \f$c_b=0\f$ to 0 at \f$c_b=1\f$.
         // Currently this just scales \f$k_{max}\f$ to 1, the minimum at \f$c_b=1\f$ will not be zero (to be implemented).
         kmax.value = 1;
      }

      ClassDef(rogly_fitting_function, 0) //fit using exponential polynomial
   };
}

#endif // ROGLY_FITTING_FUNCTION_H
