//Created by KVClassFactory on Fri Jan 15 18:14:06 2010
//Author: John Frankland,,,

#include "KVImpactParameter.h"

ClassImp(KVImpactParameter)

////////////////////////////////////////////////////////////////////////////////
/*
BEGIN_HTML
<h2>KVImpactParameter</h2>
<h4>Impact parameter analysis tools</h4>
Use a histogram containing the distribution of some observable strongly-correlated with
the impact parameter in order to transform distributions of said observable into
impact parameter distributions (using the method of C. Cavata <I>et al.</I>, <I>Phys. Rev.</I> <B>C42</B>, 1760 (1990)), and to calculate the evolution of other quantities as a function
of the impact parameter.<br>
To use, start with a pointer to a 1-D histogram of the observable, TH1* data:<br>
<pre>
<code>
KVImpactParameter ip(data);
ip.MakeScale(npoints, bmax);
</code>
</pre>
The <a href="#KVImpactParameter:MakeScale">MakeScale</a> method calculates the relationship between the observable and the
impact parameter, using
END_HTML
BEGIN_LATEX
\hat{b}^{2} = \frac{\int^{\infty}_{x=X} Y(x) dx}{\int_{0}^{\infty} Y(x) dx}
END_LATEX
BEGIN_HTML
To obtain the impact parameter distribution for some selection of events,
you need the distribution of the observable for the selection, TH1* obs_sel,
and then use:<br>
<pre>
<code>
TH1* ip_dist_sel = ip.GetIPDistribution(obs_sel);
</code>
</pre>
To obtain the impact-parameter evolution of some quantity, take a TH2* obscor, containing
the bidimensional plot of the quantity as a function of the observable, and then use<br>
<pre>
<code>
TGraph* ip_evol = ip.GetIPEvolution(obscor, "GetMean");
<code>
</pre>
e.g. to have the mean value of the quantity as a function of impact parameter.
END_HTML
*/
////////////////////////////////////////////////////////////////////////////////

KVImpactParameter::KVImpactParameter(TH1* data, Option_t* evol)
{
   // Default constructor
   // Argument 'data' is pointer to data histogram containing distribution of the observable
   // which is used to calculate the impact parameter. Usually, this will be an observable
   // which is supposed to increase or decrease monotonically as a function of b.
   // By default, evol = "D" which means observable increases as b decreases.
   // Call with evol = "C" if the observable increases as b increases.
   fData = data;
   fEvol = evol;
   fIPScale = 0;
   fObsTransform = 0;
   Bmax = 1.0;
}

KVImpactParameter::~KVImpactParameter()
{
   // Destructor
   SafeDelete(fIPScale);
   SafeDelete(fObsTransform);
}

void KVImpactParameter::MakeScale(Int_t npoints, Double_t bmax)
{
   // Calculate the relationship between the impact parameter and the observable
   // whose distribution is contained in the histogram fData.
   // For a given value X of the observable x, the reduced impact parameter
   // b_hat is calculated from the distribution of x, Y(x), using the following formula:
   /*
   BEGIN_LATEX
   \hat{b}^{2} = \frac{\int^{\infty}_{x=X} Y(x) dx}{\int_{0}^{\infty} Y(x) dx}
   END_LATEX
   */
   // npoints = number of points for which to calculate the impact parameter.
   // The greater the number of points, the more accurate the results.
   // Default value is 100. Maximum value is number of bins in histogram of observable, fData.
   // bmax is the maximum reduced impact parameter for the data.
   // To obtain absolute values of impact parameter/cross-section,
   // use MakeAbsoluteScale.

   if (bmax > 1.0) {
      Warning("MakeScale", "called with bmax>1.0 - calling MakeAbsoluteScale for absolute impact parameters");
      MakeAbsoluteScale(npoints, bmax);
      return;
   }
   Bmax = bmax;
   Smax = pow(bmax, 2.); //total reduced X-section
   make_scale(npoints);
}

void KVImpactParameter::make_scale(Int_t npoints)
{
   TH1* cumul = HM.CumulatedHisto(fData, fEvol.Data(), -1, -1, "max");
   Int_t nbins = cumul->GetNbinsX();
   Int_t first_bin = 1;
   Int_t last_bin = nbins;
   npoints = TMath::Min(nbins, npoints);
   fIPScale = new TGraph(npoints);
   fXSecScale = new TGraph(npoints);
   Double_t delta_bin = 1.*(last_bin - first_bin) / (npoints - 1.);
   Int_t bin;
   for (int i = 0; i < npoints; i++) {
      bin = first_bin + i * delta_bin;
      Double_t et12 = cumul->GetBinCenter(bin);
      Double_t b = Bmax * sqrt(cumul->GetBinContent(bin));
      Double_t sigma = Smax * cumul->GetBinContent(bin);
      fIPScale->SetPoint(i, et12, b);
      fXSecScale->SetPoint(i, et12, sigma);
   }
   delete cumul;

   fObsTransform = new TF1("fObsTransform", this, &KVImpactParameter::BTransform,
                           fData->GetXaxis()->GetXmin(), fData->GetXaxis()->GetXmax(), 0, "KVImpactParameter", "BTransform");
   fObsTransformXSec = new TF1("fObsTransformXSec", this, &KVImpactParameter::XTransform,
                               fData->GetXaxis()->GetXmin(), fData->GetXaxis()->GetXmax(), 0, "KVImpactParameter", "XTransform");
}

void KVImpactParameter::MakeAbsoluteScale(Int_t npoints, Double_t bmax)
{
   // Calculate the relationship between the impact parameter and the observable
   // whose distribution is contained in the histogram fData.
   // For a given value X of the observable x, the reduced impact parameter
   // b_hat is calculated from the distribution of x, Y(x), using the following formula:
   /*
   BEGIN_LATEX
   \hat{b}^{2} = \frac{\int^{\infty}_{x=X} Y(x) dx}{\int_{0}^{\infty} Y(x) dx}
   END_LATEX
   */
   // npoints = number of points for which to calculate the impact parameter.
   // The greater the number of points, the more accurate the results.
   // Default value is 100. Maximum value is number of bins in histogram of observable, fData.
   // bmax is the maximum absolute impact parameter for the data in [fm].
   // To obtain values of reduced impact parameter/cross-section, use MakeScale.

   Bmax = bmax;
   Smax = GetXSecFromIP(bmax); //total X-section in [mb]
   make_scale(npoints);
}

Double_t KVImpactParameter::BTransform(Double_t* x, Double_t*)
{
   // Function using the TGraph calculated with MakeScale/MakeAbsoluteScale in order to
   // transform distributions of the observable histogrammed in fData
   // into distributions of the impact parameter.
   // This function is used to generate the TF1 fObsTransform

   return fIPScale->Eval(*x);
}

Double_t KVImpactParameter::XTransform(Double_t* x, Double_t*)
{
   // Function using the TGraph calculated with MakeScale/MakeAbsoluteScale in order to
   // transform distributions of the observable histogrammed in fData
   // into distributions of cross-section.
   // This function is used to generate the TF1 fObsTransformXsec

   return fXSecScale->Eval(*x);
}

TH1* KVImpactParameter::GetIPDistribution(TH1* obs, Int_t nbinx, Option_t* norm)
{
   // Transform the distribution of the observable contained in the histogram 'obs'
   // into a distribution of the impact parameter.
   // User's responsibility to delete histo.
   //
   // nbinx = number of bins in I.P. histo (default = 100)
   //  norm = "" (default) : no adjustment is made for the change in bin width due to the transformation
   //  norm = "width" : bin contents are adjusted for width change, so that the integral of the histogram
   //                   contents taking into account the bin width (i.e. TH1::Integral("width")) is the same.

   if (!fObsTransform) {
      Error("GetIPDistribution", "Call MakeScale first to calculate correspondance observable<->i.p.");
      return 0;
   }
   return HM.ScaleHisto(obs, fObsTransform, 0, nbinx, -1, 0., Bmax, -1, -1, norm);
}

TGraph* KVImpactParameter::GetIPEvolution(TH2* obscor, TString moment, TString axis)
{
   // obscor = pointer to histogram containing bidim correlating some observable Y with
   // the observable used to calculate the impact parameter.
   // Return pointer to TGraph giving evolution of any given moment of Y as a function
   // of the impact parameter, with moment = "GetMean", "GetRMS", "GetKurtosis", etc.
   // (methods of TH1)
   // If the impact parameter observable is on the Y-axis of obscor, use axis="X"
   // (by default axis="Y", i.e. we assume that the I.P. observable is on the x axis).

   if (!fObsTransform) {
      Error("GetIPEvolution", "Call MakeScale first to calculate correspondance observable<->i.p.");
      return 0;
   }
   TGraphErrors* gre = HM.GetMomentEvolution(obscor, moment, "", axis);
   TGraph* gr = HM.ScaleGraph(gre, fObsTransform, 0);
   delete gre;
   return gr;
}


TH1* KVImpactParameter::GetXSecDistribution(TH1* obs, Int_t nbinx, Option_t* norm)
{
   // Transform the distribution of the observable contained in the histogram 'obs'
   // into a distribution of cross-section
   // User's responsibility to delete histo.
   //
   // nbinx = number of bins in I.P. histo (default = 100)
   //  norm = "" (default) : no adjustment is made for the change in bin width due to the transformation
   //  norm = "width" : bin contents are adjusted for width change, so that the integral of the histogram
   //                   contents taking into account the bin width (i.e. TH1::Integral("width")) is the same.

   if (!fObsTransformXSec) {
      Error("GetXSecDistribution", "Call MakeScale first to calculate correspondance observable<->i.p.");
      return 0;
   }
   return HM.ScaleHisto(obs, fObsTransformXSec, 0, nbinx, -1, 0., Smax, -1, -1, norm);
}

TGraph* KVImpactParameter::GetXSecEvolution(TH2* obscor, TString moment, TString axis)
{
   // obscor = pointer to histogram containing bidim correlating some observable Y with
   // the observable used to calculate the impact parameter.
   // Return pointer to TGraph giving evolution of any given moment of Y as a function
   // of cross section, with moment = "GetMean", "GetRMS", "GetKurtosis", etc.
   // (methods of TH1)
   // If the impact parameter observable is on the Y-axis of obscor, use axis="X"
   // (by default axis="Y", i.e. we assume that the I.P. observable is on the x axis).

   if (!fObsTransformXSec) {
      Error("GetXSecEvolution", "Call MakeScale first to calculate correspondance observable<->i.p.");
      return 0;
   }
   TGraphErrors* gre = HM.GetMomentEvolution(obscor, moment, "", axis);
   TGraph* gr = HM.ScaleGraph(gre, fObsTransformXSec, 0);
   delete gre;
   return gr;
}

std::vector<Double_t> KVImpactParameter::SliceXSec(Int_t nslices, Double_t totXsec)
{
   // Generate vector of observable values which can be used to select nslices
   // of constant cross-section. Each slice will correspond to a cross-section
   //    totXsec/nslices.
   // Note that the vector will contain (nslices-1) values
   //
   // Example:
   //     KVImpactParameter ip(data);    // histo containing observable distribution
   //     ip.MakeAbsoluteScale(100,ip.GetIPFromXSec(data->Integral()))
   //     std::vector<Double_t> slices = ip.SliceXSec(5, Xsec);
   //     if(obs > slices[0]){
   //            // most central events (observable increases when b decreases)
   //            // with cross-section Xsec/5
   //     } else if(obs > slices[1]){
   //            // next most central events, cross-section=Xsec/5
   //     }
   //      ...
   //     } else if(obs > slices[3]){
   //            // next-to-most-peripheral events
   //     } else {
   //            // most peripheral events (obs < slices[3])
   //     }

   Double_t xsecSlice = totXsec / double(nslices);
   std::vector<Double_t> slices(nslices - 1);
   for (int i = 0; i < nslices - 1; ++i) {
      slices[i] = GetObservableXSec((i + 1) * xsecSlice);
   }
   return slices;
}

