//Created by KVClassFactory on Thu Jun 13 09:49:14 2019
//Author: John Frankland,,,

#include "KVCalibratedSignal.h"

ClassImp(KVCalibratedSignal)

Double_t KVCalibratedSignal::GetValue(const KVNameValueList& params) const
{
   // Returns the value of the calibrated signal using the current value of the input
   // signal* and the current parameters of the calibration
   //
   // Any additional parameters required by the calibration can be passed as a
   // comma-separated list of `parameter=value` pairs
   //
   // In case a problem occurs with the calculation [in case of an inverted calibration function],
   // InversionFailure() will return kTRUE.
   //
   // *if a parameter `INPUT` is given, its value is used instead of the input signal.

   fInversionFail = false;
   double result;
   if (params.HasParameter("INPUT"))
      result = fCalibrator->Compute(params.GetDoubleValue("INPUT"), params);
   else
      result = fCalibrator->Compute(fInputSignal->GetValue(), params);
   fInversionFail = fCalibrator->InversionFailure();
   return result;
}

Double_t KVCalibratedSignal::GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params) const
{
   // Returns the value of the input signal "in_sig" for a given value of this signal,
   // using the inverse calibration function.
   //
   // Note that "in_sig" may not be the name/type of the direct input signal for this signal,
   // in which case the chain of signals/calibrators is followed back to the required signal.
   //
   // In case a problem occurs with the calculation, InversionFailure() will return kTRUE.
   //
   // Any additional parameters required by the calibration can be passed as a
   // comma-separated list of 'parameter=value' pairs

   if (in_sig == GetName()) return out_val;
   return fInputSignal->GetInverseValue(fCalibrator->Invert(out_val, params), in_sig, params);
}

//____________________________________________________________________________//

