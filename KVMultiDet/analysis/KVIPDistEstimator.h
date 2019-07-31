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
   int J; // degree of polynomial k(cb) Eq(8)
   std::vector<fit_parameter> aj; // polynomial coefficients Eq(8)
   fit_parameter theta; // the (b-independent) fluctuation width Eqs(1),(2)
   fit_parameter kmax; // value of k at b=0 Eq(8)
   fit_parameter Xmax; // value of X at b=0
   double chisquare;
   double ndf;

   bce_fit_results(int j) : J(j), aj(j) {}
   virtual ~bce_fit_results() {}

   void Print(Option_t* = "") const
   {
      cout << "Chi-2 = " << chisquare << " NDF = " << ndf << " (" << chisquare / ndf << ")" << endl;
      cout << "   kmax = ";
      kmax.print();
      cout << "   theta = ";
      theta.print();
      cout << endl;
      for (int i = 0; i < J; ++i) {
         cout << "\t a" << i << " = ";
         aj[i].print();
         cout << endl;
      }
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

   KVSmoothIPDist fIPDist;// used to convert centrality to impact parameter

   double k_cb(double cb)
   {
      // k as a function of centrality cb
      double arg = 0;
      for (int j = 0; j < params.J; ++j) arg += params.aj[j].value * TMath::Power(cb, j + 1);
      return params.kmax.value * TMath::Exp(-arg);
   }
   double mean_X_vs_cb(double* x, double*)
   {
      // function used to draw fitted <X> vs. cb
      // x[0] = cb

      update_fit_params();
      return k_cb(x[0]) * params.theta.value;
   }
   double mean_X_vs_b(double* x, double*)
   {
      // function used to draw fitted <X> vs. b
      // x[0] = b
      // before using, call SetIPDistParams(sigmaR,deltab)

      update_fit_params();
      return k_cb(fIPDist.GetCentrality().Eval(x[0])) * params.theta.value;
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
   double cb_integrated_P_X(double* x, double* p)
   {
      // integral used in function to fit experimental P(X) distribution
      // P(X) = int_cb P(X|cb) d(cb)
      // x[0] = X
      // p[0] = theta
      // p[1] = Xmax
      // p[2, 3, ]... = aj
      // in total, (2+J) parameters

      params.theta.value = p[0];
      params.Xmax.value = p[1];
      params.kmax.value = params.Xmax.value / params.theta.value;
      for (int j = 0; j < params.J; ++j) params.aj[j].value = p[2 + j];
      p_X_cb_integrator.SetParameter(0, x[0]); // set value of X in integrand
      return p_X_cb_integrator.Integral(0, 1);
   }
   double P_X_from_fit(double X)
   {
      // same as previous function, but just using current values of parameters
      p_X_cb_integrator.SetParameter(0, X); // set value of X in integrand
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

      double px = P_X_from_fit(x[0]);
      if (px) {
         mean_b_vs_X_integrator.SetParameter(0, x[0]);
         return mean_b_vs_X_integrator.Integral(0, 1) / px;
      }
      return 0;
   }

public:
   KVIPDistEstimator(int J);
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

   ClassDef(KVIPDistEstimator, 1) //Estimate impact parameter distribution by fits to data
};

#endif
