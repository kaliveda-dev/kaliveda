#include "KVAlphaCalibration.h"

ClassImp(KVAlphaCalibration)


// Constructor

KVAlphaCalibration::KVAlphaCalibration()
{

   std::cerr << "ERROR in KVAlphaCalibration : You can't use the default constructor" << std::endl;
   std::exit(1);

}

KVAlphaCalibration::KVAlphaCalibration(TH1* h_, int N_)
{
// If you want to use a loop, it is advised to use the constructor the third constructor and to initialize the histogram in the loop

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

}

// Initialization methods

void KVAlphaCalibration::HistoInit(TH1* h_)
{

   histo = h_;
   double min = histo->GetBinCenter(h_->GetMinimumBin());
   double max = histo->GetBinCenter(h_->GetMaximumBin());
   fitPeak = new TF1("fitPeak", this, &KVAlphaCalibration::FunctionToFit, min, max, NPeak + 3, "KVAlphaCalibration", "FonctionToFit");

}

void KVAlphaCalibration::SetHisto(TH1* h)
{

   HistoInit(h);

}

void KVAlphaCalibration::PeakInit(int N_)
{

   if (N_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration Constructor : The number of peak you want to fit has to be positive" << std::endl;
      std::exit(1);
   }

   NPeak = N_;
}

void KVAlphaCalibration::init(void)
{

   spec = new TSpectrum();
   fCal = new TF1("calib", "pol1", 0, 10);
   factorGraph = new TGraph;
   sigma = 1.;
   threshold = 0.05;
   NPeak = 0;
   LinearFitResults[0] = 0;
   LinearFitResults[1] = 0;

}

// Setting parameters

void KVAlphaCalibration::SetPeak(double M, double N)
{


   if (N <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetPeak : Normalization factor can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   PeakMean.push_back(M);
   PeakNorm.push_back(N);

}

void KVAlphaCalibration::SetSigma(double s)
{

   if (s <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetSigma : Sigma can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   sigma = s;

}


void KVAlphaCalibration::SetThreshold(double t)
{

   if (t <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetThreshold : Threshold can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   threshold = t;

}

// Getting parameters

double KVAlphaCalibration::GetLinearFitParameter(int j)
{

   if (j != 0 || j != 1) {
      std::cerr << "ERROR in KVAlphaCalibration::GetLinerFitParameter : You asked for a wrong parameter value, it needs to be 1 or 0"
                << std::endl
                << "-> Ignoring command" << std::endl;
      return std::numeric_limits<double>::quiet_NaN();

   }

   return LinearFitResults[j];

}

double KVAlphaCalibration::GetPeakFitParameter(int j)
{

   if (j < 0 || j > NPeak + 2) {
      std::cerr << "ERROR in KVAlphaCalibration::GetPeakFitParameter : You asked for a wrong parameter value, it needs to be between 0 or "
                << NPeak + 3
                << std::endl
                << "-> Ignoring command" << std::endl;
      return std::numeric_limits<double>::quiet_NaN();

   }

   return PeakFitResults[j];

}

double KVAlphaCalibration::GetSigma(void)
{

   return sigma;

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

   FitLinear(debug_);
   FitPeak(debug_);

}

void KVAlphaCalibration::FitLinear(bool debug_)
{


   TCanvas* cc = new TCanvas();
   if (debug_) std::cerr << "DEBUG IN FitLinear : Searching for peaks in histogram" << std::endl;
   spec->Search(histo, sigma, "", threshold);
   Float_t* xpos = spec->GetPositionX();
   pos1 = xpos[0];

   if (debug_) std::cerr << "DEBUG IN FitLinear : Number of peaks found is " << spec->GetNPeaks() << std::endl;
   if (debug_) if (spec->GetNPeaks() != NPeak) std::cerr << "DEBUG IN FitLinear : Number of peaks different from the one you asked"
               << "-> Ignoring histogram" << std::endl
               << "Please modify your TSpectrum parameters (aka Sigma and Threshold)" << std::endl;


   if (spec->GetNPeaks() == NPeak) {
      if (debug_) cc->SaveAs("FitLinearControl.pdf(");

      for (int i = 0; i < NPeak; i++) {

         if (debug_) std::cerr << "DEBUG IN FitLinear : Peak position number " << i << " = " << xpos[i] << std::endl;
         factorGraph->SetPoint(i, PeakMean[i], xpos[i]);

      }
      cc->Clear();

      if (debug_) std::cerr << "DEBUG IN FitLinear : Initializing Parameters" << std::endl;
      fCal->SetParameter(0, 1);
      fCal->SetParameter(1, xpos[0] / PeakMean[0]);

      if (debug_) std::cerr << "DEBUG IN FitLinear : Fitting" << std::endl;
      factorGraph->SetMarkerStyle(5);
      factorGraph->Fit("calib");
      if (debug_) std::cerr << "DEBUG IN FitLinear : Fitting done" << std::endl;

      if (debug_)  std::cerr << "DEBUG IN FitLinear : Writing fit output in LinearFitResults array" << std::endl;
      LinearFitResults[0] = fCal->GetParameter(0);
      LinearFitResults[1] = fCal->GetParameter(1);

   }

   delete cc;
   if (debug_)  std::cerr << "DEBUG IN FitLinear : Ending FitLineat" << std::endl;
}

void KVAlphaCalibration::FitPeak(bool debug_)
{


   TCanvas* cc = new TCanvas();

   if (debug_) std::cerr << "Entering FitPeak method" << std::endl;
   if (debug_) std::cerr << "Setting Parameters" << std::endl;
   fitPeak->SetParameter(0, LinearFitResults[1]);
   fitPeak->SetParameter(1, LinearFitResults[0]);
   fitPeak->SetParameter(2, sigma);

   if (debug_) std::cerr << "Setting Normalization factors" << std::endl;
   for (int i = 0; i < NPeak; i++) {

      fitPeak->SetParameter(i + 3, PeakNorm[i]);

   }

   // Order of parameter : Conversion Factor - Y at x = 0 - Sigma factor - Norm_i

   if (debug_) std::cerr << "Fitting" << std::endl;
   histo->Fit("fitPeak");
   if (debug_)  std::cerr << "FitEnded" << std::endl;
   if (debug_) cc->SaveAs("FitPeakControl.pdf");



   for (int i = 0; i < NPeak + 3; i++) {

      PeakFitResults.push_back(fitPeak->GetParameter(i));

   }
   if (debug_) std::cerr << "FitPeak ended" << std::endl;

   delete cc;
}

double KVAlphaCalibration::FunctionToFit(double* x, double* par)
{

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

      std::cout << "Conversion factor : " << 1 / PeakFitResults[0] << std::endl
                << " Y at x = 0 : " << PeakMean[0] - PeakFitResults[1] / PeakFitResults[0] << std::endl
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

   std::cout << 1 / PeakFitResults[0] << std::endl
             << PeakMean[0] - PeakFitResults[1] / PeakFitResults[0] << std::endl
             << PeakFitResults[2] << std::endl;

   for (int i = 3; i < NPeak + 3; i++) {

      std::cout << PeakFitResults[i] << std::endl;

   }


}
