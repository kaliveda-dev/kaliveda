//Created by KVClassFactory on Fri Nov 20 12:23:35 2015
//Author: gruyer,,,

#ifndef __KVLevelScheme_H
#define __KVLevelScheme_H

#include "KVNucleus.h"
#include "TMath.h"
#include "TObject.h"
#include "TF1.h"
#include "KVExcitedState.h"
#include "TH2F.h"
#include "TCanvas.h"

/**
\class KVLevelScheme
\brief Tool to simulate nucleus multi-particle decay
\ingroup NucProp

\note the default energy unit here is keV and not MeV
*/

class KVLevelScheme {
public:
   KVNucleus* fDecayProd, *fCompNuc;
   Double_t fQvalue;

   KVList fLevels;
   TF1* fFunc, *fFuncErel;

   int ncol;
   int txs;
   double dx;
   double ddx;
   double dy;
   TH2F* hh;
   TCanvas* cc;

protected:
   double func(double xx, double tt);
   double getWidth(double gam, TString unit);
   int    getPI(TString jpi);
   int    getJ(TString jpi);

public:

   Int_t GetNLevels()
   {
      return (int)fLevels.GetSize();
   }
   Double_t GetLevelEnergy(int il);
   Double_t GetLevelWidth(int il);
   Int_t GetLevelSpin(int il);
   Int_t GetLevelParity(int il);

   const char* GetJPiStr(int il);
   const char* GetEGammaStr(int il);

   KVLevelScheme(const char* symb);
   virtual ~KVLevelScheme();

   void InitStructure();
   void ReadLevels(const char* file);
   void Print();

   Double_t GetQvalue()
   {
      return fQvalue;
   }
   Double_t GetErelFromExci(Double_t ex)
   {
      return (ex + fQvalue);
   }
   Double_t GetExciFromErel(Double_t erel)
   {
      return (erel - fQvalue);
   }

   void AddResonance(Double_t ex/*kEV*/, const char* jpi, const char* gam);
   double Evaluate(double* x, double* p);
   double EvaluateErel(double* x, double* p);

   TF1* GetExciSpectrum()
   {
      return fFunc;
   }
   TF1* GetErelSpectrum()
   {
      return fFuncErel;
   }

   static void GetParticlesFromErel(KVNucleus* n1, KVNucleus* n2, double erel, bool randAngle = 0, TVector3* vsrc = 0);
   static void GetParticlesFromExci(KVNucleus* n1, KVNucleus* n2, double erel, bool randAngle = 0, TVector3* vsrc = 0);

   void GetRandomParticles(KVNucleus* n1, KVNucleus* n2, double T/*keV*/, bool randAngle = 0, TVector3* vsrc = 0);

   void SetDrawStyle(double deMin = 300., double fullWidth = 1.3, double lineWidth = 0.6, int textSize = 22);
   void Draw(Option_t* option = "");
   void DrawThreshold(const char* symb, Option_t* option = "", double ex = 0. /*in keV*/);
   double GetThreshold(const char* outnuc = "");

   void SetDecayProduct(KVNucleus* nuc, double excit_energy = 0/*keV*/);

   ClassDef(KVLevelScheme, 1) //tool to simulate nucleus multi-particle decay
};

#endif
