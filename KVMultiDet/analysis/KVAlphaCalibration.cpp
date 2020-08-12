/*
  $Id: KVAlphaCalibration.h, v 1.0 2019/05/14 17:00:00 lemarie Exp $
  $Revision: 1.0 $
  $Date: 2019/05/14 17:00:00 $
  $Author: lemarie $
*/

#include "KVAlphaCalibration.h"

ClassImp(KVAlphaCalibration)

KVAlphaCalibration::KVAlphaCalibration(int NumberOfPeak_)
{

   Init(NumberOfPeak_);

}

KVAlphaCalibration::KVAlphaCalibration(int NumberOfPeak_, TH1* h)
{

   Init(NumberOfPeak_);
   HistoInit(h);

}

KVAlphaCalibration::~KVAlphaCalibration()
{
   //Default destructor
}

void KVAlphaCalibration::HistoInit(TH1* h)
{

   Histo = h;
   double min = Histo->GetBinCenter(h->GetMinimumBin());
   double max = Histo->GetBinCenter(h->GetMaximumBin());
   GaussianFit = new TF1("fitPeak", this, &KVAlphaCalibration::FunctionToFit, min, max, NumberOfPeak + 3, "KVAlphaCalibration", "FonctionToFit");

}

//_____________________________________________________________________

void KVAlphaCalibration::Init(int NumberOfPeak_)
{

   spec = new TSpectrum();

   factorGraph = new TGraph();
   InitializationFit = new TF1("calib", "pol1", 0, 100);
   SigmaOfTSpectrum = 1.;
   ThresholdOfTSpectrum = 0.05;
   NumberOfPeak = 0;

   if (NumberOfPeak_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration Constructor : The number of peak you want to fit has to be positive" << std::endl;
      std::exit(1);
   }
   NumberOfPeak = NumberOfPeak_;
}

void KVAlphaCalibration::AddPeak(double Energy_, double Intensity_)
{

   //Set the energy and the normalization factor the peaks you want to fit.
   //The normalization factor is only here to constrain the model

   if (Intensity_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetPeak : Normalization factor can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   MeanOfPeak.push_back(Energy_);
   IntensityOfPeak.push_back(Intensity_);

}

void KVAlphaCalibration::SetHisto(TH1* h)
{

   //Set the histogram that contains the data
   HistoInit(h);

}

void KVAlphaCalibration::SetHistRange(double xmin, double xmax)
{

   Histo->GetXaxis()->SetRangeUser(xmin, xmax);

}

void KVAlphaCalibration::SetParameters(double SigmaOfTSpectrum_, double SigmaOfGaussian_, double ThresholdOfTSpectrum_, double IsOriginAtZero_)
{

   if (SigmaOfTSpectrum_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetParameters : SigmaOfTSpectrum can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   SigmaOfTSpectrum = SigmaOfTSpectrum_;

   if (SigmaOfGaussian_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetParameters : SigmaOfGaussian can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   SigmaOfGaussian = SigmaOfGaussian_;

   if (ThresholdOfTSpectrum_ <= 0) {
      std::cerr << "ERROR in KVAlphaCalibration::SetParameters : ThresholdOfTSpectrumOfTSpectrum can not be equal nor inferior to 0" << std::endl;
      std::exit(1);
   }

   IsOriginAtZero = IsOriginAtZero_;

   ThresholdOfTSpectrum = ThresholdOfTSpectrum_;

}



double KVAlphaCalibration::GetInitializationFitParameter(int j)
{

   //Get the initialization parameter of the model.
   //0 for the slope
   //1 for the ordinate at the origin
   if (j != 0 && j != 1) {
      std::cerr << "ERROR in KVAlphaCalibration::GetLinerFitParameter : You asked for a wrong parameter value, it needs to be 1 or 0"
                << std::endl
                << "-> Ignoring command" << std::endl;
      return 0.;

   }

   return InitializationFitResults[j];

}

double KVAlphaCalibration::GetGaussianFitParameter(int j)
{
   //Get the final parameter found by the model
   //0 for the slope
   //1 for the ordinate at the origin
   //2 for the width factor. This factor needs to be multiplied by the energy of one peak in order to get his width
   //3 - NumberOfPeak + 3 for the normalization factor

   if (j < 0 || j > NumberOfPeak + 2) {
      std::cerr << "ERROR in KVAlphaCalibration::GetGaussianFitParameter : You asked for a wrong parameter value, it needs to be between 0 or "
                << NumberOfPeak + 3
                << std::endl
                << "-> Ignoring command" << std::endl;
      return 0.;

   }

   return GaussianFitResults[j];

}

double KVAlphaCalibration::GetGaussianFitParError(int j)
{
   //Get the error of final parameter found by the model
   //0 for the error of the slope
   //1 for the error of the ordinate at the origin
   //2 for the error of the width factor. This factor needs to be multiplied by the energy of one peak in order to get his width
   //3 - NumberOfPeak + 3 for the error of the normalization factor

   if (j < 0 || j > NumberOfPeak + 2) {
      std::cerr << "ERROR in KVAlphaCalibration::GetGaussianFitParameter : You asked for a wrong parameter value, it needs to be between 0 or "
                << NumberOfPeak + 3
                << std::endl
                << "-> Ignoring command" << std::endl;
      return 0.;

   }

   return GaussianFitResultsError[j];

}


// Fitting methods

void KVAlphaCalibration::FitAll(bool debug_)
{

   //This function calls the FitInit and FitSpectrum function
   FitInit(debug_);
   FitSpectrum(debug_);

}

void KVAlphaCalibration::FitInit(bool debug_)
{

   //This method find the peaks with a TSpectrum and fit their position to
   //initialize the model
   //The boolean is false by default, but it is advised to set it true in order to
   //check if the TSpectrum find the right peaks. If not, change the parameter SigmaOfTSpectrum
   //and ThresholdOfTSpectrum

   if (debug_) std::cerr << "DEBUG IN FitInit : Searching for peaks in histogram" << std::endl;
   unsigned int npeaks = spec->Search(Histo, SigmaOfTSpectrum, "", ThresholdOfTSpectrum);

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   Double_t* xpos = spec->GetPositionX();
#else
   Float_t* xpos = spec->GetPositionX();
#endif

   InitializationPeak.clear();
   for (unsigned int i = 0; i <= npeaks; i++) {

      InitializationPeak.push_back(xpos[i]);
   }

   std::sort(InitializationPeak.begin(), InitializationPeak.end());
   std::sort(MeanOfPeak.begin(), MeanOfPeak.end());
   std::sort(IntensityOfPeak.begin(), IntensityOfPeak.end());

   if (debug_) std::cerr << "DEBUG IN FitInit : Number of peaks found is " << spec->GetNPeaks() << std::endl;
   if (debug_) if (spec->GetNPeaks() != NumberOfPeak) std::cerr << "DEBUG IN FitInit : Number of peaks different from the one you asked"
               << "-> Ignoring histogram" << std::endl
               << "Please modify your TSpectrum parameters (aka SigmaOfTSpectrum and ThresholdOfTSpectrumOfTSpectrum)" << std::endl;
   //In mode debug it will also print the result of the linear fit in the terminal
   if (spec->GetNPeaks() == NumberOfPeak) {

      for (int i = 0; i < NumberOfPeak; i++) {

         if (debug_) std::cerr << "DEBUG IN FitInit : Peak position number " << i << " = " << InitializationPeak[i] << std::endl;
         factorGraph->SetPoint(i, MeanOfPeak[i], InitializationPeak[i]);

      }


      if (debug_) std::cerr << "DEBUG IN FitInit : Initializing Parameters" << std::endl;

      InitializationFit->SetParameter(0, 1);
      InitializationFit->SetParameter(1, xpos[0] / MeanOfPeak[0]);

      if (debug_) std::cerr << "DEBUG IN FitInit : Fitting" << std::endl;
      factorGraph->SetMarkerStyle(5);
      factorGraph->Fit("calib", "Q");
      if (debug_) std::cerr << "DEBUG IN FitInit : Fitting done" << std::endl;

      if (debug_)  std::cerr << "DEBUG IN FitInit : Writing fit output in InitializationFitResults array" << std::endl;
      InitializationFitResults[0] = InitializationFit->GetParameter(0);

      InitializationFitResults[1] = InitializationFit->GetParameter(1);

   }

   if (debug_)  std::cerr << "DEBUG IN FitInit : Ending FitInit" << std::endl;
}

void KVAlphaCalibration::FitSpectrum(bool debug_)
{

   //This methods will use the model and fit the peaks in the histogram with
   // a gaussian in order to get a very accurate calibration
   //It also print the results at the end of the fit

   std::vector<double> GaussianFitResultsTemp;
   std::vector<double> GaussianFitResultsErrorTemp;

   if (debug_) std::cerr << "Entering FitSpectrum method" << std::endl;
   if (debug_) std::cerr << "Setting Parameters" << std::endl;

   std::cout << &InitializationFitResults/*[1] << "  " << InitializationFitResults[0] */ << std::endl;
   GaussianFit->SetParameter(0, InitializationFitResults[1]);

   if (IsOriginAtZero) {

      GaussianFit->FixParameter(1, 0);

   }
   else {

      GaussianFit->SetParameter(1, InitializationFitResults[0]);

   }

   GaussianFit->SetParameter(2, SigmaOfGaussian);
   GaussianFit->SetNpx(1000);

   if (debug_) std::cerr << "Setting Normalization factors" << std::endl;
   for (int i = 0; i < NumberOfPeak; i++) {

      GaussianFit->SetParameter(i + 3, IntensityOfPeak[i]);

   }

   if (debug_) {
      std::cerr << "Fitting" << std::endl;
      Histo->Fit("fitPeak");
      std::cerr << "FitEnded" << std::endl;
   }
   else Histo->Fit("fitPeak", "Q");


   GaussianFitResults.clear();
   GaussianFitResultsError.clear();
   GaussianFitResultsTemp.clear();
   GaussianFitResultsErrorTemp.clear();

   for (int i = 0; i < NumberOfPeak + 3; i++) {

      GaussianFitResultsTemp.push_back(GaussianFit->GetParameter(i));
      GaussianFitResultsErrorTemp.push_back(GaussianFit->GetParError(i));
   }


   GaussianFitResults.push_back(1 / GaussianFitResultsTemp[0]);
   GaussianFitResults.push_back(-GaussianFitResultsTemp[1] / GaussianFitResultsTemp[0]);

   GaussianFitResults.push_back(GaussianFitResultsTemp[2] / GaussianFitResultsTemp[0]);

   GaussianFitResultsError.push_back(GaussianFitResultsErrorTemp[0] / GaussianFitResultsTemp[0]);
   GaussianFitResultsError.push_back(GaussianFitResultsTemp[1] * 0.02);
   GaussianFitResultsError.push_back(GaussianFitResultsErrorTemp[2] / GaussianFitResultsTemp[0]);

   for (int i = 3; i < NumberOfPeak + 3; i++) {

      GaussianFitResults.push_back(GaussianFitResultsTemp[i]);
      GaussianFitResultsError.push_back(GaussianFitResultsErrorTemp[i]);
   }

   if (debug_) std::cerr << "FitSpectrum ended" << std::endl;
   PrintResult();

}

double KVAlphaCalibration::FunctionToFit(double* x, double* par)
{

   //Model called by the FitSpectrum method. Can not be called by the user
   //It consist in a sum of gaussian. The number is the number of peak given by the user

   double gauss[NumberOfPeak];
   double factor_ = par[0];
   double result = 0;
   double S = par[2];
   double b = par[1];


   for (int i = 0; i < NumberOfPeak; i++) {

      gauss[i] = par[i + 3] / MeanOfPeak[i] * S * TMath::Sqrt(2 * TMath::Pi())
                 * TMath::Exp(-TMath::Power(x[0] - factor_ * MeanOfPeak[i] - b, 2) / (2 * TMath::Power(S, 2)));
      result += gauss[i];

   }

   return result;

}


// Drawing and printing results

void KVAlphaCalibration::DrawResult(bool WhatToDraw)
{

   if (WhatToDraw) {

      Histo->Draw();
      GaussianFit->Draw("same");

      std::cout << "Conversion factor : " << GaussianFitResults[0] << std::endl
                << " Y at x = 0 : " <<  GaussianFitResults[1] << std::endl
                << "Sigma Factor : " << GaussianFitResults[2] << std::endl;

      for (int i = 3; i < NumberOfPeak + 3; i++) {

         std::cout << " Normalization factor " << i << " : " << GaussianFitResults[i] << std::endl;

      }

   }
   else

      factorGraph->Draw();

}


void KVAlphaCalibration::PrintResult(void)
{

   std::cout << "Slope : " << GaussianFitResults[0] << std::endl
             << "Y at x = 0 : " << GaussianFitResults[1] << std::endl
             << "Peak width factor : " << GaussianFitResults[2] << std::endl;

}
