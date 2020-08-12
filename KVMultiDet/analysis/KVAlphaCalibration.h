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

/**

   \class KVAlphaCalibration
   \brief Set up and run the calibration of siliciums
   \ingroup Calibration

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
   The default value of ThresholdOfTSpectrum may be fine but if the TSpectrum find to many peaks it can be
   modified with SetThresholdOfTSpectrum.
   Finally the user just has to use the FitInit and FitSpectrum methods in order
   to get the results
   </p>
   <p>
   It is possible for the user to draw the histogram, but he needs to create
   a canvas. The histogram will be drawn in it
   </p>
 */

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
