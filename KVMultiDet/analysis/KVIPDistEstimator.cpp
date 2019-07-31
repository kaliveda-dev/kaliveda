//Created by KVClassFactory on Fri Jul 26 16:03:15 2019
//Author: John Frankland,,,

#include "KVIPDistEstimator.h"

ClassImp(KVIPDistEstimator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIPDistEstimator</h2>
<h4>Estimate impact parameter distribution by fits to data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIPDistEstimator::KVIPDistEstimator(int J)
   : KVBase(),
     params(J),
     p_X_cb_integrator("p_X_cb_integrator", this, &KVIPDistEstimator::P_X_cb_for_integral, 0, 1, 1),
     P_X_fit_function("P_X_fit_function", this, &KVIPDistEstimator::cb_integrated_P_X, 0, 1, 2 + J),
     mean_X_vs_cb_function("mean_X_vs_cb", this, &KVIPDistEstimator::mean_X_vs_cb, 0, 1, 0),
     mean_X_vs_b_function("mean_X_vs_b", this, &KVIPDistEstimator::mean_X_vs_b, 0, 20, 0),
     mean_b_vs_X_integrator("mean_b_vs_X_integrator", this, &KVIPDistEstimator::mean_b_vs_X_integrand, 0, 1, 1),
     mean_b_vs_X_function("mean_b_vs_X", this, &KVIPDistEstimator::mean_b_vs_X, 0, 1, 0)
{
   // J is the order of the polynomial used in the parametrisation of k(cb)

   p_X_cb_integrator.SetParNames("X");
   P_X_fit_function.SetParNames("#theta", "X_{max}");
   for (int j = 0; j < J; ++j) P_X_fit_function.SetParName(2 + j, Form("a_{%d}", j + 1));
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
   }

   P_X_fit_function.SetRange(h->GetXaxis()->GetBinLowEdge(1), h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
   P_X_fit_function.SetParameter(0, params.theta.value = 0.5);
   P_X_fit_function.SetParLimits(0, 0.1, 100.);
   P_X_fit_function.SetParameter(1, params.Xmax.value = 2.5 * h->GetMean());
   P_X_fit_function.SetParLimits(1, 0.1, 10 * params.Xmax.value);
   for (int j = 0; j < params.J; ++j) {
      P_X_fit_function.SetParameter(2 + j, params.aj[j].value = pow(10., -j) * 1.5);
      P_X_fit_function.SetParLimits(2 + j, 0., 100.);
   }
   h->Fit(&P_X_fit_function);

   mean_b_vs_X_function.SetRange(h->GetXaxis()->GetBinLowEdge(1), h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
}

void KVIPDistEstimator::update_fit_params()
{
   // get latest values of parameters from histogram fit

   params.theta.value = P_X_fit_function.GetParameter(0);
   params.Xmax.value = P_X_fit_function.GetParameter(1);
   params.kmax.value = params.Xmax.value / params.theta.value;
   for (int j = 0; j < params.J; ++j) {
      params.aj[j].value = P_X_fit_function.GetParameter(2 + j);
   }
}

//____________________________________________________________________________//

