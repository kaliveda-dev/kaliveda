#include "algebraic_fitting_function_binomial.h"

templateClassImp(KVImpactParameters::algebraic_fitting_function_binomial)

namespace KVImpactParameters {

   /// Copy parameters from other
   algebraic_fitting_function_binomial::algebraic_fitting_function_binomial(const algebraic_fitting_function_binomial& other)
      : params(other.params)
   {
   }

   /// Use parameter values from previous fit
   /// \param[in] alph \f$\alpha\f$
   /// \param[in] gam \f$\gamma\f$
   /// \param[in] nmax \f$\nmax\f$
   /// \param[in] Xmi \f$X_{min}\f$
   /// \param[in] Xma \f$X_{max}\f$
   algebraic_fitting_function_binomial::algebraic_fitting_function_binomial(double alph, double gam, double n_max, double Xmi, double Xma)
   {
      double p[] = {n_max, Xma, Xmi, alph, gam};
      fill_params_from_array(p);
   }

   /// Calculate and return value of \f$k(c_b)\f$ for given centrality
   /// \param[in] cb centrality
   double algebraic_fitting_function_binomial::k_cb(double cb) const
   {
      if (params.alpha.value <= 0) return 0;
      if (params.gamma.value <= 0) return 0;
      if (cb < 0 || cb > 1) return 0;
      double arg = 1. - TMath::Power(cb, params.alpha.value);
      if (arg < 0) return 0;
      return params.kmax.value * TMath::Power(arg, params.gamma.value) + params.k0.value;
   }

   /// Set values of parameters from values in the array (interface to ROOT TF1)
   /// \param[in] p address of an array of size given by npar()
   /// \param[in] p[0] \f$N_{max}\f$
   /// \param[in] p[1] \f$X_{max}\f$
   /// \param[in] p[2] \f$X_{min}\f$
   /// \param[in] p[3] \f$\alpha\f$
   /// \param[in] p[4] \f$\gamma\f$
   void algebraic_fitting_function_binomial::fill_params_from_array(double* p)
   {
      params.NMAX.value = p[0];
      params.Xmax.value = p[1];
      params.k0.value = p[2];
      params.kmax.value = params.Xmax.value - params.k0.value;
      params.alpha.value = p[3];
      params.gamma.value = p[4];
   }

   /// Copy values of parameters into the array (interface to ROOT TF1)
   /// \param[in] p address of an array of size given by npar()
   /// \param[in] p[0] \f$N_{max}\f$
   /// \param[in] p[1] \f$X_{max}\f$
   /// \param[in] p[2] \f$X_{min}\f$
   /// \param[in] p[3] \f$\alpha\f$
   /// \param[in] p[4] \f$\gamma\f$
   void algebraic_fitting_function_binomial::fill_array_from_params(double* p) const
   {
      p[0] = params.NMAX.value;
      p[1] = params.Xmax.value;
      p[2] = params.k0.value;
      p[3] = params.alpha.value;
      p[4] = params.gamma.value;
   }

   /// Set name of parameters in TF1 object
   /// \param[in,out] f ROOT function using this implementation of \f$k(c_b)\f$
   void algebraic_fitting_function_binomial::set_par_names(TF1& f) const
   {
      f.SetParNames("N_{max}", "X_{max}", "X_{min}", "#alpha", "#gamma");
   }

   /// Set initial values for parameters in TF1 object based on data in histogram h
   /// \param[in] h histogram with data (inclusive distribution of observable, \f$P(X)\f$) to be fitted
   /// \param[in,out] f ROOT function using this implementation of \f$k(c_b)\f$
   void algebraic_fitting_function_binomial::set_initial_parameters(TH1* h, TF1& f)
   {
      f.SetParameter(0, params.NMAX.value = 100);
      f.SetParLimits(0, 50, 150.);
      double xmax = h->GetXaxis()->GetBinUpEdge(h->GetNbinsX());
      double xmin = xmax / 7.;
      f.SetParameter(1, params.Xmax.value = xmin * 2);
      f.SetParLimits(1, xmin, xmax);
      f.SetParameter(2, params.k0.value = 0);
      f.SetParLimits(2, 0, xmin);
      f.SetParameter(3, params.alpha.value = 1);
      f.SetParLimits(3, 0.1, 5);
      f.SetParameter(4, params.gamma.value = 1);
      f.SetParLimits(4, 0.1, 10);
   }

}
