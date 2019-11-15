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

Double_t KVZDependentCalibratedSignal::GetValue(const KVNameValueList& params) const
{
   KVCalibratedSignal* sig = GetSignal(params);
   return (sig ? sig->GetValue(params) : -1);
}

Double_t KVZDependentCalibratedSignal::GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params) const
{
   KVCalibratedSignal* sig = GetSignal(params);
   return (sig ? sig->GetInverseValue(out_val, in_sig, params) : -1);
}

KVCalibratedSignal* KVZDependentCalibratedSignal::GetSignal(const KVNameValueList& params) const
{
   // Based on the value of the parameter "Z=..." (which must be present)
   // find the right calibrated signal

   if (!params.HasIntParameter("Z")) {
      Error("GetSignal", "No Z parameter given in KVNameValueList!");
      return nullptr;
   }
   KVCalibratedSignal* sig = fSignalMap[params.GetIntValue("Z")];
   if (!sig) {
      //Error("GetSignal", "No calibration for Z=%d for detector %s", params.GetIntValue("Z"), GetDetector()->GetName());
   }
   return sig;
}
