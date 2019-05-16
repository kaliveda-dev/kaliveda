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
#include <limits>

class KVAlphaCalibration {

protected :
   double SigmaSpec; // Width of the peaks that TSpectrum will search, chosen by user
   double SigmaFit; // Width of the peak that the model will fit chosen by user. It is possible that it has to be different from SigmaSpec to get a better fit
   double threshold; // Lowest peak that the TSpectrum has to search for. It is the ratio of the highest peak that the TSpectrum found. Chosen by user
   int NPeak; // Number of Peak present in the histogram. Chose by user
   TSpectrum* spec; // the method that will search for peaks
   double LinearFitResults[2]; // Array that contain the results of the initialization fit
   std::vector<double> PeakFitResults;  //Array that contains results of the final fit
   std::vector<double> PeakFitResultsTemp;
   std::vector<double> PeakFitResultsError; //Array that contains the error of the results of the final fit
   std::vector<double> PeakFitResultsErrorTemp;
   double FunctionToFit(double* x, double* par); //the model we use to fit the peak

private :
   TH1* histo; //histogram that contains the data to fit. Set by user
   TF1* fCal; //function that will do the initial fit
   TF1* fitPeak; //function that will fit the peaks
   TGraph* factorGraph; //Graph where the initial fit is done
   std::vector<double> PeakMean; //array peaks energy. Set by user
   std::vector<double> PeakNorm; //array of peaks amlitude. Set by user


public :

   KVAlphaCalibration(); //-
   KVAlphaCalibration(int NPeak_); //-
   KVAlphaCalibration(TH1* h, int NPeak_); //-
   ~KVAlphaCalibration();

   void init(void);
   void HistoInit(TH1* h); //-
   void PeakInit(int N); //-

   void SetHisto(TH1* h);
   TH1* GetHisto(void);
   void SetPeak(double M, double N); //-
   void SetSigmaSpec(double s); //-
   double GetSigmaSpec(void); //-
   void SetSigmaFit(double s); //-
   double GetSigmaFit(void); //-
   void SetThreshold(double t); //-
   void SetHistRange(double xmin, double xmax);
   double GetThreshold(void); //-
   double GetPeakFitParameter(int); //-
   double GetPeakFitParError(int);
   double GetLinearFitParameter(int);


   void FitLinear(bool debug_ = false);
   void FitPeak(bool debug_ = false);
   void FitAll(bool debug_ = false);

   void DrawResult(bool WhatToDraw = true);
   void PrintResult(void);

};

#endif
