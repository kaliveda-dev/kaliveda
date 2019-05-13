#ifndef _KVCALIBRATION_H_
#define _KVCALIBRATION_H_

#include "TH1.h"
#include "TF1.h"
#include <TSpectrum.h>
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
   double sigma; //-
   double threshold; //-
   int NPeak;//
   double pos1;
   TSpectrum* spec;
   double LinearFitResults[2];
   std::vector<double> PeakFitResults;
   double FunctionToFit(double* x, double* par);

private :
   TH1* histo; //-
   TF1* fCal; //-
   TF1* fitPeak;
   TGraph* factorGraph; //-
   std::vector<double> PeakMean; //-
   std::vector<double> PeakNorm; //-


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
   void SetSigma(double s); //-
   double GetSigma(void); //-
   void SetThreshold(double t); //-
   double GetThreshold(void); //-
   double GetPeakFitParameter(int); //-
   double GetLinearFitParameter(int);

   void FitLinear(bool debug_ = false);
   void FitPeak(bool debug_ = false);
   void FitAll(bool debug_ = false);

   void DrawResult(bool WhatToDraw = true);
   void PrintResult(void);

};

#endif
