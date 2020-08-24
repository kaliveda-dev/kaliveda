//Created by KVClassFactory on Mon Jun 26 17:31:12 2017
//Author: Eric BONNET

#include "KVRandomizorCor.h"

ClassImp(KVRandomizorCor)

KVRandomizorCor::KVRandomizorCor(Int_t ndim) : KVRandomizor(ndim)
{
   // Default constructor
}

//____________________________________________________________________________//

KVRandomizorCor::~KVRandomizorCor()
{
   // Destructor
}

//____________________________________________________________________________//

Double_t KVRandomizorCor::ComputeValue(Double_t*)
{

   Double_t prod = 1;

   return prod;
}
