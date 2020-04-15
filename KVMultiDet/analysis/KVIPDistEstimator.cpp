//Created by KVClassFactory on Fri Jul 26 16:03:15 2019
//Author: John Frankland,,,

#include "KVIPDistEstimator.h"
#include <cassert>

ClassImp(KVIPDistEstimator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIPDistEstimator</h2>
<h4>Estimate impact parameter distribution by fits to data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIPDistEstimator::KVIPDistEstimator()
   : KVBase(),
     params(),
     p_X_cb_integrator("p_X_cb_integrator", this, &KVIPDistEstimator::P_X_cb_for_integral, 0, 1, 1),
     P_X_fit_function("P_X_fit_function", this, &KVIPDistEstimator::cb_integrated_P_X, 0, 1, 5),
     mean_X_vs_cb_function("mean_X_vs_cb", this, &KVIPDistEstimator::mean_X_vs_cb, 0, 1, 0),
     mean_X_vs_b_function("mean_X_vs_b", this, &KVIPDistEstimator::mean_X_vs_b, 0, 20, 0),
     mean_b_vs_X_integrator("mean_b_vs_X_integrator", this, &KVIPDistEstimator::mean_b_vs_X_integrand, 0, 1, 1),
     mean_b_vs_X_function("mean_b_vs_X", this, &KVIPDistEstimator::mean_b_vs_X, 0, 1, 0),
     p_X_X_integrator("p_X_X_integrator", this, &KVIPDistEstimator::P_X_cb_for_X_integral, 0, 1000, 1),
     p_X_X_integrator_with_selection("p_X_X_integrator_with_selection", this, &KVIPDistEstimator::P_X_cb_for_X_integral_with_selection, 0, 1000, 1),
     fitted_P_X("fitted_P_X", this, &KVIPDistEstimator::P_X_from_fit, 0, 1000, 1),
     Cb_dist_for_X_select("Cb_dist_for_X_select", this, &KVIPDistEstimator::cb_dist_for_X_selection, 0, 1, 2),
     B_dist_for_X_select("b_dist_for_X_select", this, &KVIPDistEstimator::b_dist_for_X_selection, 0, 20, 2),
     B_dist_for_arb_X_select("b_dist_for_arb_X_select", this, &KVIPDistEstimator::b_dist_for_arb_X_selection, 0, 20, 2)
{
   p_X_cb_integrator.SetParNames("X");
   P_X_fit_function.SetParNames("#theta", "X_{max}", "X_{min}", "#alpha", "#gamma");
}

KVIPDistEstimator::KVIPDistEstimator(double ALPHA, double GAMMA, double THETA, double XMIN, double XMAX, double sigmaR, double deltaB)
   : KVBase(),
     params(ALPHA, GAMMA, THETA, XMIN, XMAX),
     p_X_cb_integrator("p_X_cb_integrator", this, &KVIPDistEstimator::P_X_cb_for_integral, 0, 1, 1),
     P_X_fit_function("P_X_fit_function", this, &KVIPDistEstimator::cb_integrated_P_X, 0, 1, 5),
     mean_X_vs_cb_function("mean_X_vs_cb", this, &KVIPDistEstimator::mean_X_vs_cb, 0, 1, 0),
     mean_X_vs_b_function("mean_X_vs_b", this, &KVIPDistEstimator::mean_X_vs_b, 0, 20, 0),
     mean_b_vs_X_integrator("mean_b_vs_X_integrator", this, &KVIPDistEstimator::mean_b_vs_X_integrand, 0, 1, 1),
     mean_b_vs_X_function("mean_b_vs_X", this, &KVIPDistEstimator::mean_b_vs_X, 0, 1, 0),
     p_X_X_integrator("p_X_X_integrator", this, &KVIPDistEstimator::P_X_cb_for_X_integral, 0, 1000, 1),
     p_X_X_integrator_with_selection("p_X_X_integrator_with_selection", this, &KVIPDistEstimator::P_X_cb_for_X_integral_with_selection, 0, 1000, 1),
     fitted_P_X("fitted_P_X", this, &KVIPDistEstimator::P_X_from_fit, 0, 1000, 1),
     Cb_dist_for_X_select("Cb_dist_for_X_select", this, &KVIPDistEstimator::cb_dist_for_X_selection, 0, 1, 2),
     B_dist_for_X_select("b_dist_for_X_select", this, &KVIPDistEstimator::b_dist_for_X_selection, 0, 20, 2),
     B_dist_for_arb_X_select("b_dist_for_arb_X_select", this, &KVIPDistEstimator::b_dist_for_arb_X_selection, 0, 20, 2)
{
   p_X_cb_integrator.SetParNames("X");
   P_X_fit_function.SetParNames("#theta", "X_{max}", "X_{min}", "#alpha", "#gamma");
   SetIPDistParams(sigmaR, deltaB);
}

//____________________________________________________________________________//

KVIPDistEstimator::~KVIPDistEstimator()
{
   // Destructor
}

void KVIPDistEstimator::FitHisto(TH1* h)
{
   // Normalise histogram if not already done, then fit it
   histo = h;
   if (h->Integral() != 1.0) {
      h->Sumw2();
      h->Scale(1. / h->Integral("width"));
      h->SetMarkerStyle(24);
      h->SetMarkerSize(1.5);
   }

   P_X_fit_function.SetRange(h->GetXaxis()->GetBinLowEdge(1), h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
   P_X_fit_function.SetParameter(0, params.theta.value = 0.5);
   P_X_fit_function.SetParLimits(0, 0.1, 100.);
   double xmax = h->GetXaxis()->GetBinUpEdge(h->GetNbinsX());
   double xmin = xmax / 7.;
   P_X_fit_function.SetParameter(1, params.Xmax.value = xmin * 2);
   P_X_fit_function.SetParLimits(1, xmin, xmax);
   P_X_fit_function.SetParameter(2, params.k0.value = 0);
   P_X_fit_function.SetParLimits(2, 0, xmin);
   P_X_fit_function.SetParameter(3, params.alpha.value = 1);
   P_X_fit_function.SetParLimits(3, 0.1, 5);
   P_X_fit_function.SetParameter(4, params.gamma.value = 1);
   P_X_fit_function.SetParLimits(4, 0.1, 10);
   h->Fit(&P_X_fit_function);

   mean_b_vs_X_function.SetRange(h->GetXaxis()->GetBinLowEdge(1), h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
}

void KVIPDistEstimator::update_fit_params()
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
   double theta = fit->GetParameter(0);
   double Xmax = fit->GetParameter(1);
   double Xmin = fit->GetParameter(2);
   double alpha = fit->GetParameter(3);
   double gamma = fit->GetParameter(4);
   params.theta.value = theta;
   params.Xmax.value = Xmax;
   params.kmax.value = (Xmax - Xmin) / theta;
   params.k0.value = Xmin / theta;
   params.alpha.value = alpha;
   params.gamma.value = gamma;
}

void KVIPDistEstimator::DrawBDistForSelection(TH1* sel, TH1* incl, Option_t* opt)
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
   B_dist_for_arb_X_select.DrawCopy(opt);
   std::cout << B_dist_for_arb_X_select.Mean(0, 20) << "+/-" << TMath::Sqrt(B_dist_for_arb_X_select.Variance(0, 20)) << std::endl;
}

void KVIPDistEstimator::GetMeanAndSigmaBDistForSelection(TH1* sel, TH1* incl, double& mean, double& sigma)
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

