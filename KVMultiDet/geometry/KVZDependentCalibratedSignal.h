//Created by KVClassFactory on Wed Nov  6 17:35:31 2019
//Author: John Frankland,,,

#ifndef __KVZDEPENDENTCALIBRATEDSIGNAL_H
#define __KVZDEPENDENTCALIBRATEDSIGNAL_H

#include "KVCalibratedSignal.h"

/**
  \class KVZDependentCalibratedSignal
  \ingroup Geometry
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

   Double_t GetValue(const KVNameValueList& params = "") const;
   Double_t GetInverseValue(Double_t out_val, const TString& in_sig, const KVNameValueList& params = "") const;

   KVCalibratedSignal* GetSignal(const KVNameValueList&) const;

   ClassDef(KVZDependentCalibratedSignal, 1) //Handle several calibrations valid for different Z ranges
};

#endif
