/*
$Id: KVAlphaCalibration.h, v 1.0 2019/05/14 17:00:00 lemarie Exp $
$Revision: 1.0 $
$Date: 2019/05/14 17:00:00 $
$Author: lemarie $
*/

#include "KVAlphaCalibration.h"

ClassImp(KVAlphaCalibration)
//////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVAlphaCalibration</h2>
<h4>Set up and run the calibration of siliciums</h4>
<p>
This can be used either 'interactively' or in menu-driven mode.
In both cases, in order to run a calirbation, the user must specify
</p>
<ul>
<li>the histogram</li>
<li>the energy and normalization factor of the peaks present in the histogram</li>
<li>the parameter of the method  that will initialise the position of the peaks</li>
</ul>
<p>
The role of this class is to search for a number of peaks given by the user in a histogram
and to fit them in order to get several information about the response of the detectors. This class
was made in order to calibrate silicium with alpha but it can be use for every detectors which
response is a gaussian.
</p>
<h4>The way it has to be used </h4>
<p>
To set up the calibration class the user needs to give the number of peaks that
has to be found in the constructor.
If the user wants to fit several peaks with
one program, it is recommended to set up only one KVAlphaCalibration class and
to set the histograms in a loop, instead of using the constructor with a histogram.
</p>
<p>
Afterwards, the user needs to set the energy and the normalization factor of the peaks with
the method SetPeak(). The method SetSigmaSpec needs to be used in order to set up the
TSpectrum that will initialise the model, while SetSigmaFit will set the width for the model.
The default value of threshold may be fine but if the TSpectrum find to many peaks it can be
modified with SetThreshold.
Finally the user just has to use the FitLinear and FitPeak methods in order
to get the results
</p>
<p>
It is possible for the user to draw the histogram, but he needs to create
a canvas. The histogram will be drawn in it
</p>

<p>
</p>
<pre>
#include "KVBatchSystem.h"
...
</pre>
<!-- */
// --> END_HTML
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






KVAlphaCalibration::KVAlphaCalibration()
{
   //Default constructor. Should not be use since the initialisation of the class needs the number of peaks
   std::cerr << "ERROR in KVAlphaCalibration : You can't use the default constructor" << std::endl;
   std::exit(1);

}

KVAlphaCalibration::KVAlphaCalibration(TH1* h_, int N_)
{
   // If you want to use a loop, it is advised to use the constructor that initialise the peaks only


   init();
   PeakInit(N_);
   HistoInit(h_);

}

KVAlphaCalibration::KVAlphaCalibration(int N_)
{


   init();
   PeakInit(N_);

}

KVAlphaCalibration::~KVAlphaCalibration()
{
   //Default destructor
}

void KVAlphaCalibration::init(void)
{

   spec = new TSpectrum();

   fCal = new TF1("calib", "pol1", 0, 10);
   factorGraph = new TGraph;
   SigmaSpec = 1.;
   threshold = 0.05;
   NPeak = 0;
   LinearFitResults[0] = 0;
   LinearFitResults[1] = 0;

}



void KVAlphaCalibration::HistoInit(TH1* h_)
{

   histo = h_;
   double min = histo->GetBinCenter(h_->GetMinimumBin());
   double max = histo->GetBinCenter(h_->GetMaximumBin());
   fitPeak = new TF1("fitPeak", this, &KVAlphaCalibration::FunctionToFit, min, max, NPeak + 3, "KVAlphaCalibration", "FonctionToFit");

}

void KVAlphaCalibration::PeakInit(int N_)
{

   if (N_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration Constructor : The number of peak you want to fit has to be positive" << std::endl;
      std::exit(1);
   }

   NPeak = N_;

}

//_____________________________________________________________________

void KVAlphaCalibration::SetPeak(double M, double N)
{

   //Set the energy and the normalization factor the peaks you want to fit.
   //The normalization factor is only here to constrain the model

   if (N <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetPeak : Normalization factor can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   PeakMean.push_back(M);
   PeakNorm.push_back(N);

}

void KVAlphaCalibration::SetHisto(TH1* h)
{

   //Set the histogram that contains the data
   HistoInit(h);

}

void KVAlphaCalibration::SetHistRange(double xmin, double xmax)
{

   histo->GetXaxis()->SetRangeUser(xmin, xmax);

}

void KVAlphaCalibration::SetSigmaSpec(double s)
{

   //Set the sigma parameter of the TSpectrum that will search  the peaks.
   if (s <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetSigmaSpec : SigmaSpec can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   SigmaSpec = s;

}

void KVAlphaCalibration::SetSigmaFit(double s)
{

   //Set the width of the gaussian that will be fitted by the model. It may be different from SigmaSpec
   if (s <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetSigmaSpec : SigmaSpec can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   SigmaFit = s;

}

void KVAlphaCalibration::SetThreshold(double t)
{

   //Set the minimum amplitude of the peaks that the TSpectrum has to look for
   if (t <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetThreshold : Threshold can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   threshold = t;

}

double KVAlphaCalibration::GetLinearFitParameter(int j)
{

   //Get the initialization parameter of the model.
   //0 for the slope
   //1 for the ordinate at the origin
   if (j != 0 || j != 1) {
      std::cerr << "ERROR in KVAlphaCalibration::GetLinerFitParameter : You asked for a wrong parameter value, it needs to be 1 or 0"
                << std::endl
                << "-> Ignoring command" << std::endl;
      return 0.;

   }

   return LinearFitResults[j];

}

double KVAlphaCalibration::GetPeakFitParameter(int j)
{
   //Get the final parameter found by the model
   //0 for the slope
   //1 for the ordinate at the origin
   //2 for the width factor. This factor needs to be multiplied by the energy of one peak in order to get his width
   //3 - NPeak + 3 for the normalization factor

   if (j < 0 || j > NPeak + 2) {
      std::cerr << "ERROR in KVAlphaCalibration::GetPeakFitParameter : You asked for a wrong parameter value, it needs to be between 0 or "
                << NPeak + 3
                << std::endl
                << "-> Ignoring command" << std::endl;
      return 0.;

   }

   return PeakFitResults[j];

}

double KVAlphaCalibration::GetPeakFitParError(int j)
{
   //Get the error of final parameter found by the model
   //0 for the error of the slope
   //1 for the error of the ordinate at the origin
   //2 for the error of the width factor. This factor needs to be multiplied by the energy of one peak in order to get his width
   //3 - NPeak + 3 for the error of the normalization factor

   if (j < 0 || j > NPeak + 2) {
      std::cerr << "ERROR in KVAlphaCalibration::GetPeakFitParameter : You asked for a wrong parameter value, it needs to be between 0 or "
                << NPeak + 3
                << std::endl
                << "-> Ignoring command" << std::endl;
      return 0.;

   }

   return PeakFitResultsError[j];

}


double KVAlphaCalibration::GetSigmaSpec(void)
{

   return SigmaSpec;

}

double KVAlphaCalibration::GetSigmaFit(void)
{

   return SigmaFit;

}

double KVAlphaCalibration::GetThreshold(void)
{

   return threshold;

}

TH1* KVAlphaCalibration::GetHisto(void)
{

   return histo;

}

// Fitting methods

void KVAlphaCalibration::FitAll(bool debug_)
{

   //This function calls the FitLinear and FitPeak function
   FitLinear(debug_);
   FitPeak(debug_);

}

void KVAlphaCalibration::FitLinear(bool debug_)
{

   //This method find the peaks with a TSpectrum and fit their position to
   //initialize the model
   //The boolean is false by default, but it is advised to set it true in order to
   //check if the TSpectrum find the right peaks. If not, change the parameter SigmaSpec
   //and threshold

   if (debug_) std::cerr << "DEBUG IN FitLinear : Searching for peaks in histogram" << std::endl;
   spec->Search(histo, SigmaSpec, "", threshold);

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   Double_t* xpos = spec->GetPositionX();
#else
   Float_t* xpos = spec->GetPositionX();
#endif

   PeakSpectrum.clear();
   for (unsigned int i = 0; i < sizeof(xpos) / sizeof(xpos[0]); i++) {

      PeakSpectrum.push_back(xpos[i]);

   }

   std::sort(PeakSpectrum.begin(), PeakSpectrum.end());
   std::sort(PeakMean.begin(), PeakMean.end());
   std::sort(PeakNorm.begin(), PeakNorm.end());

   if (debug_) std::cerr << "DEBUG IN FitLinear : Number of peaks found is " << spec->GetNPeaks() << std::endl;
   if (debug_) if (spec->GetNPeaks() != NPeak) std::cerr << "DEBUG IN FitLinear : Number of peaks different from the one you asked"
               << "-> Ignoring histogram" << std::endl
               << "Please modify your TSpectrum parameters (aka SigmaSpec and Threshold)" << std::endl;
   //In mode debug it will also print the result of the linear fit in the terminal
   if (spec->GetNPeaks() == NPeak) {

      for (int i = 0; i < NPeak; i++) {

         if (debug_) std::cerr << "DEBUG IN FitLinear : Peak position number " << i << " = " << xpos[i] << std::endl;
         factorGraph->SetPoint(i, PeakMean[i], xpos[i]);

      }


      if (debug_) std::cerr << "DEBUG IN FitLinear : Initializing Parameters" << std::endl;

      fCal->SetParameter(0, 1);
      fCal->SetParameter(1, xpos[0] / PeakMean[0]);

      if (debug_) std::cerr << "DEBUG IN FitLinear : Fitting" << std::endl;
      factorGraph->SetMarkerStyle(5);
      factorGraph->Fit("calib", "Q");
      if (debug_) std::cerr << "DEBUG IN FitLinear : Fitting done" << std::endl;

      if (debug_)  std::cerr << "DEBUG IN FitLinear : Writing fit output in LinearFitResults array" << std::endl;
      LinearFitResults[0] = fCal->GetParameter(0);

      LinearFitResults[1] = fCal->GetParameter(1);

   }

   if (debug_)  std::cerr << "DEBUG IN FitLinear : Ending FitLinear" << std::endl;
}

void KVAlphaCalibration::FitPeak(bool debug_)
{

   //This methods will use the model and fit the peaks in the histogram with
   // a gaussian in order to get a very accurate calibration
   //It also print the results at the end of the fit


   if (debug_) std::cerr << "Entering FitPeak method" << std::endl;
   if (debug_) std::cerr << "Setting Parameters" << std::endl;
   fitPeak->SetParameter(0, LinearFitResults[1]);
   fitPeak->SetParameter(1, LinearFitResults[0]);
   fitPeak->SetParameter(2, SigmaFit);
   fitPeak->SetNpx(1000);

   if (debug_) std::cerr << "Setting Normalization factors" << std::endl;
   for (int i = 0; i < NPeak; i++) {

      fitPeak->SetParameter(i + 3, PeakNorm[i]);

   }

   if (debug_) {
      std::cerr << "Fitting" << std::endl;
      histo->Fit("fitPeak");
      std::cerr << "FitEnded" << std::endl;
   }
   else histo->Fit("fitPeak", "Q");


   PeakFitResults.clear();
   PeakFitResultsError.clear();
   PeakFitResultsTemp.clear();
   PeakFitResultsErrorTemp.clear();

   for (int i = 0; i < NPeak + 3; i++) {

      PeakFitResultsTemp.push_back(fitPeak->GetParameter(i));
      PeakFitResultsErrorTemp.push_back(fitPeak->GetParError(i));
   }


   PeakFitResults.push_back(1 / PeakFitResultsTemp[0]);
   PeakFitResults.push_back(-PeakFitResultsTemp[1] / PeakFitResultsTemp[0]);
   PeakFitResults.push_back(PeakFitResultsTemp[2] / PeakFitResultsTemp[0]);

   PeakFitResultsError.push_back(PeakFitResultsErrorTemp[0] / PeakFitResultsTemp[0]);
   PeakFitResultsError.push_back(PeakFitResultsTemp[1] * 0.02);
   PeakFitResultsError.push_back(PeakFitResultsErrorTemp[2] / PeakFitResultsTemp[0]);

   for (int i = 3; i < NPeak + 3; i++) {

      PeakFitResults.push_back(PeakFitResultsTemp[i]);
      PeakFitResultsError.push_back(PeakFitResultsErrorTemp[i]);
   }

   if (debug_) std::cerr << "FitPeak ended" << std::endl;
   PrintResult();

}

double KVAlphaCalibration::FunctionToFit(double* x, double* par)
{

   //Model called by the FitPeak method. Can not be called by the user
   //It consist in a sum of gaussian. The number is the number of peak given by the user

   double gauss[NPeak];
   double factor_ = par[0];
   double result = 0;
   double S = par[2];
   double b = par[1];

   for (int i = 0; i < NPeak; i++) {

      gauss[i] = par[i + 3] / PeakMean[i] * S * TMath::Sqrt(2 * TMath::Pi())
                 * TMath::Exp(-TMath::Power(x[0] - factor_ * PeakMean[i] - b, 2) / (2 * TMath::Power(PeakMean[i] * S, 2)));
      result += gauss[i];

   }

   return result;

}


// Drawing and printing results

void KVAlphaCalibration::DrawResult(bool WhatToDraw)
{

   if (WhatToDraw) {

      histo->Draw();
      fitPeak->Draw("same");

      std::cout << "Conversion factor : " << PeakFitResults[0] << std::endl
                << " Y at x = 0 : " <<  PeakFitResults[1] << std::endl
                << "Sigma Factor : " << PeakFitResults[2] << std::endl;

      for (int i = 3; i < NPeak + 3; i++) {

         std::cout << " Normalization factor " << i << " : " << PeakFitResults[i] << std::endl;

      }

   }
   else

      factorGraph->Draw();

}


void KVAlphaCalibration::PrintResult(void)
{

   std::cout << "Slope : " << PeakFitResults[0] << std::endl
             << "Y at x = 0 : " << PeakFitResults[1] << std::endl
             << "Peak width factor : " << PeakFitResults[2] << std::endl;

}
