/*
$Id: KVHistoManipulator.h,v 1.8 2009/04/07 14:54:15 ebonnet Exp $
$Revision: 1.8 $
$Date: 2009/04/07 14:54:15 $
*/

//Created by KVClassFactory on Thu Oct 18 11:48:18 2007
//Author: bonnet

#ifndef __KVHISTOMANIPULATOR_H
#define __KVHISTOMANIPULATOR_H

#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "KVList.h"
#include "TString.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMath.h"

class TCanvas;
class KVNumberList;
class TMultiGraph;

/**
  \class KVHistoManipulator
  \brief Toolkit for various operations on histograms & graphs not provided by ROOT
  \ingroup Analysis
 */

class KVHistoManipulator {
   Bool_t kVisDebug;// = kTRUE for visual debugging
   TCanvas* fVDCanvas;//! used for visual debugging

public:

   void init(void)
   {

   }

   KVHistoManipulator();
   virtual ~KVHistoManipulator(void);

   void SetVisDebug(Bool_t on = kTRUE)
   {
      // Turn on/off 'VisualDebugging' for RescaleX methods.
      // After calling SetVisDebug(kTRUE), whenever the rescaling procedure
      // is executed, a TCanvas is drawn showing 4 pads, which show:
      //  (1) the two histograms to be rescaled
      //  (2) the cumulative distributions of the two histograms
      //  (3) the fit to the comparison points of the two cumulative distributions
      //  (4) a comparison of the resulting rescaled histograms
      // See MakeHistoRescaleX for an example.
      kVisDebug = on;
   };
   Bool_t IsVisDebug() const
   {
      return kVisDebug;
   };

   Int_t CutStatBin(TH1* hh, Int_t stat_min = -1, Int_t stat_max = -1);

   Int_t Apply_TCutG(TH2* hh, TCutG* cut, TString mode = "in");

   TH1*  ScaleHisto(TH1* hh, TF1* fx, TF1* fy = NULL, Int_t nx = -1, Int_t ny = -1,
                    Double_t xmin = -1., Double_t xmax = -1., Double_t ymin = -1., Double_t ymax = -1., Option_t* norm = "");
   TGraph* ScaleGraph(TGraph* hh, TF1* fx, TF1* fy);

   TH1*  CentreeReduite(TH1* hh, Int_t nx = -1, Int_t ny = -1, Double_t xmin = -1., Double_t xmax = -1., Double_t ymin = -1., Double_t ymax = -1.);
   TH2*  CentreeReduiteX(TH2* hh, Int_t nx = -1, Double_t xmin = -1., Double_t xmax = -1.);
   TH2*  CentreeReduiteY(TH2* hh, Int_t ny = -1, Double_t ymin = -1., Double_t ymax = -1.);

   TH2*  RenormaliseHisto(TH2* hh, Int_t bmin = -1, Int_t bmax = -1, TString axis = "X", Double_t valref = 1);
   TH2*  RenormaliseHisto(TH2* hh, Double_t valmin, Double_t valmax, TString axis = "X", Double_t valref = 1);

   TH1*  CumulatedHisto(TH1* hh, TString direction = "C", Int_t bmin = -1, Int_t bmax = -1, Option_t* norm = "surf");
   TH1*  CumulatedHisto(TH1* hh, Double_t xmin, Double_t xmax, const TString& direction = "C", Option_t* norm = "surf");
   TH1*  GetDerivative(TH1* hh, Int_t order);

   TGraphErrors* GetMomentEvolution(TH2* hh, TString momentx, TString momenty, TString axis = "Y", Double_t stat_min = 0);
   Double_t GetCorrelationFactor(TH2* hh);
   TGraph* LinkGraphs(TGraph* grx, TGraph* gry);

   KVList* Give_ProjectionList(TH2* hh, Double_t MinIntegral = -1, TString axis = "X");
   KVNumberList* Saucisson(TH1* hh, Int_t ntranches = 10);
   TH2* PermuteAxis(TH2* hh);
   TGraph* PermuteAxis(TGraph* gr);
   TGraphErrors* MakeGraphFrom(TProfile* pf, Bool_t Error = kTRUE);

   void DefinePattern(TH1* ob, TString titleX = "42 0.08 0.8", TString titleY = "42 0.07 1.2", TString labelX = "42 0.05 0.005", TString labelY = "42 0.05 0.006");
   void DefinePattern(TGraph* ob, TString titleX = "42 0.08 0.8", TString titleY = "42 0.07 1.2", TString labelX = "42 0.05 0.005", TString labelY = "42 0.05 0.006");
   void DefinePattern(TF1* ob, TString titleX = "42 0.08 0.8", TString titleY = "42 0.07 1.2", TString labelX = "42 0.05 0.005", TString labelY = "42 0.05 0.006");
   void DefinePattern(TAxis* ax, TString title = "42 0.08 0.8", TString label = "42 0.05 0.005");

   void DefineLineStyle(TAttLine* ob, TString line);
   void DefineMarkerStyle(TAttMarker* ob, TString marker);
   void DefineStyle(TObject* ob, TString line, TString marker);

   void DefineTitle(TH1* ob, TString xtit, TString ytit);
   void DefineTitle(TGraph* ob, TString xtit, TString ytit);
   void DefineTitle(TF1* ob, TString xtit, TString ytit);

   Double_t GetX(TH1* ob, Double_t val, Double_t eps = 1.e-07, Int_t nmax = 50, Double_t xmin = -1.0, Double_t xmax = -1.0);
   Double_t GetXWithLimits(TH1* ob, Double_t val, Double_t xmin = -1.0, Double_t xmax = -1.0, Double_t eps = 1.e-07, Int_t nmax = 50)
   {
      // See method GetX(): the only difference is the order of the arguments
      return GetX(ob, val, eps, nmax, xmin, xmax);
   }
   TF1* RescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params,
                 Int_t npoints = -1, const Char_t* direction = "C",
                 Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                 Double_t eps = 1.e-07);
   void RescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints = 2,
                 const Char_t* direction = "C", Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                 Double_t eps = 1.e-07);
   TH1* MakeHistoRescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params,
                          Option_t* opt = "", Int_t npoints = -1, const Char_t* direction = "C",
                          Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                          Double_t eps = 1.e-07);
   TH1* MakeHistoRescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints = 2,
                          Option_t* opt = "", const Char_t* direction = "C",
                          Double_t xmin = -1, Double_t xmax = -1, Double_t qmin = 0.05, Double_t qmax = 0.95,
                          Double_t eps = 1.e-07);

   Double_t GetChisquare(TH1* h1, TF1* f1, Bool_t norm = kTRUE, Bool_t err = kTRUE, Double_t* para = nullptr);
   Double_t GetLikelihood(TH1* h1, TF1* f1, Bool_t norm = kTRUE, Double_t* para = nullptr);

   TGraph* DivideGraphs(TGraph* G1, TGraph* G2);
   TGraph* ComputeNewGraphFrom(TGraph* g0, TGraph* g1, const TString& formula);
   std::vector<Double_t> GetLimits(TGraph* G1);
   std::vector<Double_t> GetLimits(TProfile* G1);
   std::vector<Double_t> GetLimits(TMultiGraph* mgr);
   std::vector<Double_t> GetLimits(TSeqCollection* mgr);
   void ApplyCurrentLimitsToAllCanvas(Bool_t AlsoLog = kFALSE);
   TGraph* ExtractMeanAndSigmaFromProfile(TProfile* pf, TGraph*& sigma);

   ClassDef(KVHistoManipulator, 1) //Propose differentes operations sur les histo
};

//................  global variable
R__EXTERN KVHistoManipulator* gHistoManipulator;

#endif
