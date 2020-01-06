//Created by KVClassFactory on Fri Jul 26 16:03:15 2019
//Author: John Frankland,,,

#ifndef __KVIPDISTESTIMATOR_H
#define __KVIPDISTESTIMATOR_H

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

using namespace std;

struct fit_parameter {
   double value;
   double error;
   fit_parameter() : value(0), error(0) {}
   void print() const
   {
      cout << value << "+/-" << error;
   }
   void set(TF1* f, int npar)
   {
      value = f->GetParameter(npar);
      error = f->GetParError(npar);
   }
};

class bce_fit_results : public TNamed {
public:
   fit_parameter k0; // minimum of k at c=1
   fit_parameter alpha; // power of centrality
   fit_parameter gamma; // power of (1-pow(c,alpha))
   fit_parameter theta; // the (b-independent) fluctuation width Eqs(1),(2)
   fit_parameter kmax; // value of k at b=0 Eq(8)
   fit_parameter Xmax; // value of X at b=0
   double chisquare;
   double ndf;

   bce_fit_results() {}
   bce_fit_results(double ALPHA, double GAMMA, double THETA, double XMIN, double XMAX)
   {
      alpha.value = ALPHA;
      gamma.value = GAMMA;
      theta.value = THETA;
      k0.value = XMIN / THETA;
      Xmax.value = XMAX;
      kmax.value = (XMAX - XMIN) / THETA;
   }
   virtual ~bce_fit_results() {}

   void Print(Option_t* = "") const
   {
      cout << "Chi-2 = " << chisquare << " NDF = " << ndf << " (" << chisquare / ndf << ")" << endl;
      cout << "   kmax = ";
      kmax.print();
      cout << "   k0 = ";
      k0.print();
      cout << "   alpha = ";
      alpha.print();
      cout << "   gamma = ";
      gamma.print();
      cout << "   theta = ";
      theta.print();
      cout << endl;

   }

   ClassDef(bce_fit_results, 1)
};

class KVIPDistEstimator : public KVBase {
   bce_fit_results params;
   TH1* histo;
   TF1 p_X_cb_integrator;//
   TF1 P_X_fit_function;//
   TF1 mean_X_vs_cb_function;
   TF1 mean_X_vs_b_function;
   TF1 mean_b_vs_X_integrator;
   TF1 mean_b_vs_X_function;
   TF1 p_X_X_integrator;
   TF1 fitted_P_X;
   TF1 Cb_dist_for_X_select;
   TF1 B_dist_for_X_select;

   KVSmoothIPDist fIPDist;// used to convert centrality to impact parameter

   double k_cb(double cb)
   {
      // k as a function of centrality cb

      if (params.alpha.value <= 0) return 0;
      if (params.gamma.value <= 0) return 0;
      if (cb < 0 || cb > 1) return 0;
      double arg = 1. - TMath::Power(cb, params.alpha.value);
      if (arg < 0) return 0;
      return params.kmax.value * TMath::Power(arg, params.gamma.value)
             + params.k0.value;
   }
   double mean_X_vs_cb(double* x, double*)
   {
      // function used to draw fitted <X> vs. cb
      // x[0] = cb

      return k_cb(x[0]) * params.theta.value;
   }
   double mean_X_vs_b(double* x, double*)
   {
      // function used to draw fitted <X> vs. b
      // x[0] = b
      // before using, call SetIPDistParams(sigmaR,deltab)
      if (!TMath::IsNaN(fIPDist.GetCentrality().Eval(x[0])))
         return k_cb(fIPDist.GetCentrality().Eval(x[0])) * params.theta.value;
      return 0;
   }
   double P_X_cb(double X, double cb)
   {
      // probability distribution of observable X for centrality cb
      // is ROOT::Math::gamma_pdf with
      //       alpha = k(cb)
      //       theta = theta
      //       x = X
      return ROOT::Math::gamma_pdf(X, k_cb(cb), params.theta.value);
   }
   double P_X_cb_for_integral(double* x, double* par)
   {
      // Used to perform the integral int_cb P(X|cb) d(cb) via a TF1
      // x[0] = cb
      // par[0] = X
      return P_X_cb(par[0], x[0]);
   }
   double P_X_cb_for_X_integral(double* x, double* par)
   {
      // distribution of X for given centrality, P(X|cb)
      // used to integrate P(X|cb) wrt X
      // x[0] = X
      // p[0] = cb
      // p[1] = dsig/db relative to 2pib
      return par[1] * P_X_cb(x[0], par[0]);
   }
   double cb_integrated_P_X(double* x, double* p)
   {
      // integral used in function to fit experimental P(X) distribution
      // P(X) = int_cb P(X|cb) d(cb)
      // x[0] = X
      // p[0] = theta
      // p[1] = Xmax
      // p[2] = Xmin
      // p[3] = alpha
      // p[4] = gamma
      // in total, 5 parameters

      params.theta.value = p[0];
      params.Xmax.value = p[1] - p[2];
      params.kmax.value = params.Xmax.value / params.theta.value;
      params.k0.value = p[2] / params.theta.value;
      params.alpha.value = p[3];
      params.gamma.value = p[4];
      p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
      return p_X_cb_integrator.Integral(0, 1);
   }
   double P_X_from_fit(double* x, double*)
   {
      // same as previous function, but just using current values of parameters
      // x[0] = X
      p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
      return p_X_cb_integrator.Integral(0, 1);
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
         return mean_b_vs_X_integrator.Integral(0, 1) / px;
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
      double num =  p_X_X_integrator.Integral(p[0], p[1]);
      double den = fitted_P_X.Integral(p[0], p[1]);
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
      p_X_X_integrator.SetParameter(1, fIPDist.GetRelativeCrossSection(x[0]));
      double num =  p_X_X_integrator.Integral(p[0], p[1], 1.e-1);
      return num * fIPDist.GetIPDist().Eval(x[0]);
   }

public:
   KVIPDistEstimator();
   KVIPDistEstimator(double ALPHA, double GAMMA, double THETA, double XMIN, double XMAX,
                     double sigmaR, double deltaB);
   virtual ~KVIPDistEstimator();

   const bce_fit_results& GetFitResults() const
   {
      return params;
   }

   void FitHisto(TH1* h);

   void SetIPDistParams(double sigmaR, double deltab)
   {
      // Set parameters for KVSmoothIPDist used to convert centrality to impact parameter
      // sigmaR is total cross-section in [mb]
      // deltab is width of fall-off in [fm]

      fIPDist.SetDeltaB_WithConstantCrossSection(deltab, sigmaR);
   }

   void DrawMeanXvsCb()
   {
      mean_X_vs_cb_function.Draw();
   }
   void DrawMeanXvsb()
   {
      mean_X_vs_b_function.Draw();
   }
   TGraph* GetMeanbvsX(int npoints = 101)
   {
      // generate a graph of <b> vs X
      double xmin, xmax;
      mean_b_vs_X_function.GetRange(xmin, xmax);
      double dx = (xmax - xmin) / (npoints - 1.);
      double x = xmin;
      TGraph* g = new TGraph(100);
      for (int i = 0; i < 100; ++i) {
         g->SetPoint(i, x, mean_b_vs_X_function.Eval(x));
         x += dx;
      }
      return g;
   }
   void update_fit_params();
   void DrawCbDistForXSelection(double X1, double X2, Option_t* opt = "")
   {
      Cb_dist_for_X_select.SetParameters(X1, X2);
      Cb_dist_for_X_select.Draw(opt);
   }
   void DrawBDistForXSelection(double X1, double X2, Option_t* opt = "")
   {
      B_dist_for_X_select.SetParameters(X1, X2);
      B_dist_for_X_select.Draw(opt);
      std::cout << B_dist_for_X_select.Mean(0, 20) << "+/-" << B_dist_for_X_select.Variance(0, 20) << std::endl;
   }
   void DrawFittedP_X()
   {
      fitted_P_X.Draw();
   }
   void DrawP_XForGivenB(double b)
   {

      p_X_X_integrator.SetParameter(0, fIPDist.GetCentrality().Eval(b));
      p_X_X_integrator.SetParameter(1, fIPDist.GetRelativeCrossSection(b));
      p_X_X_integrator.Draw();
   }

   ClassDef(KVIPDistEstimator, 1) //Estimate impact parameter distribution by fits to data
};

#endif
