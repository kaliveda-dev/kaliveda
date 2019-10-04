/*
  $Id: KVAlphaCalibration.h, v 1.0 2019/05/14 17:00:00 lemarie Exp $
  $Revision: 1.0 $
  $Date: 2019/05/14 17:00:00 $
  $Author: lemarie $
*/

#ifndef _KVCALIBRATION_H_
#define _KVCALIBRATION_H_

#include "TH1.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TGraph.h"
#include <string>
#include <TMath.h>
#include "TCanvas.h"
#include "TSystem.h"
#include "TFile.h"
#include "TStyle.h"
#include "TROOT.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <limits>


class KVAlphaCalibration {

protected :
   double SigmaOfTSpectrum; // Width of the peaks that TSpectrum will search, chosen by user
   double SigmaOfGaussian; // Width of the peak that the model will fit chosen by user. It is possible that it has to be different from SigmaSpec to get a better fit
   double ThresholdOfTSpectrum; // Lowest peak that the TSpectrum has to search for. It is the ratio of the highest peak that the TSpectrum found. Chosen by user
   bool IsOriginAtZero;
   int NumberOfPeak; // Number of Peak present in the histogram. Chose by user
   TSpectrum* spec; // the method that will search for peaks
   double InitializationFitResults[2]; // Array that contain the results of the initialization fit
   std::vector<double> GaussianFitResults;  //Array that contains results of the final fit
   std::vector<double> GaussianFitResultsError; //Array that contains the error of the results of the final fit
   double FunctionToFit(double* x, double* par); //the model we use to fit the peak
   std::vector<double> InitializationPeak;


private :
   TH1* Histo; //histogram that contains the data to fit. Set by user
   TF1* InitializationFit; //function that will do the initial fit
   TF1* GaussianFit; //function that will fit the peaks
   TGraph* factorGraph; //Graph where the initial fit is done
   std::vector<double> MeanOfPeak; //array peaks energy. Set by user
   std::vector<double> IntensityOfPeak; //array of peaks amlitude. Set by user
   void HistoInit(TH1* h); //-
   TF1 GetFunction(void);
   void SetFunction(std::string FunctionName = "x*[0]");

public :

   KVAlphaCalibration(int NumberOfPeak_); //-
   KVAlphaCalibration(int NumberOfPeak_, TH1* h); //-
   ~KVAlphaCalibration();

   void Init(int);
   void SetHisto(TH1* h);
   void SetParameters(double SigmaOfTSpectrum_ = 1., double SigmaOfGaussian_ = 1., double ThresholdOfTSpectrum_ = 0.5, double IsOriginAtZero_ = false);
   void AddPeak(double Energy_, double Intensity_); //-
   void SetHistRange(double xmin, double xmax);
   double GetGaussianFitParameter(int); //-
   double GetGaussianFitParError(int);
   double GetInitializationFitParameter(int);



   void FitInit(bool debug_ = false);
   void FitSpectrum(bool debug_ = false);
   void FitAll(bool debug_ = false);

   void DrawResult(bool WhatToDraw = true);
   void PrintResult(void);

};

#endif
