//Created by KVClassFactory on Thu Jun 13 09:49:14 2019
//Author: John Frankland,,,

#include "KVCalibratedSignal.h"

ClassImp(KVCalibratedSignal)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCalibratedSignal</h2>
<h4>Detector signal produced by a calibrator</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Double_t KVCalibratedSignal::GetValue(const KVNameValueList& params) const
{
   // Returns the value of the calibrated signal using the current value of the input
   // signal* and the current parameters of the calibration
   //
   // Any additional parameters required by the calibration can be passed as a
   // comma-separated list of `parameter=value` pairs
   //
   // *if a parameter `INPUT` is given, its value is used instead of the input signal.

   if (params.HasParameter("INPUT"))
      return fCalibrator->Compute(params.GetDoubleValue("INPUT"), params);
   return fCalibrator->Compute(fInputSignal->GetValue(), params);
}

Double_t KVCalibratedSignal::GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params) const
{
   // Returns the value of the input signal "in_sig" for a given value of this signal,
   // using the inverse calibration function.
   //
   // Note that "in_sig" may not be the name/type of the direct input signal for this signal,
   // in which case the chain of signals/calibrators is followed back to the required signal.
   //
   // Any additional parameters required by the calibration can be passed as a
   // comma-separated list of 'parameter=value' pairs

   if (in_sig == GetName()) return out_val;
   return fInputSignal->GetInverseValue(fCalibrator->Invert(out_val, params), in_sig, params);
}

//____________________________________________________________________________//

