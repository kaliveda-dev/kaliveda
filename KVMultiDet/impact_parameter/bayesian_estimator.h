//Created by KVClassFactory on Fri Jul 26 16:03:15 2019
//Author: John Frankland,,,

#ifndef __bayesian_estimator_H
#define __bayesian_estimator_H

#include "KVBase.h"
#include "impact_parameter_distribution.h"

#include <vector>
#include <numeric>
#include <TF1.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include "TMath.h"
#include "TNamed.h"

#include "Math/PdfFuncMathCore.h"

namespace KVImpactParameters {
   /**
   \class KVImpactParameters::gamma_kernel
   \ingroup ImpactParameters
   \brief Fluctuation kernel using gamma distribution for use with bayesian_estimator

   The distribution of the observable \f$X\f$ is given by the gamma distribution
   \f[
   f(X) = \frac{1}{\Gamma(k)\theta^k} X^{k-1} \exp{(-X/\theta)}
   \f]
   with mean value \f$k\theta\f$ and variance \f$k\theta^2\f$.

   \note For this distribution, the reduced variance \f$\sigma^2/\bar{X}\f$ can take any value \f$>0\f$.

   */
   class gamma_kernel {
   public:
      double operator()(double X, double mean, double reduced_variance)
      {
         // return probability of \f$X\f$ for given mean value \f$\bar{X}\f$ and reduced variance \f$\sigma^2/\bar{X}\f$.
         return ROOT::Math::gamma_pdf(X, mean / reduced_variance, reduced_variance);
      }

      ClassDef(gamma_kernel, 0)
   };

   /**
   \class KVImpactParameters::BD_kernel
   \ingroup ImpactParameters
   \brief Fluctuation kernel using binomial distribution for use with bayesian_estimator

   The distribution of the observable \f$X\f$ is given by the distribution
   \f[
   f(X) = \frac{n!}{X!(n-X)!} p^{X}(1-p)^{n-X}
   \f]
   with mean value \f$np\f$ and variance \f$np(1-p)\f$.

   \note For this distribution, the reduced variance \f$\sigma^2/\bar{X}<1\f$, greater than Poissonian fluctuations are not allowed.

   */
   class BD_kernel {
   public:
      double operator()(double X, double mean, double reduced_variance)
      {
         // return probability of \f$X\f$ for given mean value \f$\bar{X}\f$ and reduced variance \f$\sigma^2/\bar{X}\f$.
         return ROOT::Math::binomial_pdf(TMath::Nint(X), 1. - reduced_variance, mean / (1. - reduced_variance));
      }

      ClassDef(BD_kernel, 0)
   };

   /**
   \class KVImpactParameters::NBD_kernel
   \ingroup ImpactParameters
   \brief Fluctuation kernel using negative binomial distribution for use with bayesian_estimator

   The distribution of the observable \f$X\f$ is given by the distribution
   \f[
   f(X) = \frac{(X+n-1)!}{X!(n-1)!} p^{n}(1-p)^{X}
   \f]
   with mean value \f$(1-p)n/p\f$ and variance \f$(1-p)n/p^2\f$.

   \note For this distribution, the reduced variance \f$\sigma^2/\bar{X}>1\f$, Poissonian fluctuations or smaller are not allowed.

   */
   class NBD_kernel {
   public:
      double operator()(double X, double mean, double reduced_variance)
      {
         // return probability of \f$X\f$ for given mean value \f$\bar{X}\f$ and reduced variance \f$\sigma^2/\bar{X}\f$.
         return ROOT::Math::negative_binomial_pdf(TMath::Nint(X), 1. / reduced_variance, mean / (reduced_variance - 1.));
      }

      ClassDef(NBD_kernel, 0)
   };


   /**
   \class KVImpactParameters::bayesian_estimator
   \ingroup ImpactParameters
   \brief Impact parameter distribution reconstruction from experimental data
   \tparam FittingFunction Class implementing a parameterization of the relationship between mean value of observables as a function of centrality, \f$\bar{X}(c_b)\f$
   \tparam FluctuationKernel Class implementing fluctuations of the observable according to a given PDF

   Implementation of the method of estimating impact parameter distributions for experimental data
   presented in the article Frankland et al., *Phys. Rev.* **Cxx**, yy(2020). The aim is to reproduce the
   inclusive distribution \f$P(X)\f$ of an observable assumed to have a monotonic dependence on impact parameter
   by adjusting the parameters of a fitting function defined by
   \f[
   P(X) = \int P(X|b) P(b) db = \int P(X|c_b) dc_b
   \f]
   where \f$c_b\f$ is the centrality defined by
   \f[
   c_b = \int_0^b P(b) db
   \f]

   \f$P(X|c_b)\f$ is the probability distribution of the observable as a function of the centrality,
   which is given by some FluctuationKernel which is a PDF for any value of \f$X\f$ given the expected
   mean \f$\bar{X}\f$ and reduced variance \f$\sigma^2/\bar{X}\f$.

   The FittingFunction class has to provide the functions
   ~~~~{.cpp}
   double meanX(double c) const
   double redVar(double c) const
   ~~~~
   which describe how \f$\bar{X}\f$ and \f$\sigma^2/\bar{X}\f$. depend on centrality.
   It must also provide the following methods:

   ~~~~~{.cpp}
   FittingFunction::FittingFunction()                               default constructor
   FittingFunction::FittingFunction(const FittingFunction&)         copy constructor, copy parameter values
   constexpr int FittingFunction::npar() const                      number of parameters of function
   void FittingFunction::fill_params_from_array(double*)            takes values of parameters from array
   void FittingFunction::fill_array_from_params(double*) const      puts values of parameters in array
   void FittingFunction::set_par_names(TF1&) const                  set names of parameters in TF1
   void FittingFunction::set_initial_parameters(TH1*,TF1&)          set initial values and limits on parameters to fit histogram
   void FittingFunction::print_fit_params() const                   set initial values and limits on parameters to fit histogram
   void FittingFunction::backup_params() const                      store current values of fit parameters (internally)
   void FittingFunction::restore_params() const                     restore previously backed up values of fit parameters
   void FittingFunction::normalise_shape_function() const           normalize shape parameters
   ~~~~~

   \sa KVImpactParameters::algebraic_fitting_function
   \sa KVImpactParameters::rogly_fitting_function
   \sa KVImpactParameters::gamma_kernel
   \sa KVImpactParameters::BD_kernel
   \sa KVImpactParameters::NBD_kernel

   ## Example of use
   In all of the the following, the namespace prefix `KVImpactParameters::` can be dropped if
   the following instruction is used:
   ~~~~~~~~~{.cpp}
   using namespace KVImpactParameters;
   ~~~~~~~~~
   ### Fitting a given inclusive P(X) distribution

   First initialize the estimator with the required fitting function and kernem, such as for example:
   ~~~~~~~~~{.cpp}
   --/ use 3rd order exponential polynomial function of Rogly et al with gamma kernel:
   KVImpactParameters::bayesian_estimator<KVImpactParameters::rogly_fitting_function<3>,KVImpactParameters::gamma_kernel> ipd;
   --/ or use algebraic function of Frankland et al with a binomial distribution kernel:
   KVImpactParameters::bayesian_estimator<KVImpactParameters::algebraic_fitting_function_binomial, KVImpactParameters::BD_kernel> ipd;
   ~~~~~~~~~
   See rogly_fitting_function, algebraic_fitting_function, gamma_kernel and NBD_kernel for more details.

   Assumng a pointer `TH1* h` to a histogram containing the inclusive distribution for an observable
   filled from data, the first step is to transform the histogram into a correctly normalized probability
   distribution (taking bin widths into account):
   ~~~~~~~~~{.cpp}
   ipd.RenormaliseHisto(h);
   ~~~~~~~~~

   Then to begin the fitting process, call the method
   ~~~~~~~~~{.cpp}
   ipd.FitHisto();
   ~~~~~~~~~
   (if the histogram to fit was already correctly normalised, you can skip the first method and call this one
   giving the histogram pointer as argument). This will prepare initial parameter values for the fit based on
   the histogram data, then perform a first fit attempt. As this first attempt is never successful, you should
   then open the Fit Panel by right-clicking on the histogram in the canvas, and continue fitting using
   "Previous Fit", adjusting the range of the fit, and if necessary the parameter limits, until a satisfactory
   fit is achieved.

   ### Using the results of the fit

   Once a fit has been successful, the bayesian_estimator object can be used to deduce impact parameter
   distributions etc. for the fitted observable. This can be just after performing the fit as in the
   previous section, or using the already known parameters from a previous fit. In the latter case, you
   should initialize the object using the appropriate FittingFunction constructor, e.g. for algebraic_fitting_function
   which requires 5 parameters \f$\alpha,\gamma,\theta,X_{min},X_{max}\f$, you can initialise the
   estimator like so:

   ~~~~~~~~~{.cpp}
   bayesian_estimator<algebraic_fitting_function, gamma_kernel> ipd({alpha,gamma,theta,Xmin,Xmax});

   [if C++11 is not used:
   bayesian_estimator<algebraic_fitting_function, gamma_kernel> ipd(algebraic_fitting_function(alpha,gamma,theta,Xmin,Xmax));
   ]
   ~~~~~~~~~

   See the list of methods below which can then be used in order to exploit the fit result.
   */

   template
   <class FittingFunction, class FluctuationKernel>
   class bayesian_estimator : public KVBase {

      FittingFunction theFitter;
      FluctuationKernel theKernel;

      TH1* histo;
      TH1* h_selection;
      std::vector<double> sel_rapp;
      TF1 p_X_cb_integrator;//
      TF1 P_X_fit_function;//
      TF1 mean_X_vs_cb_function;
      TF1 mean_X_vs_b_function;
      TF1 p_X_X_integrator;
      TF1 p_X_X_integrator_with_selection;
      TF1 fitted_P_X;
      TF1 Cb_dist_for_X_select;
      TF1 B_dist_for_X_select;
      TF1 B_dist_for_arb_X_select;

      Bool_t fIntegerVariable;

      impact_parameter_distribution fIPDist;// used to convert centrality to impact parameter

      double mean_X_vs_cb(double* x, double* par)
      {
         // Function used to fit/draw dependence of mean observable on centrality, \f$\bar{X}(c_b)\f$
         // \param[in] x[0] centrality \f$c_b\f$
         // \param[in] p[0],p[1],...,p[Npar-1] parameters of fitting function
         // \returns mean value of \f$X\f$ for given centrality

         theFitter.fill_params_from_array(par);
         return theFitter.meanX(x[0]);
      }
      double mean_X_vs_b(double* x, double* par)
      {
         // Function used to fit/draw dependence of mean observable on impact parameter, \f$\bar{X}(b)\f$
         // \param[in] x[0] impact parameter \f$b\f$
         // \param[in] p[0],p[1],...,p[Npar-1] parameters of fitting function
         // \returns mean value of \f$X\f$ for given centrality

         theFitter.fill_params_from_array(par);
         return theFitter.meanX(fIPDist.GetCentrality().Eval(x[0]));
      }
      double P_X_cb(double X, double cb)
      {
         // \returns probability of observable \f$X\f$ for centrality \f$c_b\f$, using FluctuationKernel
         // \param[in] X the value of the observable \f$X\f$
         // \param[in] cb the centrality \f$c_b\f$
         return fIntegerVariable ? theKernel(TMath::Nint(X), theFitter.meanX(cb), theFitter.redVar(cb))
                : theKernel(X, theFitter.meanX(cb), theFitter.redVar(cb));
      }
      double P_X_cb_for_integral(double* x, double* par)
      {
         // Function used for the integrand of
         //\f[
         //P(X)=\int P(X|c_b)\,\mathrm{d}c_b
         //\f]
         // \param[in] x[0] centrality \f$c_b\f$
         // \param[in] par[0] observable value, \f$X\f$
         return P_X_cb(par[0], x[0]);
      }
      double P_X_cb_for_X_integral(double* x, double* par)
      {
         // Function used for the integrand of
         //\f[
         //\int P(X|c_b)\,\mathrm{d}X
         //\f]
         // used to calculate centrality distributions for data.
         // \param[in] x[0] observable \f$X\f$
         // \param[in] par[0] centrality value \f$c_b\f$
         return P_X_cb(x[0], par[0]);
      }
      double P_X_cb_for_X_integral_with_selection(double* x, double* par)
      {
         // Function used for the integrand of
         //\f[
         //\int P(X|c_b)\frac{P(X|\mathbb{S})}{P(X)}\,\mathrm{d}X
         //\f]
         // used to calculate centrality distributions for data selection \f$\mathbb{S}\f$
         // \param[in] x[0] observable \f$X\f$
         // \param[in] par[0] centrality value \f$c_b\f$

         int bin = histo->FindBin(x[0]);
         if (bin < 1 || bin > histo->GetNbinsX()) return 0;
         return sel_rapp[bin - 1] * P_X_cb(x[0], par[0]);
      }
      double cb_integrated_P_X(double* x, double* p)
      {
         // Function used to fit experimental \f$P(X)\f$ distribution by integrating the distribution \f$P(X|c_b)\f$
         //\f[
         //P(X)=\int P(X|c_b)\,\mathrm{d}c_b
         //\f]
         // \returns value of \f$P(X)\f$ at \f$X\f$ for current fit parameters
         // \param[in] x[0] \f$X\f$
         // \param[in] p[0],p[1],...,p[Npar-1] parameters of fitting function

         theFitter.fill_params_from_array(p);
         p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
         return p_X_cb_integrator.Integral(0, 1, 1.e-4);
      }
      double P_X_from_fit(double* x, double* par)
      {
         // Same as cb_integrated_P_X(), but just using current values of parameters. Used to display fit result.
         // \param[in] x[0] \f$X\f$
         // \param[in] par[0] normalisation factor
         // \returns value of \f$P(X)\f$ at \f$X\f$ for current fit parameters
         p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
         return par[0] * p_X_cb_integrator.Integral(0, 1, 1.e-4);
      }
      double cb_dist_for_X_selection(double* x, double* p)
      {
         // Function implementing the calculation of the centrality distribution \f$P(c_b|X_{1}<X<X_{2})\f$ for events selected
         // by cuts in \f$X\f$, \f$X_{1}<X<X_{2}\f$
         //\f[
         //P(c_b|X_{1}<X<X_{2})=\frac{1}{c_{X_{1}}-c_{X_{2}}}\int_{X_{1}}^{X_{2}}P(X|c_b)\,\mathrm{d}X
         //\f]
         //
         // \return value of \f$P(c_b|X_{1}<X<X_{2})\f$ for centrality \f$c_b\f$
         // \param[in] x[0] centrality \f$c_b\f$
         // \param[in] p[0],p[1] \f$X_1\f$, \f$X_2\f$

         p_X_X_integrator.SetParameter(0, x[0]);
         double num =  p_X_X_integrator.Integral(p[0], p[1], 1.e-4);
         double den = fitted_P_X.Integral(p[0], p[1], 1.e-4);
         if (den > 0) return num / den;
         return 0;
      }
      double b_dist_for_X_selection(double* x, double* p)
      {
         // Function implementing the calculation of the impact parameter distribution \f$P(b|X_{1}<X<X_{2})\f$ for events selected
         // by cuts in \f$X\f$, \f$X_{1}<X<X_{2}\f$
         //\f[
         //P(b|X_{1}<X<X_{2})=\frac{P(b)}{c_{X_{1}}-c_{X_{2}}}\int_{X_{1}}^{X_{2}}P(X|b)\,\mathrm{d}X
         //\f]
         //
         // \return value of \f$P(b|X_{1}<X<X_{2})\f$ for impact parameter \f$b\f$
         // \param[in] x[0]  impact parameter \f$b\f$
         // \param[in] p[0],p[1] \f$X_1\f$, \f$X_2\f$

         p_X_X_integrator.SetParameter(0, fIPDist.GetCentrality().Eval(x[0]));
         double num =  p_X_X_integrator.Integral(p[0], p[1], 1.e-4);
         return num * fIPDist.GetIPDist().Eval(x[0]);
      }
      double b_dist_for_arb_X_selection(double* x, double* p)
      {
         // Function implementing the calculation of the impact parameter distribution \f$P(b|\mathbb{S})\f$ for an arbitrary selection of events \f$\mathbb{S}\f$
         //\f[
         //P(b|\mathbb{S})=P(b)\frac{\int P(X|b)\frac{P(X|\mathbb{S})}{P(X)}\,\mathrm{d}X}{\int P(X|\mathbb{S})\,\mathrm{d}X}
         //\f]
         //
         // \return value of \f$P(b|\mathbb{S})\f$ for impact parameter \f$b\f$
         // \param[in] x[0]  impact parameter \f$b\f$
         // \param[in] p[0],p[1] integration limits for \f$X\f$, \f$[X_1,X_2]\f$

         p_X_X_integrator_with_selection.SetParameter(0, fIPDist.GetCentrality().Eval(x[0]));
         double num =  p_X_X_integrator_with_selection.Integral(p[0], p[1], 1.e-4);
         return 2 * num * fIPDist.GetIPDist().Eval(x[0]);
      }

   public:
      bayesian_estimator(Bool_t integer_variable = false)
         : KVBase(),
           theFitter(),
           p_X_cb_integrator("p_X_cb_integrator", this, &bayesian_estimator::P_X_cb_for_integral, 0, 1, 1),
           P_X_fit_function("P_X_fit_function", this, &bayesian_estimator::cb_integrated_P_X, 0, 1, theFitter.npar()),
           mean_X_vs_cb_function("mean_X_vs_cb", this, &bayesian_estimator::mean_X_vs_cb, 0, 1, theFitter.npar()),
           mean_X_vs_b_function("mean_X_vs_b", this, &bayesian_estimator::mean_X_vs_b, 0, 20, theFitter.npar()),
           p_X_X_integrator("p_X_X_integrator", this, &bayesian_estimator::P_X_cb_for_X_integral, 0, 1000, 1),
           p_X_X_integrator_with_selection("p_X_X_integrator_with_selection", this, &bayesian_estimator::P_X_cb_for_X_integral_with_selection, 0, 1000, 1),
           fitted_P_X("fitted_P_X", this, &bayesian_estimator::P_X_from_fit, 0, 1000, 1),
           Cb_dist_for_X_select("Cb_dist_for_X_select", this, &bayesian_estimator::cb_dist_for_X_selection, 0, 1, 2),
           B_dist_for_X_select("b_dist_for_X_select", this, &bayesian_estimator::b_dist_for_X_selection, 0, 20, 2),
           B_dist_for_arb_X_select("b_dist_for_arb_X_select", this, &bayesian_estimator::b_dist_for_arb_X_selection, 0, 20, 2),
           fIntegerVariable(integer_variable)
      {
         // Initialise estimator for fitting the inclusive distribution of an observable.
         //
         // \param[in] integer_variable [default:false] set true if the observable to be fitted only takes integer values
         p_X_cb_integrator.SetParNames("X");
         theFitter.set_par_names(P_X_fit_function);
         theFitter.set_par_names(mean_X_vs_cb_function);
         theFitter.set_par_names(mean_X_vs_b_function);
      }

      bayesian_estimator(const FittingFunction& previous_fit, Bool_t integer_variable = false)
         : KVBase(),
           theFitter(previous_fit),
           p_X_cb_integrator("p_X_cb_integrator", this, &bayesian_estimator::P_X_cb_for_integral, 0, 1, 1),
           P_X_fit_function("P_X_fit_function", this, &bayesian_estimator::cb_integrated_P_X, 0, 1, theFitter.npar()),
           mean_X_vs_cb_function("mean_X_vs_cb", this, &bayesian_estimator::mean_X_vs_cb, 0, 1, theFitter.npar()),
           mean_X_vs_b_function("mean_X_vs_b", this, &bayesian_estimator::mean_X_vs_b, 0, 20, theFitter.npar()),
           p_X_X_integrator("p_X_X_integrator", this, &bayesian_estimator::P_X_cb_for_X_integral, 0, 1000, 1),
           p_X_X_integrator_with_selection("p_X_X_integrator_with_selection", this, &bayesian_estimator::P_X_cb_for_X_integral_with_selection, 0, 1000, 1),
           fitted_P_X("fitted_P_X", this, &bayesian_estimator::P_X_from_fit, 0, 1000, 1),
           Cb_dist_for_X_select("Cb_dist_for_X_select", this, &bayesian_estimator::cb_dist_for_X_selection, 0, 1, 2),
           B_dist_for_X_select("b_dist_for_X_select", this, &bayesian_estimator::b_dist_for_X_selection, 0, 20, 2),
           B_dist_for_arb_X_select("b_dist_for_arb_X_select", this, &bayesian_estimator::b_dist_for_arb_X_selection, 0, 20, 2),
           fIntegerVariable(integer_variable)
      {
         // Initialise estimator with results of a previous fit.
         //
         // \param[in] previous_fit parameters of a previous fit
         // \param[in] integer_variable [default:false] set true if the observable to be fitted only takes integer values
         p_X_cb_integrator.SetParNames("X");
         theFitter.set_par_names(P_X_fit_function);
         theFitter.set_par_names(mean_X_vs_cb_function);
         theFitter.set_par_names(mean_X_vs_b_function);
      }

      virtual ~bayesian_estimator() {}

      void RenormaliseHisto(TH1* h)
      {
         // Turn data histogram for observable into probability distribution \f$P(X)\f$ (taking account of binning).
         //
         // After calling this method, the histogram fitting procedure can be started by calling FitHisto().
         //
         // \param[in] h pointer to histogram to normalise

         histo = h;
         histo->Scale(1. / h->Integral("width"));
      }

      void FitHisto(TH1* h = nullptr)
      {
         // Method used to fit a data histogram of \f$P(X)\f$ in order to deduce the parameters of \f$P(X|b)\f$.
         //
         // Prepare initial parameter values for fit based on data in histogram, then perform a first fit attempt.
         // As this first attempt is never successful, you should then open the Fit Panel by right-clicking
         // on the histogram in the canvas, and continue fitting using "Previous Fit", adjusting the range of the
         // fit, and if necessary the parameter limits, until a satisfactory fit is achieved.
         //
         // \note Call RenormaliseHisto() first if histo is not a correctly normalised probability distribution
         //
         // \param[in] h pointer to histogram to fit. if not given, use internal histogram set by previous call to RenormaliseHisto()

         if (h) histo = h;
         P_X_fit_function.SetRange(histo->GetXaxis()->GetBinLowEdge(1), histo->GetXaxis()->GetBinUpEdge(histo->GetNbinsX()));
         theFitter.set_initial_parameters(histo, P_X_fit_function);
         histo->Fit(&P_X_fit_function);
      }

      void SetIPDistParams(double sigmaR, double deltab)
      {
         // Any distributions of \f$b\f$ deduced from data depend on the
         // impact parameter distribution \f$P(b)\f$ which is assumed for the full inclusive dataset,
         // i.e. for all collisions contributing to the \f$P(X)\f$ distribution fitted in order
         // to deduce the parameters for \f$k(c_b)\f$ and \f$\theta\f$.
         //
         // By default this is a triangular distribution between \f$b=0\f$ & \f$b=1\f$ fm with cross-section 31.4 mb,
         // which can be treated as though it were the sharp cut-off distribution for
         // \f$\hat{b}=b/b_{\mathrm{max}}\f$.
         // \todo The differential cross-section in this case is incorrect, although mean values and
         // variaces are calculated correctly
         //
         // To obtain absolute impact parameters, call this method with
         // \param[in] sigmaR total reaction cross-section \f$\sigma_R\f$ in [mb]
         // \param[in] deltab fall-off parameter for distribution \f$\Delta b\f$ in [fm]
         //
         // \sa impact_parameter_distribution

         fIPDist.SetDeltaB_WithConstantCrossSection(deltab, sigmaR);
      }
      impact_parameter_distribution& GetIPDist()
      {
         // Access the impact parameter distribution assumed to correspond to the full inclusive
         // distribution of \f$X\f$, \f$P(X)\f$.
         //
         // \returns reference to impact_parameter_distribution object
         //
         // \sa impact_parameter_distribution
         return fIPDist;
      }

      void DrawMeanXvsCb(const TString& title = "", Color_t color = -1, Option_t* opt = "")
      {
         // Draw the dependence of the mean value of the observable with centrality, \f$\bar{X}(c_b)\f$,
         // given by
         //\f[
         //\bar{X}(c_b)=\theta k(c_b)
         //\f]
         // \param[in] title title to affect to the drawn function
         // \param[in] color colour to use for drawing function
         // \param[in] opt drawing option if required, e.g. "same"

         GetMeanXvsCb().Draw();
      }
      TF1& GetMeanXvsCb()
      {
         // Access the function implementing the dependence of the mean value of the observable with centrality, \f$\bar{X}(c_b)\f$,
         // given by
         //\f[
         //\bar{X}(c_b)=\theta k(c_b)
         //\f]
         // \returns reference to TF1 function object

         Double_t par[theFitter.npar()];
         theFitter.fill_array_from_params(par);
         mean_X_vs_cb_function.SetParameters(par);
         return mean_X_vs_cb_function;
      }
      void DrawMeanXvsb(const TString& title = "", Color_t color = -1, Option_t* opt = "")
      {
         // Draw the dependence of the mean value of the observable with impact parameter, \f$\bar{X}(b)\f$,
         // given by
         //\f[
         //\bar{X}(b)=\theta k(b)
         //\f]
         // \param[in] title title to affect to the drawn function
         // \param[in] color colour to use for drawing function
         // \param[in] opt drawing option if required, e.g. "same"
         //
         // \note To have absolute impact parameters in [fm], call SetIPDistParams() first with the
         // total measured cross-section \f$\sigma_R\f$ in [mb] and the desired fall-off parameter
         // \f$\Delta b\f$ in [fm] (see impact_parameter_distribution).

         Double_t par[theFitter.npar()];
         theFitter.fill_array_from_params(par);
         mean_X_vs_b_function.SetRange(0, GetIPDist().GetB0() + 4 * GetIPDist().GetDeltaB());
         mean_X_vs_b_function.SetParameters(par);
         TF1* copy = mean_X_vs_b_function.DrawCopy(opt);
         if (!title.IsNull()) copy->SetTitle(title);
         if (color >= 0) copy->SetLineColor(color);
      }
      void update_fit_params()
      {
         // Retrieve latest values of fit parameters from fitting function stored with histogram,
         // in case the internal parameters do not correspond to them.
         //
         // \note This method may be obsolete, the problem doesn't seem to occur anymore

         if (!histo) {
            Warning("update_fit_params", "no histogram set with FitHisto(TH1*)");
            return;
         }
         TF1* fit = (TF1*)histo->FindObject("P_X_fit_function");
         if (!fit) {
            Warning("update_fit_params", "no fit function found in histogram");
            return;
         }
         theFitter.fill_params_from_array(fit->GetParameters());
      }
      double DrawCbDistForXSelection(double X1, double X2, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // Draw centrality distribution for observable cuts\f$X_1<X<X_2\f$
         //\f[
         //P(c_b|X_{1}<X<X_{2})=\frac{1}{c_{X_{1}}-c_{X_{2}}}\int_{X_{1}}^{X_{2}}P(X|c_b)\,\mathrm{d}X
         //\f]
         //
         // \param[in] X1 lower limit for observable cut
         // \param[in] X2 upper limit for observable cut
         // \param[in] opt drawing option if required, e.g. "same"
         // \param[in] color colour to use for drawing distribution
         // \param[in] title title to affect to the drawn distribution
         //
         // \returns maximum value of probability distribution
         Cb_dist_for_X_select.SetParameters(X1, X2);
         TF1* f = Cb_dist_for_X_select.DrawCopy(opt);
         f->SetNpx(500);
         f->SetLineColor(color);
         f->SetMarkerColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
         return f->GetMaximum();
      }
      double DrawBDistForXSelection(double X1, double X2, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // Draw impact parameter distribution (as differential cross-section) for observable cuts\f$X_1<X<X_2\f$
         //\f[
         //\frac{\mathrm{d}\sigma}{\mathrm{d}b}=\sigma_{\mathrm{tot}}P(b|X_{1}<X<X_{2})=\frac{P(b)}{c_{X_{1}}-c_{X_{2}}}\int_{X_{1}}^{X_{2}}P(X|b)\,\mathrm{d}X
         //\f]
         //
         // \param[in] X1 lower limit for observable cut
         // \param[in] X2 upper limit for observable cut
         // \param[in] opt drawing option if required, e.g. "same"
         // \param[in] color colour to use for drawing distribution
         // \param[in] title title to affect to the drawn distribution
         //
         // \returns maximum value of differential cross-section
         //
         // \note To have absolute impact parameters in [fm], call SetIPDistParams() first with the
         // total measured cross-section \f$\sigma_R\f$ in [mb] and the desired fall-off parameter
         // \f$\Delta b\f$ in [fm] (see impact_parameter_distribution).


         B_dist_for_X_select.SetParameters(X1, X2);
         //TF1* f = B_dist_for_X_select.DrawCopy(opt);
         TGraph* f = new TGraph;
         double maxS = 0;
         for (int i = 0; i < 500; ++i) {
            double b = 2 * i * GetIPDist().GetB0() / 499.;
            double sig = B_dist_for_X_select.Eval(b);
            if (sig > maxS) maxS = sig;
            f->SetPoint(i, b, sig);
         }
         f->SetLineColor(color);
         f->SetMarkerColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
         if (TString(opt) == "same") f->Draw("c");
         else f->Draw("ac");
         return maxS;
      }
      void DrawBDistForSelection(TH1* sel, TH1* incl, double& mean, double& sigma, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // Draw impact parameter distribution (as differential cross-section) for an arbitrary selection
         // \f$\mathbb{S}\f$ of data,
         //\f[
         //\frac{\mathrm{d}\sigma}{\mathrm{d}b}=\sigma_{\mathrm{tot}}P(b|\mathbb{S})=P(b)\frac{\int P(X|b)\frac{P(X|\mathbb{S})}{P(X)}\,\mathrm{d}X}{\int P(X|\mathbb{S})\,\mathrm{d}X}
         //\f]
         //
         // \param[in] sel pointer to histogram containing observable distribution for selected events
         // \param[in] incl pointer to histogram containing inclusive observable distribution
         // \param[out] mean mean value of b distribution
         // \param[out] sigma standard deviation of b distribution
         // \param[in] opt drawing option if required, e.g. "same"
         // \param[in] color colour to use for drawing distribution
         // \param[in] title title to affect to the drawn distribution
         //
         // \note Histograms sel and incl must have exactly the same axis definitions, binning etc.
         //
         // \note To have absolute impact parameters in [fm], call SetIPDistParams() first with the
         // total measured cross-section \f$\sigma_R\f$ in [mb] and the desired fall-off parameter
         // \f$\Delta b\f$ in [fm] (see impact_parameter_distribution).


         assert(sel->GetNbinsX() == incl->GetNbinsX());

         sel_rapp.assign((std::vector<double>::size_type)incl->GetNbinsX(), 0.0);
         int first_bin(0), last_bin(0);
         for (int i = 1; i <= incl->GetNbinsX(); ++i) {
            if (incl->GetBinContent(i) > 0) {
               sel_rapp[i - 1] = sel->GetBinContent(i) / incl->GetBinContent(i);
               if (sel->GetBinContent(i) > 0) {
                  if (!first_bin) first_bin = i;
                  last_bin = i;
               }
            }
         }
         double Xmin = incl->GetBinLowEdge(first_bin);
         double Xmax = incl->GetXaxis()->GetBinUpEdge(last_bin);

         histo = incl;
         h_selection = sel;

         B_dist_for_arb_X_select.SetParameters(Xmin, Xmax);
         B_dist_for_arb_X_select.SetRange(0, 2 * GetIPDist().GetB0());
         B_dist_for_arb_X_select.GetHistogram();

         double bmean(0), bsqrmean(0), sigtot(0);
         TGraph* f = new TGraph;
         for (int i = 0; i < 500; ++i) {
            double b = 2 * i * GetIPDist().GetB0() / 499.;
            double sig = B_dist_for_arb_X_select.Eval(b);
            bmean += sig * b;
            bsqrmean += sig * b * b;
            sigtot += sig;
            f->SetPoint(i, b, sig);
         }
         f->SetLineColor(color);
         f->SetMarkerColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
         mean = bmean / sigtot;
         bsqrmean /= sigtot;
         sigma = TMath::Sqrt(bsqrmean - mean * mean);
         if (TString(opt) == "same") f->Draw("c");
         else f->Draw("ac");
      }

      void GetMeanAndSigmaBDistForSelection(TH1* sel, TH1* incl, double& mean, double& sigma)
      {
         // Calculate mean and standard deviation of impact parameter distribution for an arbitrary selection
         // \f$\mathbb{S}\f$ of data
         //\f[
         //P(b|\mathbb{S})=P(b)\frac{\int P(X|b)\frac{P(X|\mathbb{S})}{P(X)}\,\mathrm{d}X}{\int P(X|\mathbb{S})\,\mathrm{d}X}
         //\f]
         //
         // \param[in] sel pointer to histogram containing observable distribution for selected events
         // \param[in] incl pointer to histogram containing inclusive observable distribution
         // \param[out] mean mean value of impact parameter for selection, \f$\langle b\rangle\f$
         // \param[out] sigma standard deviation of impact parameter distribution
         //
         // \note Histograms sel and incl must have exactly the same axis definitions, binning etc.
         //
         // \note To have absolute impact parameters in [fm], call SetIPDistParams() first with the
         // total measured cross-section \f$\sigma_R\f$ in [mb] and the desired fall-off parameter
         // \f$\Delta b\f$ in [fm] (see impact_parameter_distribution).


         assert(sel->GetNbinsX() == incl->GetNbinsX());

         sel_rapp.assign((std::vector<double>::size_type)incl->GetNbinsX(), 0.0);
         int first_bin(0), last_bin(0);
         for (int i = 1; i <= incl->GetNbinsX(); ++i) {
            if (incl->GetBinContent(i) > 0) {
               sel_rapp[i - 1] = sel->GetBinContent(i) / incl->GetBinContent(i);
               if (sel->GetBinContent(i) > 0) {
                  if (!first_bin) first_bin = i;
                  last_bin = i;
               }
            }
         }
         double Xmin = incl->GetBinLowEdge(first_bin);
         double Xmax = incl->GetXaxis()->GetBinUpEdge(last_bin);

         histo = incl;
         h_selection = sel;

         B_dist_for_arb_X_select.SetParameters(Xmin, Xmax);
         mean = B_dist_for_arb_X_select.Mean(0, 20);
         double var = B_dist_for_arb_X_select.Variance(0, 20);
         sigma = TMath::Sqrt(var);
      }

      void GetMeanAndSigmaBDistForXSelection(double X1, double X2, double& mean, double& sigma)
      {
         // Calculate mean and standard deviation of impact parameter distribution
         // for observable cuts\f$X_1<X<X_2\f$
         //\f[
         //P(b|X_{1}<X<X_{2})=\frac{P(b)}{c_{X_{1}}-c_{X_{2}}}\int_{X_{1}}^{X_{2}}P(X|b)\,\mathrm{d}X
         //\f]
         //
         // \param[in] X1 lower limit for observable cut
         // \param[in] X2 upper limit for observable cut
         // \param[out] mean mean value of impact parameter for selection, \f$\langle b\rangle\f$
         // \param[out] sigma standard deviation of impact parameter distribution
         //
         // \note To have absolute impact parameters in [fm], call SetIPDistParams() first with the
         // total measured cross-section \f$\sigma_R\f$ in [mb] and the desired fall-off parameter
         // \f$\Delta b\f$ in [fm] (see impact_parameter_distribution).


         B_dist_for_X_select.SetParameters(X1, X2);
         mean = B_dist_for_X_select.Mean(0, 20);
         sigma = TMath::Sqrt(B_dist_for_X_select.Variance(0, 20));
      }
      void DrawFittedP_X(double norm = 1.0, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // Draw the distribution \f$P(X)\f$ resulting from the fit to the experimental distribution
         //
         // \param[in] norm optional normalisation parameter e.g. if differential cross-section required: norm=\f$\sigma_R\f$
         // \param[in] opt drawing option if required, e.g. "same"
         // \param[in] color colour to use for drawing distribution
         // \param[in] title title to affect to the drawn distribution

         TF1* f = GetFittedP_X(norm)->DrawCopy(opt);
         f->SetNpx(500);
         f->SetLineColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
      }
      TF1* GetFittedP_X(double norm = 1.0)
      {
         // Access the distribution \f$P(X)\f$ resulting from the fit to the experimental distribution
         //
         // \param[in] norm optional normalisation parameter e.g. if differential cross-section required: norm=\f$\sigma_R\f$
         // \returns pointer to TF1 function object
         fitted_P_X.SetParameter(0, norm);
         return &fitted_P_X;
      }
      void DrawP_XForGivenB(double b, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // Draw conditional probability distribution of observable,\f$P(X|b)\f$, for a single value of \f$b\f$.
         //
         // \param[in] b impact parameter
         // \param[in] opt drawing option if required, e.g. "same"
         // \param[in] color colour to use for drawing distribution
         // \param[in] title title to affect to the drawn distribution
         p_X_X_integrator.SetParameter(0, fIPDist.GetCentrality().Eval(b));
         TF1* f = p_X_X_integrator.DrawCopy(opt);
         f->SetNpx(500);
         f->SetLineColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
      }
      void Print(Option_t* = "") const
      {
         // Print informations on current values of fit parameters
         theFitter.print_fit_params();
      }
      void DrawNormalisedMeanXvsb(const TString& title, Color_t color, Option_t* opt)
      {
         // Draw the dependence of the mean value of the observable with impact parameter, \f$\bar{X}(b)\f$,
         // given by
         //\f[
         //\bar{X}(b)=\theta k(b)
         //\f]
         // but with \f$\bar{X}\f$ normalised so that \f$X_{\mathrm{min}}=0\f$ and \f$X_{\mathrm{max}}=1\f$,
         // allowing to compare shapes for different fits
         //
         // \param[in] title title to affect to the drawn function
         // \param[in] color colour to use for drawing function
         // \param[in] opt drawing option if required, e.g. "same"
         //
         // \note To have absolute impact parameters in [fm], call SetIPDistParams() first with the
         // total measured cross-section \f$\sigma_R\f$ in [mb] and the desired fall-off parameter
         // \f$\Delta b\f$ in [fm] (see impact_parameter_distribution).


         theFitter.backup_params();
         theFitter.normalise_shape_function();
         Double_t par[5];
         theFitter.fill_array_from_params(par);
         mean_X_vs_b_function.SetRange(0, GetIPDist().GetB0() + 2 * GetIPDist().GetDeltaB());
         mean_X_vs_b_function.SetParameters(par);
         TF1* copy = mean_X_vs_b_function.DrawCopy(opt);
         if (!title.IsNull()) copy->SetTitle(title);
         if (color >= 0) copy->SetLineColor(color);
         theFitter.restore_params();
      }

      ClassDef(bayesian_estimator, 1) //Estimate impact parameter distribution by fits to data
   };

}

#endif
