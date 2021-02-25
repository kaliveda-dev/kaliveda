//Created by KVClassFactory on Wed Nov  6 17:35:31 2019
//Author: John Frankland,,,

#include "KVZDependentCalibratedSignal.h"

ClassImp(KVZDependentCalibratedSignal)

void KVZDependentCalibratedSignal::AddSignal(KVCalibratedSignal* sig, const KVNumberList& zrange)
{
   // Add signal to be used for all values of Z in the number list range

   zrange.Begin();
   while (!zrange.End()) fSignalMap[zrange.Next()] = sig;
   fSignals.Add(sig);
}
