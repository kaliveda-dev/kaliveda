//Created by KVClassFactory on Thu Mar  2 10:50:41 2017
//Author: Diego Gruyer

#include "KVExcitedState.h"
#include "TMath.h"
#include "TF1.h"
#include "Riostream.h"
using namespace std;

ClassImp(KVExcitedState)

KVExcitedState::KVExcitedState()
{
   // Default constructor
}

//____________________________________________________________________________//

KVExcitedState::~KVExcitedState()
{
   // Destructor
}

//____________________________________________________________________________//

void KVExcitedState::print()
{
   cout << Form("%15s", fJPi.Data()) << " _____  "
        << Form("%5d", TMath::Nint(fEnergy)) << " keV"
        << Form("%15s", fGamma.Data())
        << endl;
}

void KVExcitedState::set(Double_t ee, Double_t ww, Double_t jj, Int_t pi)
{
   fEnergy = ee;
   fWidth = ww;
   if (jj - TMath::Nint(jj) > 0.1) {
      fSpin = TMath::Nint(jj * 2);
      fIsOdd = kTRUE;
   }
   else {
      fSpin = TMath::Nint(jj);
      fIsOdd = kFALSE;
   }
   fParity = pi;
   fJPi = Form("%d%s", fSpin, ((fParity > 0) ? "+" : "-"));
   fGamma = Form("%.2lf keV", ww);
}


void KVExcitedState::set(Double_t ee, const char* jpi, const char* t12)
{
   fEnergy = ee;
   fJPi = jpi;
   fGamma = t12;
   computeParity();
   computeSpin();
   computeWidth();
}

void KVExcitedState::computeParity()
{
   TString jpi = fJPi.Data();
   jpi.ReplaceAll("(", "");
   jpi.ReplaceAll(")", "");
   if (jpi.EqualTo("") || jpi.Contains("GE")) {
      fParity = 1;
      return;
   }
   if ((jpi.Contains("-")) && (jpi.Contains("+"))) {
      if (jpi.Index("+") < jpi.Index("-")) fParity = 1;
      else  fParity = -1;
   }
   else if (jpi.Contains("-")) fParity = -1;
   else                       fParity = 1;
}

void KVExcitedState::computeSpin()
{
   TString jpi = fJPi.Data();
   jpi.ReplaceAll("(", "");
   jpi.ReplaceAll(")", "");
   if (jpi.EqualTo("") || jpi.Contains("GE")) {
      fSpin = 0;
      fIsOdd = kFALSE;
      return;
   }
   if (jpi.Contains("-"))  jpi = jpi(0, jpi.Index("-"));
   if (jpi.Contains("+"))  jpi = jpi(0, jpi.Index("+"));
   if (jpi.Contains("/2")) {
      jpi = jpi(0, jpi.Index("/"));
      fIsOdd = kTRUE;
   }
   else {
      fIsOdd = kFALSE;
   }
   if (jpi.Contains(","))  jpi = jpi(0, jpi.Index(","));
   fSpin = jpi.Atoi();
}

void KVExcitedState::computeWidth()
{
   Double_t gam;
//   Double_t dgam;
   TString  ugam;

   gam  = -1;
//   dgam = -1;
   ugam = "";

   KVString tmp = fGamma;
   tmp.RemoveAllExtraWhiteSpace();
   int np = tmp.GetNValues(" ");
   tmp.Begin(" ");

   if (np >= 1) gam = tmp.Next().Atof();
   if (np >= 2) ugam = tmp.Next().Data();
//   if (np >= 3) dgam = tmp.Next().Atof();

   fWidth = gam;

   ugam.ToUpper();
   if (ugam.EqualTo("EV"))        fWidth *= 1e-3;
   else if (ugam.EqualTo("KEV"))  fWidth *= 1;
   else if (ugam.EqualTo("MEV"))  fWidth *= 1e3;
   else if (ugam.Contains("S"))   fWidth *= 1e-6;

   ugam.ToLower();
   ugam.ReplaceAll("v", "V");
   if (!ugam.Contains("s")) ugam.ReplaceAll("m", "M");


   if (gam == 0.) fGamma = Form("%.1f %3s", gam, ugam.Data());
   else        fGamma = Form("%.1f %3s", gam, ugam.Data());
}

Double_t KVExcitedState::eval(Double_t excit)
{
   double xx = excit;
   double JJ = fSpin;
   if (fIsOdd) JJ *= 0.5;
   double yy = (2.*JJ + 1.) * fWidth / ((xx - fEnergy) * (xx - fEnergy) + 0.25 * fWidth * fWidth);
   return 0.5 * yy / TMath::Pi();
}
