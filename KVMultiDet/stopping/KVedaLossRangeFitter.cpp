//Created by KVClassFactory on Thu Dec 20 14:56:27 2012
//Author: John Frankland,,,

#include "KVedaLossRangeFitter.h"
#include "TGraph.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "Riostream.h"
#include <math.h>
using namespace::std;

ClassImp(KVedaLossRangeFitter)

static Double_t energy[] = {
   0.1, .2, 0.5, 1, 2, 5, 10, 25, 50, 100, 250, 500, -1
};

Double_t KVedaLossRangeFitter::range(Double_t* e, Double_t* a)
{
   // express logR as a polynomial of log(E/A)
   Double_t lr = a[5];
   for (int i = 4; i >= 0; --i) lr = e[0] * lr + a[i];
   return lr;
}

KVedaLossRangeFitter::KVedaLossRangeFitter()
{
   // Default constructor
   fRangeFunction = new TF1("VedaRangeFit", this, &KVedaLossRangeFitter::range, -10., 10., 6);
}

KVedaLossRangeFitter::~KVedaLossRangeFitter()
{
   // Destructor
}

void KVedaLossRangeFitter::SetMaterial(KVIonRangeTableMaterial* m)
{
   // Sets range table to fit. Also finds material with closest Z in VEDALOSS library.
   fMaterial = m;
   Double_t zmat = fMaterial->GetZ();
   unique_ptr<TObjArray> mats(VEDALOSS.GetListOfMaterials());
   TString closest;
   Double_t closestZ = 100;
   TNamed* mat;
   TIter next(mats.get());
   while ((mat = (TNamed*)next())) {
      KVIonRangeTableMaterial* y = VEDALOSS.GetMaterial(mat->GetName());
      Double_t yZ = y->GetZ();
      if (TMath::Abs(yZ - zmat) < TMath::Abs(closestZ - zmat)) {
         closestZ = yZ;
         closest = mat->GetName();
      }
   }
   fClosestVedaMat = (KVedaLossMaterial*)VEDALOSS.GetMaterial(closest);
   Info("SetMaterial", "Initial fit parameters will be taken from:");
   fClosestVedaMat->Print();
}

void KVedaLossRangeFitter::SetInitialParameters(Int_t Z)
{
   // Set initial parameters for this Z by using closest known material in
   // VedaLoss range tables

   std::vector<Double_t> pars;
   fClosestVedaMat->GetParameters(Z, Aref, pars);
   int npar = fRangeFunction->GetNpar();
   for (int i = 0; i < npar; ++i) {
      fRangeFunction->SetParameter(i, pars[i]);
   }
}

TGraph* KVedaLossRangeFitter::GenerateRangeTable(Int_t Z)
{
   // Create TGraph containing log(range) vs. log (E/A) for ions of given
   // atomic number (the mass used is the reference mass found in the VEDALOSS
   // tables).
   // Convert range to mg/cm**2 (units of internal VEDALOSS range function)

   TGraph* r = new TGraph;
   r->SetMarkerStyle(24);
   r->SetTitle(Form("Z = %d", Z));
   int i = 0, ip = 0;
   while (1) {
      if (energy[i] < 0.) break;
      Double_t logE = log(energy[i]);
      Double_t R = fMaterial->GetRangeOfIon(Z, Aref, Aref * energy[i]) / KVUnits::mg;
      if (R > 0.) {
         r->SetPoint(ip++, logE, log(R));
      }
      i++;
   }
   return r;
}

TGraph* KVedaLossRangeFitter::GenerateVEDALOSSRangeTable(Int_t Z)
{
   // Create TGraph containing log(range) vs. log (E/A) for ions of given
   // atomic number using current VEDALOSS reference material.
   // Convert range to mg/cm**2 (units of internal VEDALOSS range function)
   TGraph* r = new TGraph;
   r->SetMarkerStyle(20);
   r->SetMarkerColor(kBlue);
   r->SetTitle(Form("VEDA Z = %d", Z));
   int i = 0, ip = 0;
   while (1) {
      if (energy[i] < 0.) break;
      Double_t logE = log(energy[i]);
      Double_t R = fClosestVedaMat->GetRangeOfIon(Z, Aref, Aref * energy[i]) / KVUnits::mg;
      if (R > 0.) {
         r->SetPoint(ip++, logE, log(R));
      }
      i++;
   }
   return r;
}

Int_t KVedaLossRangeFitter::FitRangeTable(TGraph* R)
{
   // Fit ranges in TGraph
   // return value = 0 if fit is good

   return (Int_t)R->Fit(fRangeFunction, "WQ");
}

void KVedaLossRangeFitter::DoFits(TString output_file, Int_t Zmin, Int_t Zmax)
{
   // Perform fits to range tables for elements from Zmin to Zmax
   TGraph* g = 0;
   TGraph* v = 0;
   new TCanvas;

   ofstream results(output_file.Data());

   PrintFitHeader(results);
   for (int Z = Zmin; Z <= Zmax; Z++) {
      gPad->Clear();
      SetInitialParameters(Z);
      if (g) delete g;
      if (v) delete v;
      g = GenerateRangeTable(Z);
      g->Draw("AP");
      Int_t fitStatus = FitRangeTable(g);
      v = GenerateVEDALOSSRangeTable(Z);
      v->Draw("P");
      printf("Z = %d : fit status ", Z);
      if (fitStatus == 0) {
         printf("OK\n");
      }
      else
         printf("PROBLEM (%d)\n", fitStatus);
      PrintFitParameters(Z, results);
      gPad->Modified();
      gPad->Update();
      gSystem->Sleep(500);
   }
   results.close();
}

void KVedaLossRangeFitter::PrintFitParameters(Int_t Z, ostream& output)
{
   output << setw(4) << Z << "." << setw(4) << Aref << ". ";
   output.precision(4);
   for (int i = 0; i < 6; i++) output << uppercase << scientific << fRangeFunction->GetParameter(i) << " ";
   output << endl << "           ";
   for (int i = 0; i < 6; i++) output << uppercase << scientific << fRangeFunction->GetParameter(i) << " ";
   output << endl;
}

void KVedaLossRangeFitter::PrintFitHeader(ostream& output)
{
//    + Si Silicon solid 2.33 14.0 28.0855 0. 0.
//    ELEMENT
//    Z = 1,2     0.1 < E/A  <  400 MeV
//    Z = 3,100   0.1 < E/A  <  250 MeV

//            + Myl Mylar solid 1.395 4.545 8.735 0. 0.
//            COMPOUND
//            3
//            6 12 10
//            1 1 8
//            8 16 4
//            Z = 1,2     0.1 < E/A  <  400 MeV
//            Z = 3,100   0.1 < E/A  <  250 MeV

//            + NE102 Plastic solid 1.032 3.5 6.509 0. 0.
//            MIXTURE
//            2
//            1 1 1 5.25
//            6 12 1 4.73
//            Z = 1,2     0.1 < E/A  <  400 MeV
//            Z = 3,100   0.1 < E/A  <  250 MeV
   output << "+ " << fMaterial->GetType() << " " << fMaterial->GetName();
   if (fMaterial->IsGas()) output << " gas 0";
   else output << " solid " << fMaterial->GetDensity();
   output << " " << fMaterial->GetZ() << " " << fMaterial->GetMass();
   if (fMaterial->IsGas()) output << " " << fMaterial->GetMoleWt() << " 19." << endl;
   else output << " 0. 0." << endl;
   fMaterial->PrintComposition(output);
   output << "Z = 1,2     0.1 < E/A  <  400 MeV" << endl << "Z = 3,100   0.1 < E/A  <  250 MeV" << endl;
}
