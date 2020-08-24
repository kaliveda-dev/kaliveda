//Created by KVClassFactory on Fri Jun 23 12:02:13 2017
//Author: Eric BONNET

#ifndef __KVRandomizor_H
#define __KVRandomizor_H

#include "TNamed.h"
#include <vector>
class TRandom3;
class TH1;

/**
\class KVRandomizor
\brief Test of generic class to perform sample on functions
\ingroup Math
*/

class KVRandomizor : public TNamed {

protected:
   Int_t fNtest;
   Int_t fNd;
   Int_t fNdMax;

   Double_t fVmax;
   Double_t fVmin;
   std::vector<Double_t> fMin;
   std::vector<Double_t> fMax;

   TH1* FillHisto1D(Int_t);
   TH1* FillHisto2D(Int_t);
   TH1* FillHisto3D(Int_t);

public:
   KVRandomizor(Int_t ndim);
   //KVRandomizor(const char* name, const char* title);

   void SetExtrema(Double_t, Double_t vmin = 0);
   void SetRange(Double_t* min, Double_t* max);
   std::vector<Double_t> GetPosition();
   Double_t GetPosition(Int_t idx);
   virtual Double_t ComputeValue(Double_t* pos);

   Bool_t TestValue(Double_t);

   TH1* FillHisto(Int_t ntimes = 1000);
   TH1* Test(Int_t ntimes = 1000);
   virtual ~KVRandomizor();

   ClassDef(KVRandomizor, 1) //Test of generic class to perform sample on fonctions
};

#endif
