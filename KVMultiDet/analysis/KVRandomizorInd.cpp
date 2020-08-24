//Created by KVClassFactory on Mon Jun 26 17:31:07 2017
//Author: Eric BONNET

#include "KVRandomizorInd.h"

typedef Double_t (KVRandomizorInd::*FNMETHOD)(Double_t);

ClassImp(KVRandomizorInd)


//-------------------------
KVRandomizorInd::KVRandomizorInd(Int_t ndim) : KVRandomizor(ndim)
//-------------------------
{
   // Default constructor
}

//-------------------------
KVRandomizorInd::~KVRandomizorInd()
//-------------------------
{
   // Destructor
}

//-------------------------
Double_t KVRandomizorInd::ComputeValue(Double_t* pos)
//-------------------------
{

   FNMETHOD Compute[] = {
      &KVRandomizorInd::ComputeValue0,
      &KVRandomizorInd::ComputeValue1,
      &KVRandomizorInd::ComputeValue2,
      &KVRandomizorInd::ComputeValue3,
      &KVRandomizorInd::ComputeValue4,
      &KVRandomizorInd::ComputeValue5,
   };

   Double_t prod = 1;
   for (Int_t ii = 0; ii < fNd; ii += 1) {
      prod *= (this->*Compute[ii])(pos[ii]);
   }

   return prod;
}

//-------------------------
Double_t  KVRandomizorInd::ComputeValue0(Double_t)
//-------------------------
{
   Info("ComputeValue0", "To be defined in child class");
   return 0;
}

//-------------------------
Double_t  KVRandomizorInd::ComputeValue1(Double_t)
//-------------------------
{
   Info("ComputeValue1", "To be defined in child class");
   return 0;
}

//-------------------------
Double_t  KVRandomizorInd::ComputeValue2(Double_t)
//-------------------------
{
   Info("ComputeValue2", "To be defined in child class");
   return 0;
}

//-------------------------
Double_t  KVRandomizorInd::ComputeValue3(Double_t)
//-------------------------
{
   Info("ComputeValue3", "To be defined in child class");
   return 0;
}

//-------------------------
Double_t  KVRandomizorInd::ComputeValue4(Double_t)
//-------------------------
{
   Info("ComputeValue4", "To be defined in child class");
   return 0;
}

//-------------------------
Double_t  KVRandomizorInd::ComputeValue5(Double_t)
//-------------------------
{
   Info("ComputeValue5", "To be defined in child class");
   return 0;
}

