//Created by KVClassFactory on Wed Nov  6 17:35:31 2019
//Author: John Frankland,,,

#ifndef __KVZDEPENDENTCALIBRATEDSIGNAL_H
#define __KVZDEPENDENTCALIBRATEDSIGNAL_H

#include "KVCalibratedSignal.h"

/**
  \class KVZDependentCalibratedSignal
  \ingroup Calibration
  \brief Handle several calibrations valid for different Z ranges

  If 2 or more calibrations which define the same SignalOut have a ZRange parameter:

~~~~~~~~~~~~~~~
  calib1.dat:
  SignalOut:  Energy
  ZRange:  1-5

  calib2.dat:
  SignalOut:  Energy
  ZRange: 6-92
~~~~~~~~~~~~~~~

  a single `Energy` signal object of this class will be added to the concerned detectors,
  it will handle switching between the calibrators when GetValue() etc. are called with
  a "Z=..." parameter in the KVNameValueList argument.
 */

#include <map>

class KVZDependentCalibratedSignal : public KVCalibratedSignal {
   mutable std::map<int, KVCalibratedSignal*> fSignalMap;
   KVList fSignals;//! to cleanup signals on delete

public:
   KVZDependentCalibratedSignal() : KVCalibratedSignal() {}
   KVZDependentCalibratedSignal(KVDetectorSignal* input, const KVString& output)
      : KVCalibratedSignal(input, output)
   {}
   virtual ~KVZDependentCalibratedSignal() {}

   void AddSignal(KVCalibratedSignal* sig, const KVNumberList& zrange);

   Double_t GetValue(const KVNameValueList& params = "") const
   {
      KVCalibratedSignal* sig = GetSignal(params);
      return (sig ? sig->GetValue(params) : -1);
   }
   Double_t GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params = "") const
   {
      KVCalibratedSignal* sig = GetSignal(params);
      return (sig ? sig->GetInverseValue(out_val, in_sig, params) : -1);
   }

   KVCalibratedSignal* GetSignal(const KVNameValueList& params) const
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

   Bool_t IsCalibratedFor(const KVNameValueList& params) const
   {
      // Returns true if a calibration is defined for the "Z=..." parameter value in the list

      if (!params.HasIntParameter("Z")) return false;
      return fSignalMap[params.GetIntValue("Z")] != nullptr;
   }

   Bool_t GetValueNeedsExtraParameters() const
   {
      return kTRUE;
   }

   ClassDef(KVZDependentCalibratedSignal, 1) //Handle several calibrations valid for different Z ranges
};

#endif
