//Created by KVClassFactory on Fri Jul 26 16:03:15 2019
//Author: John Frankland,,,

#ifndef __bayesian_estimator_H
#define __bayesian_estimator_H

#include "KVBase.h"
#include "KVSmoothIPDist.h"

#include <vector>
#include <numeric>
#include <TF1.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include "TMath.h"
#include "Math/PdfFuncMathCore.h"
#include "TNamed.h"

namespace KVImpactParameters {
   /**
   \class KVImpactParameters::bayesian_estimator
   \ingroup ImpactParameters
   \brief Impact parameter distribution reconstruction from experimental data

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
   and is given by the following gamma distribution:
   \f[
   P(X|c_b) = \frac{1}{\Gamma(k)\theta^k} X^{k-1} \exp{(-X/\theta)}
   \f]
   where \f$\theta\f$ is the ratio between the mean value \f$\bar{X}\f$ and the variance \f$\sigma^2\f$
   of the observable for any given value of centrality \f$c_b\f$. \f$k(c_b)\f$ is a parametrised function
   describing the evolution of \f$\bar{X}\f$ with centrality. This has to be provided by the template
   class parameter FittingFunction, which must provide the following methods:

   ~~~~~{.cpp}
   FittingFunction::FittingFunction()                               default constructor
   FittingFunction::FittingFunction(const FittingFunction&)         copy constructor, copy parameter values
   constexpr int FittingFunction::npar() const                      number of parameters of function (including theta)
   double FittingFunction::k_cb(double cb) const                    returns value of k_cb for given centrality
   void FittingFunction::fill_params_from_array(double*)            takes values of parameters from array
   void FittingFunction::fill_array_from_params(double*) const      puts values of parameters in array
   double FittingFunction::theta() const                            returns value of theta parameter
   void FittingFunction::set_par_names(TF1&) const                  set names of parameters in TF1
   void FittingFunction::set_initial_parameters(TH1*,TF1&)          set initial values and limits on parameters to fit histogram
   void FittingFunction::print_fit_params() const                   set initial values and limits on parameters to fit histogram
   void FittingFunction::backup_params() const                      store current values of fit parameters (internally)
   void FittingFunction::restore_params() const                     restore previously backed up values of fit parameters
   void FittingFunction::normalise_shape_function() const           normalize shape parameters
   ~~~~~

   \sa KVImpactParameters::algebraic_fitting_function
   \sa KVImpactParameters::rogly_fitting_function
   */

   template
   <class FittingFunction>
   class bayesian_estimator : public KVBase {

      FittingFunction theFitter;

      TH1* histo;
      TH1* h_selection;
      std::vector<double> sel_rapp;
      TF1 p_X_cb_integrator;//
      TF1 p_X_b_integrator;//
      TF1 P_X_fit_function;//
      TF1 mean_X_vs_cb_function;
      TF1 mean_X_vs_b_function;
      TF1 mean_b_vs_X_integrator;
      TF1 mean_b_vs_X_function;
      TF1 p_X_X_integrator;
      TF1 p_X_X_integrator_with_selection;
      TF1 fitted_P_X;
      TF1 Cb_dist_for_X_select;
      TF1 B_dist_for_X_select;
      TF1 B_dist_for_arb_X_select;

      KVSmoothIPDist fIPDist;// used to convert centrality to impact parameter

      double mean_X_vs_cb(double* x, double* par)
      {
         // function used to draw fitted <X> vs. cb
         // x[0] = cb
         // p[0]-p[Npar-1] = params of fitting function

         theFitter.fill_params_from_array(par);
         return theFitter.k_cb(x[0]) * theFitter.theta();
      }
      double mean_X_vs_b(double* x, double* par)
      {
         // function used to draw fitted <X> vs. b
         // x[0] = b
         // p[0]-p[Npar-1] = params of fitting function
         // for absolute impact parameter, call SetIPDistParams(sigmaR,deltab) first

         theFitter.fill_params_from_array(par);
         return theFitter.k_cb(fIPDist.GetCentrality().Eval(x[0])) * theFitter.theta();
      }
      double P_X_cb(double X, double cb)
      {
         // probability distribution of observable X for centrality cb
         // is ROOT::Math::gamma_pdf with
         //       alpha = k(cb)
         //       theta = theta
         //       x = X
         return ROOT::Math::gamma_pdf(X, theFitter.k_cb(cb), theFitter.theta());
      }
      double P_X_b(double X, double b)
      {
         // probability distribution of observable X for centrality b (=b/bmax)
         // is ROOT::Math::gamma_pdf with
         //       alpha = k(cb)
         //       theta = theta
         //       x = X
         return ROOT::Math::gamma_pdf(X, theFitter.k_cb(TMath::Sq(b)), theFitter.theta());
      }
      double P_X_cb_for_integral(double* x, double* par)
      {
         // Used to perform the integral int_cb P(X|cb) d(cb) via a TF1
         // x[0] = cb
         // par[0] = X
         return P_X_cb(par[0], x[0]);
      }
      double P_X_b_for_integral(double* x, double* par)
      {
         // Used to perform the integral int_b P(X|b)P(b) d(b) via a TF1
         // x[0] = b  (=b/bmax)
         // par[0] = X
         // we assume triangular distribution P(b)=2b db
         return 2 * x[0] * P_X_b(par[0], x[0]);
      }
      double P_X_cb_for_X_integral(double* x, double* par)
      {
         // distribution of X for given centrality, P(X|cb)
         // used to integrate P(X|cb) wrt X
         // x[0] = X
         // p[0] = cb
         return P_X_cb(x[0], par[0]);
      }
      double P_X_cb_for_X_integral_with_selection(double* x, double* par)
      {
         // distribution of X for given centrality, P(X|cb)
         // used to integrate P(X|cb) wrt X weighted by a distribution of X
         // generated by some selection of data
         // x[0] = X
         // p[0] = cb

         int bin = histo->FindBin(x[0]);
         if (bin < 1 || bin > histo->GetNbinsX()) return 0;
         return sel_rapp[bin - 1] * P_X_cb(x[0], par[0]);
      }
      double cb_integrated_P_X(double* x, double* p)
      {
         // integral used in function to fit experimental P(X) distribution
         // P(X) = int_cb P(X|cb) d(cb)
         // x[0] = X
         // p[0]-p[Npar-1] = parameters of fitting function

         theFitter.fill_params_from_array(p);
         if (theFitter.theta() <= 0) return 0;
         p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
         return p_X_cb_integrator.Integral(0, 1, 1.e-4);
      }
      double b_integrated_P_X(double* x, double* p)
      {
         // integral used in function to fit experimental P(X) distribution
         // P(X) = int_b P(X|b)P(b) d(b)
         // x[0] = X
         // p[0]-p[Npar-1] = parameters of fitting function

         theFitter.fill_params_from_array(p);
         if (theFitter.theta() <= 0) return 0;
         p_X_b_integrator.SetParameter(0, x[0]); // set value of X in integrand
         return p_X_b_integrator.Integral(0, 1, 1.e-4);
      }
      double P_X_from_fit(double* x, double* par)
      {
         // same as previous function, but just using current values of parameters
         // x[0] = X
         // par[0] = normalisation
         p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
         return par[0] * p_X_cb_integrator.Integral(0, 1, 1.e-4);
      }
      double mean_b_vs_X_integrand(double* x, double* p)
      {
         // integrand used to calculate <b> vs. X
         //   i.e. in int_cb[0,1]{ b*P(X|cb) }
         // before using, call SetIPDistParams(sigmaR,deltab)
         // x[0] = cb
         // p[0] = X
         return fIPDist.Calculate_b(x[0]) * P_X_cb_for_integral(x, p);
      }
      double mean_b_vs_X(double* x, double*)
      {
         // function returning mean value of b given X
         // x[0] = X

         double px = P_X_from_fit(x, nullptr);
         if (px > 0) {
            mean_b_vs_X_integrator.SetParameter(0, x[0]);
            return mean_b_vs_X_integrator.Integral(0, 1, 1.e-4) / px;
         }
         return 0;
      }
      double cb_dist_for_X_selection(double* x, double* p)
      {
         // return P(cb|X1<X<X2) calculated from fit
         // x[0] = cb
         // p[0] = X1
         // p[1] = X2

         p_X_X_integrator.SetParameter(0, x[0]);
         double num =  p_X_X_integrator.Integral(p[0], p[1], 1.e-4);
         double den = fitted_P_X.Integral(p[0], p[1], 1.e-4);
         if (den > 0) return num / den;
         return 0;
      }
      double b_dist_for_X_selection(double* x, double* p)
      {
         // return P(b|X1<X<X2) calculated from fit
         // x[0] = b
         // p[0] = X1
         // p[1] = X2

         p_X_X_integrator.SetParameter(0, fIPDist.GetCentrality().Eval(x[0]));
         double num =  p_X_X_integrator.Integral(p[0], p[1], 1.e-4);
         return num * fIPDist.GetIPDist().Eval(x[0]);
      }
      double b_dist_for_arb_X_selection(double* x, double* p)
      {
         // return P(b) for arbitrary distribution of X
         // x[0] = b
         // p[0] = X1
         // p[1] = X2

         p_X_X_integrator_with_selection.SetParameter(0, fIPDist.GetCentrality().Eval(x[0]));
         double num =  p_X_X_integrator_with_selection.Integral(p[0], p[1], 1.e-4);
         return 2 * num * fIPDist.GetIPDist().Eval(x[0]);
      }

   public:
      bayesian_estimator()
         : KVBase(),
           theFitter(),
           p_X_cb_integrator("p_X_cb_integrator", this, &bayesian_estimator::P_X_cb_for_integral, 0, 1, 1),
           p_X_b_integrator("p_X_b_integrator", this, &bayesian_estimator::P_X_b_for_integral, 0, 1, 1),
           P_X_fit_function("P_X_fit_function", this, &bayesian_estimator::cb_integrated_P_X, 0, 1, theFitter.npar()),
           mean_X_vs_cb_function("mean_X_vs_cb", this, &bayesian_estimator::mean_X_vs_cb, 0, 1, theFitter.npar()),
           mean_X_vs_b_function("mean_X_vs_b", this, &bayesian_estimator::mean_X_vs_b, 0, 20, theFitter.npar()),
           mean_b_vs_X_integrator("mean_b_vs_X_integrator", this, &bayesian_estimator::mean_b_vs_X_integrand, 0, 1, 1),
           mean_b_vs_X_function("mean_b_vs_X", this, &bayesian_estimator::mean_b_vs_X, 0, 1, 0),
           p_X_X_integrator("p_X_X_integrator", this, &bayesian_estimator::P_X_cb_for_X_integral, 0, 1000, 1),
           p_X_X_integrator_with_selection("p_X_X_integrator_with_selection", this, &bayesian_estimator::P_X_cb_for_X_integral_with_selection, 0, 1000, 1),
           fitted_P_X("fitted_P_X", this, &bayesian_estimator::P_X_from_fit, 0, 1000, 1),
           Cb_dist_for_X_select("Cb_dist_for_X_select", this, &bayesian_estimator::cb_dist_for_X_selection, 0, 1, 2),
           B_dist_for_X_select("b_dist_for_X_select", this, &bayesian_estimator::b_dist_for_X_selection, 0, 20, 2),
           B_dist_for_arb_X_select("b_dist_for_arb_X_select", this, &bayesian_estimator::b_dist_for_arb_X_selection, 0, 20, 2)
      {
         p_X_cb_integrator.SetParNames("X");
         p_X_b_integrator.SetParNames("X");
         theFitter.set_par_names(P_X_fit_function);
         theFitter.set_par_names(mean_X_vs_cb_function);
         theFitter.set_par_names(mean_X_vs_b_function);
      }

      bayesian_estimator(const FittingFunction& previous_fit)
         : KVBase(),
           theFitter(previous_fit),
           p_X_cb_integrator("p_X_cb_integrator", this, &bayesian_estimator::P_X_cb_for_integral, 0, 1, 1),
           p_X_b_integrator("p_X_b_integrator", this, &bayesian_estimator::P_X_b_for_integral, 0, 1, 1),
           P_X_fit_function("P_X_fit_function", this, &bayesian_estimator::cb_integrated_P_X, 0, 1, theFitter.npar()),
           mean_X_vs_cb_function("mean_X_vs_cb", this, &bayesian_estimator::mean_X_vs_cb, 0, 1, theFitter.npar()),
           mean_X_vs_b_function("mean_X_vs_b", this, &bayesian_estimator::mean_X_vs_b, 0, 20, theFitter.npar()),
           mean_b_vs_X_integrator("mean_b_vs_X_integrator", this, &bayesian_estimator::mean_b_vs_X_integrand, 0, 1, 1),
           mean_b_vs_X_function("mean_b_vs_X", this, &bayesian_estimator::mean_b_vs_X, 0, 1, 0),
           p_X_X_integrator("p_X_X_integrator", this, &bayesian_estimator::P_X_cb_for_X_integral, 0, 1000, 1),
           p_X_X_integrator_with_selection("p_X_X_integrator_with_selection", this, &bayesian_estimator::P_X_cb_for_X_integral_with_selection, 0, 1000, 1),
           fitted_P_X("fitted_P_X", this, &bayesian_estimator::P_X_from_fit, 0, 1000, 1),
           Cb_dist_for_X_select("Cb_dist_for_X_select", this, &bayesian_estimator::cb_dist_for_X_selection, 0, 1, 2),
           B_dist_for_X_select("b_dist_for_X_select", this, &bayesian_estimator::b_dist_for_X_selection, 0, 20, 2),
           B_dist_for_arb_X_select("b_dist_for_arb_X_select", this, &bayesian_estimator::b_dist_for_arb_X_selection, 0, 20, 2)
      {
         p_X_cb_integrator.SetParNames("X");
         p_X_b_integrator.SetParNames("X");
         theFitter.set_par_names(P_X_fit_function);
         theFitter.set_par_names(mean_X_vs_cb_function);
         theFitter.set_par_names(mean_X_vs_b_function);
      }

      virtual ~bayesian_estimator() {}

      void renormalise_histo(TH1* h)
      {
         // Turn data histogram into probability distribution (taking account of binning)

         histo = h;
         histo->Scale(1. / h->Integral("width"));
      }

      void fit_histo(TH1* h = nullptr)
      {
         // Prepare initial values for fit from data in histogram, then fit
         //
         // Call renormalise_histo(h) first if histo is not a correctly normalised probability distribution
         //
         // \param[in] h pointer to histogram to fit. if not given, use internal histogram set by call to renormalise_histo

         if (h) histo = h;
         P_X_fit_function.SetRange(histo->GetXaxis()->GetBinLowEdge(1), histo->GetXaxis()->GetBinUpEdge(histo->GetNbinsX()));
         theFitter.set_initial_parameters(histo, P_X_fit_function);
         histo->Fit(&P_X_fit_function);
         mean_b_vs_X_function.SetRange(histo->GetXaxis()->GetBinLowEdge(1), histo->GetXaxis()->GetBinUpEdge(histo->GetNbinsX()));
      }

      void SetIPDistParams(double sigmaR, double deltab)
      {
         // Set parameters for KVSmoothIPDist used to convert centrality to impact parameter
         // sigmaR is total cross-section in [mb]
         // deltab is width of fall-off in [fm]
         //
         // By default this is a triangular distribution between b=0 & b=1 fm with cross-section 31.4 mb,
         // which can be treated as though it were sharp cut-off distribution for b/bmax

         fIPDist.SetDeltaB_WithConstantCrossSection(deltab, sigmaR);
      }
      KVSmoothIPDist& GetIPDist()
      {
         return fIPDist;
      }

      void DrawMeanXvsCb()
      {
         Double_t par[theFitter.npar()];
         theFitter.fill_array_from_params(par);
         mean_X_vs_cb_function.SetParameters(par);
         mean_X_vs_cb_function.Draw();
      }
      TF1& GetMeanXvsCb()
      {
         Double_t par[theFitter.npar()];
         theFitter.fill_array_from_params(par);
         mean_X_vs_cb_function.SetParameters(par);
         return mean_X_vs_cb_function;
      }
      void DrawMeanXvsb(const TString& title = "", Color_t color = -1, Option_t* opt = "")
      {
         Double_t par[theFitter.npar()];
         theFitter.fill_array_from_params(par);
         mean_X_vs_b_function.SetRange(0, GetIPDist().GetB0() + 2 * GetIPDist().GetDeltaB());
         mean_X_vs_b_function.SetParameters(par);
         TF1* copy = mean_X_vs_b_function.DrawCopy(opt);
         if (!title.IsNull()) copy->SetTitle(title);
         if (color >= 0) copy->SetLineColor(color);
      }
      TGraph* GetMeanbvsX(int npoints = 101)
      {
         // generate a graph of <b> vs X
         double xmin, xmax;
         mean_b_vs_X_function.GetRange(xmin, xmax);
         double dx = (xmax - xmin) / (npoints - 1.);
         double x = xmin;
         TGraph* g = ::new TGraph(100);
         for (int i = 0; i < 100; ++i) {
            g->SetPoint(i, x, mean_b_vs_X_function.Eval(x));
            x += dx;
         }
         return g;
      }
      void update_fit_params()
      {
         // get latest values of parameters from histogram fit

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
      void DrawCbDistForXSelection(double X1, double X2, Option_t* opt = "")
      {
         Cb_dist_for_X_select.SetParameters(X1, X2);
         Cb_dist_for_X_select.Draw(opt);
      }
      double DrawBDistForXSelection(double X1, double X2, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // returns maximum of distribution
         B_dist_for_X_select.SetParameters(X1, X2);
         TF1* f = B_dist_for_X_select.DrawCopy(opt);
         f->SetNpx(500);
         f->SetLineColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
         return f->GetMaximum();
      }
      void DrawBDistForSelection(TH1* sel, TH1* incl, Option_t* opt = "", Color_t color = kRed, const TString& title = "")
      {
         // sel = histo with X distribution generated by some selection
         // incl = histo with full inclusive X distribution (normally the one fitted with P(X))

         assert(sel->GetNbinsX() == incl->GetNbinsX());

         //calculate ratios P_sel(X)/P(X)
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
         //B_dist_for_arb_X_select.SetRange(0, GetIPDist().GetB0() + 2 * GetIPDist().GetDeltaB());
         B_dist_for_arb_X_select.SetRange(0, 2 * GetIPDist().GetB0());
         B_dist_for_arb_X_select.GetHistogram();
         TF1* f =  B_dist_for_arb_X_select.DrawCopy(opt);
         f->SetNpx(500);
         f->SetLineColor(color);
         f->SetLineWidth(2);
         f->SetTitle(title);
      }

      void GetMeanAndSigmaBDistForSelection(TH1* sel, TH1* incl, double& mean, double& sigma)
      {
         // sel = histo with X distribution generated by some selection
         // incl = histo with full inclusive X distribution (normally the one fitted with P(X))

         assert(sel->GetNbinsX() == incl->GetNbinsX());

         //calculate ratios P_sel(X)/P(X)
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
         double Xmax = incl->GetBinCenter(last_bin) + 0.5 * incl->GetBinWidth(last_bin);

         histo = incl;
         h_selection = sel;

         B_dist_for_arb_X_select.SetParameters(Xmin, Xmax);
         mean = B_dist_for_arb_X_select.Mean(0, 20);
         sigma = TMath::Sqrt(B_dist_for_arb_X_select.Variance(0, 20));
      }

      void GetMeanAndSigmaBDistForXSelection(double X1, double X2, double& mean, double& sigma)
      {
         B_dist_for_X_select.SetParameters(X1, X2);
         mean = B_dist_for_X_select.Mean(0, 20);
         sigma = TMath::Sqrt(B_dist_for_X_select.Variance(0, 20));
      }
      void DrawFittedP_X(double norm = 1.0)
      {
         GetFittedP_X(norm)->Draw();
      }
      TF1* GetFittedP_X(double norm = 1.0)
      {
         fitted_P_X.SetParameter(0, norm);
         return &fitted_P_X;
      }
      void DrawP_XForGivenB(double b)
      {

         p_X_X_integrator.SetParameter(0, fIPDist.GetCentrality().Eval(b));
         p_X_X_integrator.Draw();
      }
      void Print(Option_t* option = "") const
      {
         theFitter.print_fit_params();
      }
      void DrawNormalisedMeanXvsb(const TString& title, Color_t color, Option_t* opt)
      {
         // Draw mean X vs b with Xmin=0 and Xmax=1, allowing to compare shapes for different fits
         //
         //  "title" : set title of drawn graph (for figure legend)
         //  "color" : set color of graph
         //  "opt"   : drawing option (i.e. "same")

         // save parameters
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
